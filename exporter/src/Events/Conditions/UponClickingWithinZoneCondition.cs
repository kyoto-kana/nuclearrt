using CTFAK.CCN.Chunks.Frame;
using CTFAK.MMFParser.EXE.Loaders.Events.Parameters;

public class UponClickingWithinZoneCondition : ConditionBase
{
	public override int[] ObjectType { get; set; } = [-6];
	public override int Num { get; set; } = -6;

	public override string Build(EventBase eventBase, ref string nextLabel, ref int orIndex, Dictionary<string, object>? parameters = null, string ifStatement = "if (")
	{
		Click click = (Click)eventBase.Items[0].Loader;
		Zone zone = (Zone)eventBase.Items[1].Loader;

		int button = click.Button;
		if (button == 0)
			button = 1;
		else if (button == 1)
			button = 4;
		else if (button == 4)
			button = 1;

		bool isDouble = click.IsDouble != 0;
		return $"{ifStatement} (Application::Instance().GetInput()->IsMouseButtonPressed({button}, {isDouble.ToString().ToLower()}) && GetMouseX() >= {zone.X1} && GetMouseX() <= {zone.X2} && GetMouseY() >= {zone.Y1} && GetMouseY() <= {zone.Y2})) goto {nextLabel};";
	}
}