using System.Text;
using CTFAK.CCN.Chunks.Frame;
using CTFAK.MMFParser.EXE.Loaders.Events.Parameters;

public class PickLowestExpressionCondition : ConditionBase
{
	public override int[] ObjectType { get; set; } = [2, 7];
	public override int Num { get; set; } = -47;

	public override string Build(EventBase eventBase, ref string nextLabel, ref int orIndex, Dictionary<string, object>? parameters = null, string ifStatement = "if (")
	{
		StringBuilder result = new();

		result.AppendLine("{");

		result.AppendLine($"std::vector<ObjectInstance*> lowestInstances;");
		result.AppendLine($"CValue lowestExpression = nullptr;");

		result.AppendLine($"for (ObjectIterator it({GetSelector(eventBase.ObjectInfo, eventBase.ObjectType)}); !it.end(); ++it) {{");
		result.AppendLine($"    auto instance = *it;");
		result.AppendLine($"    CValue expression = {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)};");
		result.AppendLine($"    if (lowestExpression == nullptr || expression < lowestExpression) {{");
		result.AppendLine($"        lowestExpression = expression;");
		result.AppendLine($"        lowestInstances.clear();");
		result.AppendLine($"        lowestInstances.push_back(instance);");
		result.AppendLine($"    }} else if (expression == lowestExpression) {{");
		result.AppendLine($"        lowestInstances.push_back(instance);");
		result.AppendLine($"    }}");
		result.AppendLine("}");

		result.AppendLine("if (lowestInstances.empty())");
		result.AppendLine($"    goto {nextLabel};");
		result.AppendLine("else");
		result.AppendLine("{");
		result.AppendLine($"    {GetSelector(eventBase.ObjectInfo, eventBase.ObjectType)}.DeselectAll();");
		result.AppendLine($"    for (auto instance : lowestInstances) {{");
		result.AppendLine($"        {GetSelector(eventBase.ObjectInfo, eventBase.ObjectType)}.Select(instance);");
		result.AppendLine($"    }}");
		result.AppendLine("}");

		result.AppendLine("}");

		return result.ToString();
	}
}
