using System.Text;
using CTFAK.CCN.Chunks.Frame;

public class OrLogicalCondition : ConditionBase
{
	public override int[] ObjectType { get; set; } = [-1];
	public override int Num { get; set; } = -25;

	public override string Build(EventBase eventBase, ref string nextLabel, ref int orIndex, Dictionary<string, object>? parameters = null, string ifStatement = "if (")
	{
		StringBuilder result = new();

		//if we got here then the conditions mustve succeeded, so we can skip to the actions
		result.AppendLine($"goto {parameters["eventBaseName"]}_actions;");

		result.AppendLine($"{parameters["eventBaseName"]}_or_{orIndex}:;");

		orIndex++;
		nextLabel = EventProcessor.GenerateEventNextLabel(parameters["eventGroup"] as EventGroup, orIndex, (int)parameters["numOfOrs"]);

		//Reset instances
		var inheritedSelectors = (List<Tuple<int, int, string>>)parameters["inheritedSelectors"];

		foreach (var relevantObjectInfo in EventProcessor.GetRelevantObjectInfos(parameters["eventGroup"] as EventGroup))
		{
			if (inheritedSelectors.Any(x => x.Item1 == relevantObjectInfo.Item1 && x.Item2 == relevantObjectInfo.Item2))
				result.AppendLine($"{StringUtils.SanitizeObjectName(relevantObjectInfo.Item3)}_{relevantObjectInfo.Item1}_selector.RestoreSelection(__entry_{StringUtils.SanitizeObjectName(relevantObjectInfo.Item3)}_{relevantObjectInfo.Item1});");
			else
				result.AppendLine($"{StringUtils.SanitizeObjectName(relevantObjectInfo.Item3)}_{relevantObjectInfo.Item1}_selector.Reset(trueEventSource);");
		}

		return result.ToString();
	}
}
