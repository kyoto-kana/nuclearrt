using System.Text;
using CTFAK.CCN.Chunks.Frame;
using CTFAK.MMFParser.EXE.Loaders.Events.Parameters;
using CTFAK.Utils;

public class JumpFrameAction : ActionBase
{
	public override int[] ObjectType { get; set; } = [-3];
	public override int Num { get; set; } = 2;

	public override string Build(EventBase eventBase, ref string nextLabel, ref int orIndex, Dictionary<string, object>? parameters = null, string ifStatement = "if (")
	{
		string frame;
		if (eventBase.Items[0].Loader is ExpressionParameter)
		{
			frame = $"({ConvertExpression(eventBase, 0)}).GetIntValue() - 1";
		}
		else
		{
			short index = ((Short)eventBase.Items[0].Loader).Value;
			if (index < 0 || index >= Exporter.Instance.GameData.frameHandles.Items.Count)
			{
				Logger.Log($"JumpFrameAction: Invalid frame index: {index}");
				frame = "0";
			}
			else
			{
				frame = Exporter.Instance.GameData.frameHandles.Items[index].ToString();
			}
		}

		return $"Application::Instance().QueueStateChange(GameState::JumpToFrame, {frame});";
	}
}
