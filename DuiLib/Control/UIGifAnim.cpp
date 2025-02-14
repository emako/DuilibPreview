#include "StdAfx.h"
#include "UIGifAnim.h"

namespace DuiLib
{
	extern Color ARGB2Color(DWORD dwColor);

	CGifAnimUI::CGifAnimUI(void)
	{
		m_pGifImage			=	NULL;
		m_pPropertyItem		=	NULL;
		m_nFrameCount		=	0;	
		m_nFramePosition	=	0;	
		m_bIsAutoPlay		=	true;
		m_bIsAutoSize		=	false;
		m_bIsPlaying		=	false;
		m_pStream				=	NULL;

		m_uTextStyle = DT_SINGLELINE | DT_VCENTER | DT_CENTER;
		m_TextRenderingAlias = TextRenderingHintAntiAlias;
		m_dwTextColor = 0xFF000000;
		m_dwDisabledTextColor = 0xFF000000;
		m_iFont = -1;
	}


	CGifAnimUI::~CGifAnimUI(void)
	{
		DeleteGif();
		if (m_pManager)
			m_pManager->KillTimer( this, EVENT_TIME_ID );

	}

	LPCTSTR CGifAnimUI::GetClass() const
	{
		return DUI_CTR_GIFANIM;
	}

	LPVOID CGifAnimUI::GetInterface( LPCTSTR pstrName )
	{
		if( _tcscmp(pstrName, DUI_CTR_GIFANIM) == 0 ) return static_cast<CGifAnimUI*>(this);
		return CControlUI::GetInterface(pstrName);
	}

	void CGifAnimUI::DoInit()
	{
		InitGifImage();
	}

	bool CGifAnimUI::DoPaint(HDC hDC, const RECT& rcPaint, CControlUI* pStopControl)
	{
		if ( NULL == m_pGifImage )
		{		
			InitGifImage();
		}
		DrawFrame( hDC );
		return true;
	}

	void CGifAnimUI::DoEvent( TEventUI& event )
	{
		if( event.Type == UIEVENT_TIMER )
			OnTimer( (UINT_PTR)event.wParam );
		else
			CControlUI::DoEvent(event);
	}

	void CGifAnimUI::SetVisible(bool bVisible /* = true */)
	{
		CControlUI::SetVisible(bVisible);
		if (bVisible)
			PlayGif();
		else
			StopGif();
	}

	void CGifAnimUI::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
	{
		if( _tcscmp(pstrName, _T("bkimage")) == 0 ) SetBkImage(pstrValue);
		else if( _tcscmp(pstrName, _T("autoplay")) == 0 ) {
			SetAutoPlay(_tcscmp(pstrValue, _T("true")) == 0);
		}
		else if( _tcscmp(pstrName, _T("autosize")) == 0 ) {
			SetAutoSize(_tcscmp(pstrValue, _T("true")) == 0);
		}
		else if( _tcscmp(pstrName, _T("align")) == 0 ) 
		{
			if( _tcscmp(pstrValue, _T("left")) == 0 ) {
				m_uTextStyle &= ~(DT_CENTER | DT_RIGHT);
				m_uTextStyle |= DT_LEFT;
			}
			else if( _tcscmp(pstrValue, _T("center")) == 0 ) {
				m_uTextStyle &= ~(DT_LEFT | DT_RIGHT);
				m_uTextStyle |= DT_CENTER;
			}
			else if( _tcscmp(pstrValue, _T("right")) == 0 ) {
				m_uTextStyle &= ~(DT_LEFT | DT_CENTER);
				m_uTextStyle |= DT_RIGHT;
			}
		}
		else if (_tcscmp(pstrName, _T("valign")) == 0)
		{
			if (_tcscmp(pstrValue, _T("top")) == 0) {
				m_uTextStyle &= ~(DT_BOTTOM | DT_VCENTER);
				m_uTextStyle |= DT_TOP;
			}
			else if (_tcscmp(pstrValue, _T("vcenter")) == 0) {
				m_uTextStyle &= ~(DT_TOP | DT_BOTTOM);
				m_uTextStyle |= DT_VCENTER;
			}
			else if (_tcscmp(pstrValue, _T("bottom")) == 0) {
				m_uTextStyle &= ~(DT_TOP | DT_VCENTER);
				m_uTextStyle |= DT_BOTTOM;
			}
		}
		else if( _tcscmp(pstrName, _T("textcolor")) == 0 ) {
			if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetTextColor(clrColor);
		}
		else if( _tcscmp(pstrName, _T("disabledtextcolor")) == 0 ) {
			if( *pstrValue == _T('#')) pstrValue = ::CharNext(pstrValue);
			LPTSTR pstr = NULL;
			DWORD clrColor = _tcstoul(pstrValue, &pstr, 16);
			SetDisabledTextColor(clrColor);
		}
		else if(_tcscmp(pstrName, _T("rhaa")) == 0 ) SetTextRenderingAlias(_ttoi(pstrValue));
		else if( _tcscmp(pstrName, _T("multiline")) == 0 ) SetMultiLine(_tcscmp(pstrValue, _T("true")) == 0);
		else if( _tcscmp(pstrName, _T("font")) == 0 ) SetFont(_ttoi(pstrValue));
		else if( _tcscmp(pstrName, _T("textpadding")) == 0 ) {
			RECT rcTextPadding = { 0 };
			LPTSTR pstr = NULL;
			rcTextPadding.left = _tcstol(pstrValue, &pstr, 10);  ASSERT(pstr);    
			rcTextPadding.top = _tcstol(pstr + 1, &pstr, 10);    ASSERT(pstr);    
			rcTextPadding.right = _tcstol(pstr + 1, &pstr, 10);  ASSERT(pstr);    
			rcTextPadding.bottom = _tcstol(pstr + 1, &pstr, 10); ASSERT(pstr);    
			SetTextPadding(rcTextPadding);
		}
		else
			CControlUI::SetAttribute(pstrName, pstrValue);
	}

