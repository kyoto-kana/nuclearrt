using System.Text;
using CTFAK.CCN.Chunks.Frame;
using CTFAK.Memory;
using CTFAK.MMFParser.EXE.Loaders.Events.Expressions;
using CTFAK.MMFParser.EXE.Loaders.Events.Parameters;

public class PerspectiveExporter : ExtensionExporter
{
	public override string ObjectIdentifier => "tksp";
	public override string ExtensionName => "Perspective";
	public override string CppClassName => "PerspectiveExtention";

	public override string ExportExtension(byte[] extensionData)
	{
		ByteReader reader = new(extensionData);

		reader.Skip(4);
		short width = reader.ReadInt16();
		short height = reader.ReadInt16();
		byte effect = reader.ReadByte();
        string direction = reader.ReadBoolean().ToString().ToLower();
		reader.Skip(2);
		int zoomValue = reader.ReadInt32();
        int offset = reader.ReadInt32();
        int sineWaveWaves = reader.ReadInt32();
        string perspectiveDir = reader.ReadBoolean().ToString().ToLower();

		return CreateExtension($"{width}, {height}, {effect}, {direction}, {zoomValue}, {offset}, {sineWaveWaves}, {perspectiveDir}");
	}

	public override string ExportCondition(EventBase eventBase, int conditionNum, ref string nextLabel, ref int orIndex, Dictionary<string, object>? parameters = null, string ifStatement = "if (", bool isGlobal = false)
	{
		return string.Empty;
	}

	public override string ExportAction(EventBase eventBase, int actionNum, ref string nextLabel, ref int orIndex, Dictionary<string, object>? parameters = null, bool isGlobal = false)
	{
		StringBuilder result = new();

		switch (actionNum)
		{
			case 0: // Set Zoom Value
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->SetZoomValue({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)});");
				break;
			case 1: // Set effect to Panorama
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->SetEffectType(0);");
				break;
			case 2: // Set effect to Perspective
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->SetEffectType(1);");
				break;
			case 3: // Set effect to Sine Waves
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->SetEffectType(2);");
				break;
			case 5: // Set Sine Wave Waves
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->SetSineWaveWaves({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)});");
				break;
			case 6: // Set Offset
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->SetOffset({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)});");
				break;
			case 7: // Set Horizontal
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->SetDirection(false);");
				break;
			case 8: // Set Vertical
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->SetDirection(true);");
				break;
			case 9: // Set Perspective Direction
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->SetPerspectiveDirection(false);");
				break;
			case 10: // Set Perspective Direction
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->SetPerspectiveDirection(true);");
				break;
			case 12: // Set Width
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->SetWidth({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)});");
				break;
			case 13: // Set Height
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->SetHeight({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)});");
				break;
			case 16: // Set effect to Sine Offset
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->SetEffectType(3);");
				break;
			default:
				result.AppendLine($"// Perspective action {actionNum} not implemented");
				break;
		}

		return result.ToString();
	}

	public override string ExportExpression(Expression expression, EventBase eventBase = null)
	{
		string result;

		switch (expression.Num)
		{
			case 0: // Get Zoom Value
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->GetZoomValue()";
				break;
			case 1: // Get Offset
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->GetOffset()";
				break;
			case 2: // Get Sine Wave Waves
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->GetSineWaveWaves()";
				break;
			case 4: // Get Width
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->GetWidth()";
				break;
			case 5: // Get Height
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->GetHeight()";
				break;
			default:
				result = $"0 /* Perspective expression {expression.Num} not implemented */";
				break;
		}

		return result;
	}
}
