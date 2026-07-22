using System.Text;
using CTFAK.CCN.Chunks.Frame;
using CTFAK.MMFParser.EXE.Loaders.Events.Parameters;

public class AnimationOverCondition : ConditionBase
{
	public override int[] ObjectType { get; set; } = [2];
	public override int Num { get; set; } = -2;

	public override string Build(EventBase eventBase, ref string nextLabel, ref int orIndex, Dictionary<string, object>? parameters = null, string ifStatement = "if (")
	{
		StringBuilder result = new();

		string value;
		if (eventBase.Items[0].Loader is Short shortValue)
			value = shortValue.Value.ToString();
		else
			value = ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase);

		result.AppendLine($"for (ObjectIterator it(*{GetSelector(eventBase.ObjectInfo, eventBase.ObjectType)}); !it.end(); ++it) {{");
		result.AppendLine($"    auto instance = *it;");
		result.AppendLine($"    if (!((Active*)instance)->animations.IsSequenceOver({value})) it.deselect();");
		result.AppendLine("}");

		result.AppendLine($"if ({GetSelector(eventBase.ObjectInfo, eventBase.ObjectType)}->Count() == 0) goto {nextLabel};");

		return result.ToString();
	}
}