	void CGifAnimUI::SetTextColor(DWORD dwTextColor)
	{
		m_dwTextColor = dwTextColor;
	}

	DWORD CGifAnimUI::GetTextColor() const
	{
		return m_dwTextColor;
	}

	bool CGifAnimUI::IsMultiLine()
	{
		return (m_uTextStyle & DT_SINGLELINE) == 0;
	}

	void CGifAnimUI::SetMultiLine(bool bMultiLine)
	{
		if (bMultiLine)	{
			m_uTextStyle  &= ~DT_SINGLELINE;
			m_uTextStyle |= DT_WORDBREAK;
		}
		else 
			m_uTextStyle |= DT_SINGLELINE;
	}

	void CGifAnimUI::SetDisabledTextColor(DWORD dwTextColor)
	{
		m_dwDisabledTextColor = dwTextColor;
		Invalidate();
	}

	DWORD CGifAnimUI::GetDisabledTextColor() const
	{
		return m_dwDisabledTextColor;
	}

	void CGifAnimUI::SetTextRenderingAlias(int nTextRenderingAlias)
	{
		m_TextRenderingAlias = (TextRenderingHint)nTextRenderingAlias;
		Invalidate();
	}

	TextRenderingHint CGifAnimUI::GetTextRenderingAlias()
	{
		return m_TextRenderingAlias;
	}

	void CGifAnimUI::SetFont(int index)
	{
		m_iFont = index;
	}

	int CGifAnimUI::GetFont() const
	{
		return m_iFont;
	}

	RECT CGifAnimUI::GetTextPadding() const
	{
		return m_rcTextPadding;
	}

	void CGifAnimUI::SetTextPadding(RECT rc)
	{
		m_rcTextPadding = rc;
	}

	void CGifAnimUI::SetBkImage(const CDuiString& strImage)
	{
		if( m_sBkImage == strImage || strImage.IsEmpty()) return;

		m_sBkImage = strImage;

		StopGif();
		DeleteGif();

		Invalidate();
	}

	CDuiString CGifAnimUI::GetBkImage()
	{
		return m_sBkImage;
	}

	void CGifAnimUI::SetAutoPlay(bool bIsAuto)
	{
		m_bIsAutoPlay = bIsAuto;
	}

	bool CGifAnimUI::IsAutoPlay() const
	{
		return m_bIsAutoPlay;
	}

	void CGifAnimUI::SetAutoSize(bool bIsAuto)
	{
		m_bIsAutoSize = bIsAuto;
	}

	bool CGifAnimUI::IsAutoSize() const
	{
		return m_bIsAutoSize;
	}

	void CGifAnimUI::PlayGif()
	{
		if (m_bIsPlaying || m_pGifImage == NULL)
		{
			return;
		}

		long lPause = ((long*) m_pPropertyItem->value)[m_nFramePosition] * 10;
		if ( lPause == 0 ) lPause = 100;
		m_pManager->SetTimer( this, EVENT_TIME_ID, lPause );

		m_bIsPlaying = true;
	}

	void CGifAnimUI::PauseGif()
	{
		if (!m_bIsPlaying || m_pGifImage == NULL)
		{
			return;
		}

		m_pManager->KillTimer(this, EVENT_TIME_ID);
		this->Invalidate();
		m_bIsPlaying = false;
	}

