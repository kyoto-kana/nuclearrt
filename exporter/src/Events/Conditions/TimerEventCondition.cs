using CTFAK.CCN.Chunks.Frame;

public class TimerEventCondition : ConditionBase
{
	public override int[] ObjectType { get; set; } = [-4];
	public override int Num { get; set; } = -6;

	public override string Build(EventBase eventBase, ref string nextLabel, ref int orIndex, Dictionary<string, object>? parameters = null, string ifStatement = "if (")
	{
		return "";
	}
}
