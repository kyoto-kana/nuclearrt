using CTFAK.CCN.Chunks.Frame;

public class SetWindowWindowedAction : ActionBase
{
	public override int[] ObjectType { get; set; } = [-3];
	public override int Num { get; set; } = 15;

	public override string Build(EventBase eventBase, ref string nextLabel, ref int orIndex, Dictionary<string, object>? parameters = null, string ifStatement = "if (")
	{
		return $"Application::Instance().GetBackend()->platform->SetFullscreen(false);";
	}
}
