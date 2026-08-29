using System.Text;
using CTFAK.CCN.Chunks.Frame;
using CTFAK.MMFParser.EXE.Loaders.Events.Expressions;
using CTFAK.MMFParser.EXE.Loaders.Events.Parameters;

public class LayerObjectExporter : ExtensionExporter
{
	public override string ObjectIdentifier => "RYAL";
	public override string ExtensionName => "Layer";
	public override string CppClassName => "LayerObjectExtension";

	public override string ExportExtension(byte[] extensionData)
	{
		return CreateExtension("");
	}

	public override string ExportCondition(EventBase eventBase, int conditionNum, ref string nextLabel, ref int orIndex, Dictionary<string, object>? parameters = null, string ifStatement = "if (", bool isGlobal = false)
	{
		StringBuilder result = new();

		switch (conditionNum)
		{
			case 11:
				result.AppendLine($"{ifStatement} {GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->IsLayerVisible(({EvaluateExpression(eventBase, 0)}).GetIntValue()-1)) goto {nextLabel};");
				break;
			case 12:
				result.AppendLine($"{ifStatement} {GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->IsLayerVisible(({EvaluateExpression(eventBase, 0)}).GetStringValue())) goto {nextLabel};");
				break;
			default:
				result.AppendLine($"// Layer Object condition {conditionNum} not implemented");
				result.AppendLine($"goto {nextLabel};");
				break;
		}

		return result.ToString();
	}

	public override string ExportAction(EventBase eventBase, int actionNum, ref string nextLabel, ref int orIndex, Dictionary<string, object>? parameters = null, bool isGlobal = false)
	{
		StringBuilder result = new();

		switch (actionNum)
		{
			case 12:
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->XSort(false);");
				break;
			case 13:
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->YSort(false);");
				break;
			case 14:
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->XSort();");
				break;
			case 15:
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->YSort();");
				break;
			case 26:
				{
					int altIndex = ((AlterableValue)eventBase.Items[0].Loader).Value;
					string defaultValue = ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[1].Loader, eventBase);
					result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->AltValueSort(false, {altIndex}, {defaultValue});");
				}
				break;
			case 27:
				{
					int altIndex = ((AlterableValue)eventBase.Items[0].Loader).Value;
					string defaultValue = ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[1].Loader, eventBase);
					result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->AltValueSort(true, {altIndex}, {defaultValue});");
				}
				break;
			case 31:
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->ShowLayer(({EvaluateExpression(eventBase, 0)}).GetIntValue()-1);");
				break;
			case 32:
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->HideLayer(({EvaluateExpression(eventBase, 0)}).GetIntValue()-1);");
				break;
			case 36:
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->ShowLayer(({EvaluateExpression(eventBase, 0)}).GetStringValue());");
				break;
			case 37:
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->HideLayer(({EvaluateExpression(eventBase, 0)}).GetStringValue());");
				break;
			default:
				result.AppendLine($"// Layer Object action {actionNum} not implemented");
				break;
		}

		return result.ToString();
	}

	public override string ExportExpression(Expression expression, EventBase eventBase = null)
	{
		string result;

		switch (expression.Num)
		{
			case 23:
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->GetLayerEffectParameter(";
				break;
			default:
				result = $"0 /* Layer Object expression {expression.Num} not implemented */";
				break;
		}

		return result;
	}
}