	void CGifAnimUI::StopGif()
	{
		if (!m_bIsPlaying)
		{
			return;
		}

		m_pManager->KillTimer(this, EVENT_TIME_ID);
		m_nFramePosition = 0;
		this->Invalidate();
		m_bIsPlaying = false;
	}

	void CGifAnimUI::InitGifImage()
	{
		m_pGifImage = LoadGifFromFile(GetBkImage().GetData());
		if ( NULL == m_pGifImage ) return;
		UINT nCount	= 0;
		nCount	=	m_pGifImage->GetFrameDimensionsCount();
		GUID* pDimensionIDs	=	new GUID[ nCount ];
		m_pGifImage->GetFrameDimensionsList( pDimensionIDs, nCount );
		m_nFrameCount	=	m_pGifImage->GetFrameCount( &pDimensionIDs[0] );
		int nSize		=	m_pGifImage->GetPropertyItemSize( PropertyTagFrameDelay );
		m_pPropertyItem	=	(Gdiplus::PropertyItem*) malloc( nSize );
		m_pGifImage->GetPropertyItem( PropertyTagFrameDelay, nSize, m_pPropertyItem );
		delete[] pDimensionIDs;
		pDimensionIDs = NULL;

		if (m_bIsAutoSize)
		{
			SetFixedWidth(m_pGifImage->GetWidth());
			SetFixedHeight(m_pGifImage->GetHeight());
		}
		if (m_bIsAutoPlay && nSize > 0)
		{
			PlayGif();
		}
	}

	void CGifAnimUI::DeleteGif()
	{
		if (m_pStream != NULL )
		{
			m_pStream->Release();
			m_pStream = NULL;
		}
		if ( m_pGifImage != NULL )
		{
			delete m_pGifImage;
			m_pGifImage = NULL;
		}

		if ( m_pPropertyItem != NULL )
		{
			free( m_pPropertyItem );
			m_pPropertyItem = NULL;
		}
		m_nFrameCount		=	0;	
		m_nFramePosition	=	0;	
	}

	void CGifAnimUI::OnTimer( UINT_PTR idEvent )
	{
		if ( idEvent != EVENT_TIME_ID )
			return;
		m_pManager->KillTimer( this, EVENT_TIME_ID );
		this->Invalidate();

		m_nFramePosition = (++m_nFramePosition) % m_nFrameCount;

		long lPause = ((long*) m_pPropertyItem->value)[m_nFramePosition] * 10;
		if ( lPause == 0 ) lPause = 100;
		m_pManager->SetTimer( this, EVENT_TIME_ID, lPause );
	}

	void CGifAnimUI::DrawFrame( HDC hDC )
	{
		if ( NULL == hDC || NULL == m_pGifImage ) return;
		GUID pageGuid = Gdiplus::FrameDimensionTime;
		Gdiplus::Graphics graphics( hDC );
		graphics.SetTextRenderingHint(m_TextRenderingAlias);
		graphics.SetSmoothingMode(SmoothingMode::SmoothingModeHighQuality);
		graphics.DrawImage( m_pGifImage, m_rcItem.left, m_rcItem.top, m_rcItem.right-m_rcItem.left, m_rcItem.bottom-m_rcItem.top );
		
		if (m_sText.IsEmpty() == false)
		{
			//绘制文本
#ifdef _UNICODE
			LPWSTR pWideText = (LPWSTR)m_sText.GetData();
#else 
			int iLen = _tcslen(m_sText.GetData());
			LPWSTR pWideText = new WCHAR[iLen + 1];
			::ZeroMemory(pWideText, (iLen + 1) * sizeof(WCHAR));
			::MultiByteToWideChar(CP_ACP, 0, m_sText.GetData(), -1, pWideText, iLen);
#endif
			Gdiplus::Font	nFont(hDC,m_pManager->GetFont(GetFont()));

			RECT rcText = {m_rcItem.left+m_rcTextPadding.left,m_rcItem.top+m_rcTextPadding.top,m_rcItem.right-m_rcTextPadding.right,m_rcItem.bottom-m_rcTextPadding.bottom};
			RectF nRc((float)rcText.left,(float)rcText.top,(float)rcText.right-rcText.left,(float)rcText.bottom-rcText.top);

			StringFormat format;
			StringAlignment sa = StringAlignmentNear;
			if ((m_uTextStyle & DT_VCENTER) != 0) 
				sa = StringAlignmentCenter;
			else if( (m_uTextStyle & DT_BOTTOM) != 0) 
				sa = StringAlignmentFar;
			format.SetLineAlignment((StringAlignment)sa);
			sa = StringAlignmentNear;
			if ((m_uTextStyle & DT_CENTER) != 0) 
				sa = StringAlignmentCenter;
			else if( (m_uTextStyle & DT_RIGHT) != 0) 
				sa = StringAlignmentFar;
			format.SetAlignment((StringAlignment)sa);
			if ((m_uTextStyle & DT_SINGLELINE) != 0) 
				format.SetFormatFlags(StringFormatFlagsNoWrap);

			DWORD clrColor = IsEnabled()?m_dwTextColor:m_dwDisabledTextColor;
			SolidBrush nBrush( ARGB2Color(clrColor) );

			graphics.DrawString(pWideText,wcslen(pWideText),&nFont,nRc,&format,&nBrush);
#ifndef _UNICODE
			delete[] pWideText;
#endif
		}


		m_pGifImage->SelectActiveFrame( &pageGuid, m_nFramePosition );
	}

