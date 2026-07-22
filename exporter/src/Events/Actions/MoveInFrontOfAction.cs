using System.Text;
using CTFAK.CCN.Chunks.Frame;
using CTFAK.MMFParser.EXE.Loaders.Events.Parameters;

public class MoveInFrontOfAction : ActionBase
{
	public override int[] ObjectType { get; set; } = [2];
	public override int Num { get; set; } = 60;

	public override string Build(EventBase eventBase, ref string nextLabel, ref int orIndex, Dictionary<string, object>? parameters = null, string ifStatement = "if (")
	{
		StringBuilder result = new StringBuilder();

		ParamObject paramObject = (ParamObject)eventBase.Items[0].Loader;

		result.AppendLine($"for (ObjectIterator it(*{GetSelector(eventBase.ObjectInfo, eventBase.ObjectType)}); !it.end(); ++it) {{");
		result.AppendLine($"    auto instance = *it;");
		result.AppendLine($"    MoveObjectInFrontOf(instance, ({ExpressionConverter.GetObject(paramObject.ObjectInfo, paramObject.ObjectType).Item1}));");
		result.AppendLine("}");

		return result.ToString();
	}
}
