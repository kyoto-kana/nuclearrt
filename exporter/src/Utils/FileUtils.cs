using System.IO;

public static class FileUtils
{
	public static void CopyFilesRecursively(string sourcePath, string targetPath)
	{
		Directory.CreateDirectory(targetPath);

		foreach (string dirPath in Directory.GetDirectories(sourcePath, "*", SearchOption.AllDirectories))
		{
			Directory.CreateDirectory(dirPath.Replace(sourcePath, targetPath));
		}

		foreach (string sourceFilePath in Directory.GetFiles(sourcePath, "*.*", SearchOption.AllDirectories))
		{
			string targetFilePath = sourceFilePath.Replace(sourcePath, targetPath);

			Directory.CreateDirectory(Path.GetDirectoryName(targetFilePath)!);

			if (!File.Exists(targetFilePath) ||
				!File.ReadAllBytes(sourceFilePath).SequenceEqual(File.ReadAllBytes(targetFilePath)))
			{
				File.Copy(sourceFilePath, targetFilePath, true);
			}
		}
	}

	public static void SaveFile(string path, string content)
	{
		// check if the content is different from the file (helps with compile times)
		if (!File.Exists(path))
		{
			File.WriteAllText(path, content);
		}
		else
		{
			string currentContent = File.ReadAllText(path);
			if (currentContent != content)
			{
				File.WriteAllText(path, content);
			}
		}
	}
}
