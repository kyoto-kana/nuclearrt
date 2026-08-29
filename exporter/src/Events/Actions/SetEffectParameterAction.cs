using System.Text;
using CTFAK.CCN;
using CTFAK.CCN.Chunks.Frame;
using CTFAK.MMFParser.EXE.Loaders.Events.Parameters;

public class SetEffectParameterAction : ActionBase
{
	public override int[] ObjectType { get; set; } = [2, 3, 7];
	public override int Num { get; set; } = 64;

	public override string Build(EventBase eventBase, ref string nextLabel, ref int orIndex, Dictionary<string, object>? parameters = null, string ifStatement = "if (")
	{
		StringBuilder result = new StringBuilder();

		result.AppendLine($"for (ObjectIterator it({GetSelector(eventBase.ObjectInfo, eventBase.ObjectType)}); !it.end(); ++it) {{");
		result.AppendLine($"	auto instance = *it;");
		result.AppendLine($"	instance->SetEffectInstanceParameter({ConvertExpression(eventBase, 0)}, {ConvertExpression(eventBase, 1)});");
		result.AppendLine("}");

		return result.ToString();
	}
}