	Gdiplus::Image* CGifAnimUI::LoadGifFromFile(LPCTSTR pstrGifPath)
	{
		LPBYTE pData = NULL;
		DWORD dwSize = 0;

		do 
		{
			CDuiString sFile = CPaintManagerUI::GetResourcePath();
			if( CPaintManagerUI::GetResourceZip().IsEmpty() ) {
				sFile += pstrGifPath;
				HANDLE hFile = ::CreateFile(sFile.GetData(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, \
					FILE_ATTRIBUTE_NORMAL, NULL);
				if( hFile == INVALID_HANDLE_VALUE ) break;
				dwSize = ::GetFileSize(hFile, NULL);
				if( dwSize == 0 ) break;

				DWORD dwRead = 0;
				pData = new BYTE[ dwSize ];
				::ReadFile( hFile, pData, dwSize, &dwRead, NULL );
				::CloseHandle( hFile );

				if( dwRead != dwSize ) {
					delete[] pData;
					pData = NULL;
					break;
				}
			}
			else {
				sFile += CPaintManagerUI::GetResourceZip();
				HZIP hz = NULL;
				if( CPaintManagerUI::IsCachedResourceZip() ) 
					hz = (HZIP)CPaintManagerUI::GetResourceZipHandle();
				else
					hz = OpenZip(sFile.GetData(),CPaintManagerUI::GetResourceZipPassword());
				if( hz == NULL ) 
					break;
				ZIPENTRY ze; 
				int i; 
				if( FindZipItem(hz, pstrGifPath, true, &i, &ze) != 0 ) 
					break;
				dwSize = ze.unc_size;
				if( dwSize == 0 ) 
					break;
				pData = new BYTE[ dwSize ];
				int res = UnzipItem(hz, i, pData, dwSize);
				if( res != 0x00000000 && res != 0x00000600) 
				{
					delete[] pData;
					pData = NULL;
					if( !CPaintManagerUI::IsCachedResourceZip() ) CloseZip(hz);
					break;
				}
				if( !CPaintManagerUI::IsCachedResourceZip() ) CloseZip(hz);
			}

		} while (0);

		while (!pData)
		{
			//读不到图片, 则直接去读取bitmap.m_lpstr指向的路径
			HANDLE hFile = ::CreateFile(pstrGifPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, \
				FILE_ATTRIBUTE_NORMAL, NULL);
			if( hFile == INVALID_HANDLE_VALUE ) break;
			dwSize = ::GetFileSize(hFile, NULL);
			if( dwSize == 0 ) 
			{
				CloseHandle(hFile);
				break;
			}

			DWORD dwRead = 0;
			pData = new BYTE[ dwSize ];
			::ReadFile( hFile, pData, dwSize, &dwRead, NULL );
			::CloseHandle( hFile );

			if( dwRead != dwSize ) {
				delete[] pData;
				pData = NULL;
			}
			break;
		}
		if (!pData)
		{
			return NULL;
		}

		Gdiplus::Image* pImage = LoadGifFromMemory(pData, dwSize);
		delete[] pData;
		return pImage;
	}

	Gdiplus::Image* CGifAnimUI::LoadGifFromMemory( LPVOID pBuf,size_t dwSize )
	{
		HGLOBAL hMem = ::GlobalAlloc(GMEM_MOVEABLE, dwSize);
		BYTE* pMem = (BYTE*)::GlobalLock(hMem);

		memcpy(pMem, pBuf, dwSize);
		::GlobalUnlock(hMem);

		::CreateStreamOnHGlobal(hMem, TRUE, &m_pStream);
		Gdiplus::Image *pImg = Gdiplus::Image::FromStream(m_pStream);
		if(!pImg || pImg->GetLastStatus() != Gdiplus::Ok)
		{
			m_pStream->Release();
			m_pStream = NULL;
			return 0;
		}
		return pImg;
	}


}