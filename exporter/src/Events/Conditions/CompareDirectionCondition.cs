using System.Text;
using CTFAK.CCN.Chunks.Frame;
using CTFAK.MMFParser.EXE.Loaders.Events.Parameters;

public class CompareDirectionCondition: ConditionBase
{
	public override int[] ObjectType { get; set; } = [2];
	public override int Num { get; set; } = -8;

	public override string Build(EventBase eventBase, ref string nextLabel, ref int orIndex, Dictionary<string, object>? parameters = null, string ifStatement = "if (")
	{
		StringBuilder result = new();

		result.AppendLine($"for (ObjectIterator it(*{GetSelector(eventBase.ObjectInfo, eventBase.ObjectType)}); !it.end(); ++it) {{");
		result.AppendLine($"    auto instance = *it;");
		if (eventBase.Items[0].Loader is IntParam direction)
		{
			result.AppendLine($"    {ifStatement} ((({ExpressionConverter.GetObjectClassName(eventBase.ObjectInfo, eventBase.ObjectType)}*)instance)->animations.IsFacingDirectionMask({direction.Value}))) it.deselect();");
		}
		else
		{
			result.AppendLine($"    {ifStatement} ((({ExpressionConverter.GetObjectClassName(eventBase.ObjectInfo, eventBase.ObjectType)}*)instance)->animations.IsFacingDirection({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)}))) it.deselect();");
		}
		result.AppendLine("}");

		result.AppendLine($"if ({GetSelector(eventBase.ObjectInfo, eventBase.ObjectType)}->Count() == 0) goto {nextLabel};");

		return result.ToString();
	}
}
