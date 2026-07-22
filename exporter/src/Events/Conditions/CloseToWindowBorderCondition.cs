using System.Text;
using CTFAK.CCN.Chunks.Frame;
using CTFAK.MMFParser.EXE.Loaders.Events.Parameters;

public class CloseToWindowBorderCondition : ConditionBase
{
	public override int[] ObjectType { get; set; } = [2, 3, 7, 32];
	public override int Num { get; set; } = -22;

	public override string Build(EventBase eventBase, ref string nextLabel, ref int orIndex, Dictionary<string, object>? parameters = null, string ifStatement = "if (")
	{
		StringBuilder result = new();

		string border = $"{ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)}";

		bool negated = ifStatement == "if (!";

		result.AppendLine($"for (ObjectIterator it(*{GetSelector(eventBase.ObjectInfo, eventBase.ObjectType)}); !it.end(); ++it) {{");
		result.AppendLine($"    auto instance = *it;");
		result.AppendLine($"    CollisionInstanceBounds bounds = GetInstanceBounds(instance);");
		result.AppendLine($"    if (bounds.minX {(negated ? ">" : "<=")} GetXLeftEdge() + {border} &&");
		result.AppendLine($"        bounds.maxX {(negated ? "<" : ">=")} GetXRightEdge() - {border} &&");
		result.AppendLine($"        bounds.minY {(negated ? ">" : "<=")} GetYTopEdge() + {border} &&");
		result.AppendLine($"        bounds.maxY {(negated ? "<" : ">=")} GetYBottomEdge() - {border})");
		result.AppendLine($"        it.deselect();");
		result.AppendLine("}");

		//If no instances are selected, we go to the end label
		result.AppendLine($"if ({GetSelector(eventBase.ObjectInfo, eventBase.ObjectType)}->Count() == 0) goto {nextLabel};");

		return result.ToString();
	}
}
