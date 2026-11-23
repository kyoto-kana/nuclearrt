using CTFAK.Memory;
using CTFAK.CCN.Chunks.Frame;
using System.Text;
using CTFAK.MMFParser.EXE.Loaders.Events.Expressions;
using CTFAK.MMFParser.EXE.Loaders.Events.Parameters;

public class EncryptionObjectExporter : ExtensionExporter
{
	public override string ObjectIdentifier => "rCne";
	public override string ExtensionName => "EncryptionObject";
	public override string CppClassName => "EncryptionObjectExtension";

	public override string ExportExtension(byte[] extensionData)
	{
		// No props
		return string.Empty;
	}

	public override string ExportCondition(EventBase eventBase, int conditionNum, ref string nextLabel, ref int orIndex, Dictionary<string, object>? parameters = null, string ifStatement = "if (", bool isGlobal = false)
	{
		// No conditions
		return string.Empty;
	}

	public override string ExportAction(EventBase eventBase, int actionNum, ref string nextLabel, ref int orIndex, Dictionary<string, object>? parameters = null, bool isGlobal = false)
	{
		// No actions
		return string.Empty;
	}

	public override string ExportExpression(Expression expression, EventBase eventBase = null)
	{
		string result;

		switch (expression.Num)
		{
			case 0: // Encrypt
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->Encrypt(";
				break;
			case 1: // Decrypt
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->Decrypt(";
				break;
			default:
				result = $"(/* Encryption object expression {expression.Num} not implemented */";
				break;
		}

		return result;
	}
}
