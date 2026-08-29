using System.Text;
using CTFAK.CCN.Chunks;
using CTFAK.CCN.Chunks.Frame;
using CTFAK.MMFParser.EXE.Loaders.Events.Parameters;
using CTFAK.Utils;

public class TimerFireEventAction : ActionBase
{
	public override int[] ObjectType { get; set; } = [-4];
	public override int Num { get; set; } = 1;

	public override string Build(EventBase eventBase, ref string nextLabel, ref int orIndex, Dictionary<string, object>? parameters = null, string ifStatement = "if (")
	{
		string delay;
		if (eventBase.Items[0].Loader is Time time)
			delay = time.Timer.ToString();
		else
			delay = $"({ConvertExpression(eventBase, 0)}).GetIntValue()";

		string eventName = ConvertExpression(eventBase, 1);

		return $"GameTimer.StartTimerEvent(ToLowerStr({eventName}.GetStringValue()), 1, {delay}, 0, false);";
	}
}

public class TimerFireEventNTimesAction : TimerFireEventAction
{
	public override int Num { get; set; } = 2;

	public override string Build(EventBase eventBase, ref string nextLabel, ref int orIndex, Dictionary<string, object>? parameters = null, string ifStatement = "if (")
	{
		string delay;
		if (eventBase.Items[0].Loader is Time time)
			delay = time.Timer.ToString();
		else
			delay = $"({ConvertExpression(eventBase, 0)}).GetIntValue()";

		string loopCount = ConvertExpression(eventBase, 1);

		string interval;
		if (eventBase.Items[2].Loader is Time intervalTime)
			interval = intervalTime.Timer.ToString();
		else
			interval = $"({ConvertExpression(eventBase, 2)}).GetIntValue()";

		string eventName = ConvertExpression(eventBase, 3);

		return $"GameTimer.StartTimerEvent(ToLowerStr({eventName}.GetStringValue()), {loopCount}.GetIntValue(), {delay}, {interval}, true);";
	}
}
