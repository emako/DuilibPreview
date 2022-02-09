using System;
using System.Diagnostics;
using System.IO;

namespace DuilibPreviewWarper
{
    internal class Program
    {
        [STAThread]
        public static void Main(string[] _)
        {
            string tempDir = Path.Combine(Path.GetTempPath(), "DuilibPreviewWarper");
            try
            {
                if (!Directory.Exists(tempDir))
                {
                    Directory.CreateDirectory(tempDir);
                }
                if (!File.Exists(Path.Combine(tempDir, "DuiLib_u.dll")))
                {
                    File.WriteAllBytes(Path.Combine(tempDir, "DuiLib_u.dll"), Resource.DuiLib_u);
                }
                if (!File.Exists(Path.Combine(tempDir, "DuilibPreview.exe")))
                {
                    File.WriteAllBytes(Path.Combine(tempDir, "DuilibPreview.exe"), Resource.DuilibPreview);
                }
            }
            catch
            {
            }
            Process.Start(Path.Combine(tempDir, "DuilibPreview.exe"));
        }
    }
}
