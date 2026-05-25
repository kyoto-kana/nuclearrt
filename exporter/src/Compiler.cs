using System.Diagnostics;

public class Compiler
{
	public void Compile(BuildType buildType, DirectoryInfo outputPath)
	{
		if (buildType == BuildType.SourceCode) return;

		if (buildType == BuildType.WindowsDebug || buildType == BuildType.WindowsRelease)
		{
			//make 'build/windows' directory
			DirectoryInfo buildDir = new DirectoryInfo(Path.Combine(outputPath.FullName, "build", "windows"));
			buildDir.Create();

			CTFAK.Utils.Logger.Log($"Running CMake to generate the project files...");
			ProcessStartInfo cmakeProjectInfo = new ProcessStartInfo("cmake", "../..");
			cmakeProjectInfo.WorkingDirectory = Path.Combine(outputPath.FullName, "build", "windows");
			cmakeProjectInfo.CreateNoWindow = true;
			cmakeProjectInfo.UseShellExecute = false;
			cmakeProjectInfo.RedirectStandardOutput = true;
			cmakeProjectInfo.RedirectStandardError = true;
			Process cmakeProcess = Process.Start(cmakeProjectInfo);

			cmakeProcess.OutputDataReceived += (sender, e) =>
			{
				CTFAK.Utils.Logger.Log(e.Data);
			};

			cmakeProcess.ErrorDataReceived += (sender, e) =>
			{
				CTFAK.Utils.Logger.Log(e.Data);
			};

			cmakeProcess.BeginOutputReadLine();
			cmakeProcess.BeginErrorReadLine();
			cmakeProcess.WaitForExit();

			if (cmakeProcess.ExitCode != 0)
			{
				CTFAK.Utils.Logger.Log($"CMake failed with exit code {cmakeProcess.ExitCode}");
				throw new Exception("CMake failed to generate the project files");
			}

			CTFAK.Utils.Logger.Log($"Building...");
			ProcessStartInfo buildInfo = new ProcessStartInfo("cmake", "--build . --config " + (buildType == BuildType.WindowsDebug ? "Debug" : "Release"));
			buildInfo.WorkingDirectory = Path.Combine(outputPath.FullName, "build", "windows");
			buildInfo.CreateNoWindow = true;
			buildInfo.UseShellExecute = false;
			buildInfo.RedirectStandardOutput = true;
			buildInfo.RedirectStandardError = true;
			Process buildProcess = Process.Start(buildInfo);

			buildProcess.OutputDataReceived += (sender, e) =>
			{
				CTFAK.Utils.Logger.Log(e.Data);
			};

			buildProcess.ErrorDataReceived += (sender, e) =>
			{
				CTFAK.Utils.Logger.Log(e.Data);
			};

			buildProcess.BeginOutputReadLine();
			buildProcess.BeginErrorReadLine();
			buildProcess.WaitForExit();

			if (buildProcess.ExitCode != 0)
			{
				CTFAK.Utils.Logger.Log($"Build failed with exit code {buildProcess.ExitCode}");
				throw new Exception("Build failed");
			}
		}
		else if (buildType == BuildType.Web)
		{
			DirectoryInfo buildDir = new DirectoryInfo(Path.Combine(outputPath.FullName, "build", "web"));
			buildDir.Create();

			CTFAK.Utils.Logger.Log($"Running CMake to generate the project files...");
			ProcessStartInfo cmakeProjectInfo = new ProcessStartInfo(
				"cmd.exe",
				"/c emcmake cmake ../.. -DCMAKE_BUILD_TYPE=Release -G Ninja"
			);
			cmakeProjectInfo.WorkingDirectory = Path.Combine(outputPath.FullName, "build", "web");
			cmakeProjectInfo.CreateNoWindow = true;
			cmakeProjectInfo.UseShellExecute = false;
			cmakeProjectInfo.RedirectStandardOutput = true;
			cmakeProjectInfo.RedirectStandardError = true;
			Process cmakeProcess = Process.Start(cmakeProjectInfo);

			cmakeProcess.OutputDataReceived += (sender, e) =>
			{
				CTFAK.Utils.Logger.Log(e.Data);
			};

			cmakeProcess.ErrorDataReceived += (sender, e) =>
			{
				CTFAK.Utils.Logger.Log(e.Data);
			};

			cmakeProcess.BeginOutputReadLine();
			cmakeProcess.BeginErrorReadLine();
			cmakeProcess.WaitForExit();

			if (cmakeProcess.ExitCode != 0)
			{
				CTFAK.Utils.Logger.Log($"CMake failed with exit code {cmakeProcess.ExitCode}");
				throw new Exception("CMake failed to generate the project files");
			}

			CTFAK.Utils.Logger.Log($"Building...");
			ProcessStartInfo buildInfo = new ProcessStartInfo(
				"cmd.exe",
				"/c cmake --build . --config Release"
			);
			buildInfo.WorkingDirectory = Path.Combine(outputPath.FullName, "build", "web");
			buildInfo.CreateNoWindow = true;
			buildInfo.UseShellExecute = false;
			buildInfo.RedirectStandardOutput = true;
			buildInfo.RedirectStandardError = true;
			Process buildProcess = Process.Start(buildInfo);

			buildProcess.OutputDataReceived += (sender, e) =>
			{
				CTFAK.Utils.Logger.Log(e.Data);
			};

			buildProcess.ErrorDataReceived += (sender, e) =>
			{
				CTFAK.Utils.Logger.Log(e.Data);
			};

			buildProcess.BeginOutputReadLine();
			buildProcess.BeginErrorReadLine();
			buildProcess.WaitForExit();

			if (buildProcess.ExitCode != 0)
			{
				CTFAK.Utils.Logger.Log($"Build failed with exit code {buildProcess.ExitCode}");
				throw new Exception("Build failed");
			}
		}
		else if (buildType == BuildType.Vita)
		{
			DirectoryInfo buildDir = new DirectoryInfo(Path.Combine(outputPath.FullName, "build", "vita"));
			buildDir.Create();

			CTFAK.Utils.Logger.Log($"Building for Vita...");

			string wslPath = "/" + outputPath.FullName
				.Replace("\\", "/")
				.Replace(":", "")
				.Insert(1, "/")
				.TrimStart('/');
			wslPath = "/mnt/" + char.ToLower(outputPath.FullName[0]) + outputPath.FullName.Substring(2).Replace("\\", "/");

			ProcessStartInfo buildInfo = new ProcessStartInfo();
			buildInfo.FileName = "wsl.exe";
			buildInfo.Arguments = $"-e bash -c \"cd '{wslPath}' && export VITASDK=/usr/local/vitasdk && export PATH=$VITASDK/bin:$PATH && export PKG_CONFIG_PATH=$VITASDK/arm-vita-eabi/lib/pkgconfig && rm -f build_vita/*.d && make -f Makefile.vita 2>&1\"";
			buildInfo.WorkingDirectory = outputPath.FullName;
			buildInfo.CreateNoWindow = true;
			buildInfo.UseShellExecute = false;
			buildInfo.RedirectStandardOutput = true;
			buildInfo.RedirectStandardError = true;

			Process buildProcess = Process.Start(buildInfo);

			buildProcess.OutputDataReceived += (sender, e) =>
			{
				if (!string.IsNullOrEmpty(e.Data)) CTFAK.Utils.Logger.Log(e.Data);
			};

			buildProcess.ErrorDataReceived += (sender, e) =>
			{
				if (!string.IsNullOrEmpty(e.Data)) CTFAK.Utils.Logger.Log(e.Data);
			};

			buildProcess.BeginOutputReadLine();
			buildProcess.BeginErrorReadLine();
			buildProcess.WaitForExit();

			if (buildProcess.ExitCode != 0)
			{
				CTFAK.Utils.Logger.Log($"Build failed with exit code {buildProcess.ExitCode}");
				throw new Exception("Build failed");
			}
		}
		else if (buildType == BuildType.WiiU)
		{
			DirectoryInfo buildDir = new DirectoryInfo(Path.Combine(outputPath.FullName, "build", "wiiu"));
			buildDir.Create();

			CTFAK.Utils.Logger.Log($"Building for Wii U...");

			string msysPath = outputPath.FullName
				.Replace("\\", "/");

			if (msysPath.Length >= 2 && msysPath[1] == ':')
			{
				char drive = char.ToLower(msysPath[0]);
				msysPath = "/" + drive + msysPath.Substring(2);
			}

			ProcessStartInfo buildInfo = new ProcessStartInfo();
			buildInfo.FileName = "C:\\msys64\\usr\\bin\\bash.exe";
			buildInfo.Arguments =
				$"-lc \"cd '{msysPath}' && " +
				"export DEVKITPRO=/opt/devkitpro && " +
				"export DEVKITPPC=$DEVKITPRO/devkitPPC && " +
				"export PATH=$DEVKITPRO/tools/bin:$DEVKITPPC/bin:$PATH && " +
				"export PKG_CONFIG_PATH=$DEVKITPRO/portlibs/wiiu/lib/pkgconfig && " +
				"rm -f build_wiiu/*.d && " +
				"make -f Makefile.wiiu 2>&1\"";

			buildInfo.WorkingDirectory = outputPath.FullName;
			buildInfo.CreateNoWindow = true;
			buildInfo.UseShellExecute = false;
			buildInfo.RedirectStandardOutput = true;
			buildInfo.RedirectStandardError = true;

			Process buildProcess = Process.Start(buildInfo);

			buildProcess.OutputDataReceived += (sender, e) =>
			{
				if (!string.IsNullOrEmpty(e.Data)) CTFAK.Utils.Logger.Log(e.Data);
			};

			buildProcess.ErrorDataReceived += (sender, e) =>
			{
				if (!string.IsNullOrEmpty(e.Data)) CTFAK.Utils.Logger.Log(e.Data);
			};

			buildProcess.BeginOutputReadLine();
			buildProcess.BeginErrorReadLine();
			buildProcess.WaitForExit();

			if (buildProcess.ExitCode != 0)
			{
				CTFAK.Utils.Logger.Log($"Build failed with exit code {buildProcess.ExitCode}");
				throw new Exception("Build failed");
			}
		}
		else if (buildType == BuildType.Wii)
		{
			DirectoryInfo buildDir = new DirectoryInfo(Path.Combine(outputPath.FullName, "build", "wii"));
			buildDir.Create();

			CTFAK.Utils.Logger.Log($"Building for Wii...");

			string msysPath = outputPath.FullName.Replace("\\", "/");

			if (msysPath.Length >= 2 && msysPath[1] == ':')
			{
				char drive = char.ToLower(msysPath[0]);
				msysPath = "/" + drive + msysPath.Substring(2);
			}

			ProcessStartInfo buildInfo = new ProcessStartInfo();
			buildInfo.FileName = "C:\\msys64\\usr\\bin\\bash.exe";
			buildInfo.Arguments =
				$"-lc \"cd '{msysPath}' && " +
				"export DEVKITPRO=/opt/devkitpro && " +
				"export DEVKITPPC=$DEVKITPRO/devkitPPC && " +
				"export PATH=$DEVKITPRO/tools/bin:$DEVKITPPC/bin:$PATH && " +
				"export PKG_CONFIG_PATH=$DEVKITPRO/portlibs/wii/lib/pkgconfig && " +
				"rm -f build_wii/*.d && " +
				"make -f Makefile.wii 2>&1\"";

			buildInfo.WorkingDirectory = outputPath.FullName;
			buildInfo.CreateNoWindow = true;
			buildInfo.UseShellExecute = false;
			buildInfo.RedirectStandardOutput = true;
			buildInfo.RedirectStandardError = true;

			Process buildProcess = Process.Start(buildInfo);

			buildProcess.OutputDataReceived += (sender, e) =>
			{
				if (!string.IsNullOrEmpty(e.Data)) CTFAK.Utils.Logger.Log(e.Data);
			};

			buildProcess.ErrorDataReceived += (sender, e) =>
			{
				if (!string.IsNullOrEmpty(e.Data)) CTFAK.Utils.Logger.Log(e.Data);
			};

			buildProcess.BeginOutputReadLine();
			buildProcess.BeginErrorReadLine();
			buildProcess.WaitForExit();

			if (buildProcess.ExitCode != 0)
			{
				CTFAK.Utils.Logger.Log($"Build failed with exit code {buildProcess.ExitCode}");
				throw new Exception("Build failed");
			}
		}
		else if (buildType == BuildType.PS2)
		{
			DirectoryInfo buildDir = new DirectoryInfo(Path.Combine(outputPath.FullName, "build", "ps2"));
			buildDir.Create();

			CTFAK.Utils.Logger.Log($"Building for PS2...");

			string wslPath = "/mnt/" + char.ToLower(outputPath.FullName[0]) +
				outputPath.FullName.Substring(2).Replace("\\", "/");

			ProcessStartInfo buildInfo = new ProcessStartInfo();
			buildInfo.FileName = "wsl.exe";
			buildInfo.Arguments = $"-e bash -c \"cd '{wslPath}' && " +
				"export PS2DEV=/usr/local/ps2dev && " +
				"export PS2SDK=$PS2DEV/ps2sdk && " +
				"export GSKIT=$PS2DEV/gsKit && " +
				"export PATH=$PATH:$PS2DEV/bin:$PS2DEV/ee/bin:$PS2DEV/iop/bin:$PS2DEV/dvp/bin:$PS2SDK/bin && " +
				"export PKG_CONFIG_PATH=$PS2SDK/ports/lib/pkgconfig && " +
				"rm -f build_ps2/*.d && " +
				"make -f Makefile.ps2 2>&1\"";

			buildInfo.WorkingDirectory = outputPath.FullName;
			buildInfo.CreateNoWindow = true;
			buildInfo.UseShellExecute = false;
			buildInfo.RedirectStandardOutput = true;
			buildInfo.RedirectStandardError = true;

			Process buildProcess = Process.Start(buildInfo);

			buildProcess.OutputDataReceived += (sender, e) =>
			{
				if (!string.IsNullOrEmpty(e.Data)) CTFAK.Utils.Logger.Log(e.Data);
			};

			buildProcess.ErrorDataReceived += (sender, e) =>
			{
				if (!string.IsNullOrEmpty(e.Data)) CTFAK.Utils.Logger.Log(e.Data);
			};

			buildProcess.BeginOutputReadLine();
			buildProcess.BeginErrorReadLine();
			buildProcess.WaitForExit();

			if (buildProcess.ExitCode != 0)
			{
				CTFAK.Utils.Logger.Log($"Build failed with exit code {buildProcess.ExitCode}");
				throw new Exception("Build failed");
			}
		}
		else if (buildType == BuildType.PSP)
		{
			DirectoryInfo buildDir = new DirectoryInfo(Path.Combine(outputPath.FullName, "build", "psp"));
			buildDir.Create();

			CTFAK.Utils.Logger.Log($"Building for PSP...");

			string wslPath = "/mnt/" + char.ToLower(outputPath.FullName[0]) + outputPath.FullName.Substring(2).Replace("\\", "/");

			ProcessStartInfo buildInfo = new ProcessStartInfo();
			buildInfo.FileName = "wsl.exe";
			buildInfo.Arguments = $"-e bash -c \"cd '{wslPath}' && " +
				"export PSPDEV=$HOME/pspdev && " +
				"export PSPSDK=$PSPDEV/psp/sdk && " +
				"export PATH=$PATH:$PSPDEV/bin:$PSPDEV/psp/bin && " +
				"export PKG_CONFIG_PATH=$PSPDEV/psp/lib/pkgconfig && " +
				"rm -f build_psp/*.d && " +
				"make -f Makefile.psp 2>&1\"";
			buildInfo.WorkingDirectory = outputPath.FullName;
			buildInfo.CreateNoWindow = true;
			buildInfo.UseShellExecute = false;
			buildInfo.RedirectStandardOutput = true;
			buildInfo.RedirectStandardError = true;

			Process buildProcess = Process.Start(buildInfo);

			buildProcess.OutputDataReceived += (sender, e) =>
			{
				if (!string.IsNullOrEmpty(e.Data)) CTFAK.Utils.Logger.Log(e.Data);
			};

			buildProcess.ErrorDataReceived += (sender, e) =>
			{
				if (!string.IsNullOrEmpty(e.Data)) CTFAK.Utils.Logger.Log(e.Data);
			};

			buildProcess.BeginOutputReadLine();
			buildProcess.BeginErrorReadLine();
			buildProcess.WaitForExit();

			if (buildProcess.ExitCode != 0)
			{
				CTFAK.Utils.Logger.Log($"Build failed with exit code {buildProcess.ExitCode}");
				throw new Exception("Build failed");
			}
		}
		else if (buildType == BuildType.N3DS)
		{
			DirectoryInfo buildDir = new DirectoryInfo(Path.Combine(outputPath.FullName, "build", "3ds"));
			buildDir.Create();

			CTFAK.Utils.Logger.Log($"Building for 3DS...");

			string msysPath = outputPath.FullName.Replace("\\", "/");

			if (msysPath.Length >= 2 && msysPath[1] == ':')
			{
				char drive = char.ToLower(msysPath[0]);
				msysPath = "/" + drive + msysPath.Substring(2);
			}

			ProcessStartInfo buildInfo = new ProcessStartInfo();
			buildInfo.FileName = "C:\\msys64\\usr\\bin\\bash.exe";
			buildInfo.Arguments =
				$"-lc \"cd '{msysPath}' && " +
				"export DEVKITPRO=/opt/devkitpro && " +
				"export DEVKITARM=$DEVKITPRO/devkitARM && " +
				"export PATH=$DEVKITPRO/tools/bin:$DEVKITARM/bin:$PATH && " +
				"export PKG_CONFIG_PATH=$DEVKITPRO/portlibs/3ds/lib/pkgconfig && " +
				"make -f Makefile.3ds 2>&1\"";

			buildInfo.WorkingDirectory = outputPath.FullName;
			buildInfo.CreateNoWindow = true;
			buildInfo.UseShellExecute = false;
			buildInfo.RedirectStandardOutput = true;
			buildInfo.RedirectStandardError = true;

			Process buildProcess = Process.Start(buildInfo);

			buildProcess.OutputDataReceived += (sender, e) =>
			{
				if (!string.IsNullOrEmpty(e.Data)) CTFAK.Utils.Logger.Log(e.Data);
			};

			buildProcess.ErrorDataReceived += (sender, e) =>
			{
				if (!string.IsNullOrEmpty(e.Data)) CTFAK.Utils.Logger.Log(e.Data);
			};

			buildProcess.BeginOutputReadLine();
			buildProcess.BeginErrorReadLine();
			buildProcess.WaitForExit();

			if (buildProcess.ExitCode != 0)
			{
				CTFAK.Utils.Logger.Log($"Build failed with exit code {buildProcess.ExitCode}");
				throw new Exception("Build failed");
			}
		}
	}
}
