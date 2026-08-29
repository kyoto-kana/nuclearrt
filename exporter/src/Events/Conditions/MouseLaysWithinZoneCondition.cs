using CTFAK.CCN.Chunks.Frame;
using CTFAK.MMFParser.EXE.Loaders.Events.Parameters;

public class MouseLaysWithinZoneCondition : ConditionBase
{
	public override int[] ObjectType { get; set; } = [-6];
	public override int Num { get; set; } = -3;

	public override string Build(EventBase eventBase, ref string nextLabel, ref int orIndex, Dictionary<string, object>? parameters = null, string ifStatement = "if (")
	{
		Zone zone = (Zone)eventBase.Items[0].Loader;

		return $"{ifStatement} (GetMouseX() >= {zone.X1} && GetMouseX() <= {zone.X2} && GetMouseY() >= {zone.Y1} && GetMouseY() <= {zone.Y2})) goto {nextLabel};";
	}
}
