using System.Text;
using CTFAK.CCN.Chunks.Frame;
using CTFAK.MMFParser.EXE.Loaders.Events.Parameters;

public class SetParagraphAction : ActionBase
{
	public override int[] ObjectType { get; set; } = [3];
	public override int Num { get; set; } = 84;

	public override string Build(EventBase eventBase, ref string nextLabel, ref int orIndex, Dictionary<string, object>? parameters = null, string ifStatement = "if (")
	{
		StringBuilder result = new StringBuilder();

		result.AppendLine($"for (ObjectIterator it(*{GetSelector(eventBase.ObjectInfo, eventBase.ObjectType)}); !it.end(); ++it) {{");
		result.AppendLine($"  auto instance = *it;");
		if (eventBase.Items[0].Loader is Short shortValue)
		{
			result.AppendLine($"  ((StringObject*)instance)->SetCurrentParagraph({shortValue.Value});");
		}
		else if (eventBase.Items[0].Loader is ExpressionParameter expressionParameter)
		{
			result.AppendLine($"  ((StringObject*)instance)->SetCurrentParagraph({ExpressionConverter.ConvertExpression(expressionParameter, eventBase)} - 1);");
		}
		result.AppendLine("}");

		return result.ToString();
	}
}
