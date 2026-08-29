using CTFAK.Memory;
using CTFAK.CCN.Chunks.Frame;
using System.Text;
using CTFAK.Utils;

public class CrasherExporter : ExtensionExporter
{
	public override string ObjectIdentifier => "hsrc";
	public override string ExtensionName => "Crasher";
	public override string CppClassName => "CrasherExtension";

	public override string ExportExtension(byte[] extensionData) => string.Empty;

	public override string ExportAction(EventBase eventBase, int actionNum, ref string nextLabel, ref int orIndex, Dictionary<string, object>? parameters = null, bool isGlobal = false)
	{
		StringBuilder result = new StringBuilder();

		switch (actionNum)
		{
			case 0:
				result.AppendLine("{");
				result.AppendLine("    #ifndef PLATFORM_WEB");
				result.AppendLine("    volatile int numerator = 10;");
				result.AppendLine("    volatile int denominator = 5;");
				result.AppendLine("    denominator = 0;");
				result.AppendLine("    volatile int result = numerator / denominator;");
				result.AppendLine("    #else");
				result.AppendLine("    Application::Instance().QueueStateChange(GameState::RestartApplication);");
				result.AppendLine("    #endif");
				result.AppendLine("}");
				break;
		}

		return result.ToString();
	}
}
