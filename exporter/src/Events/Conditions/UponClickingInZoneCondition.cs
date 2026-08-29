using CTFAK.CCN.Chunks.Frame;
using CTFAK.MMFParser.EXE.Loaders.Events.Parameters;

public class UponClickingInZoneCondition : ConditionBase
{
	public override int[] ObjectType { get; set; } = [-6];
	public override int Num { get; set; } = -6;
	public override bool IsTrueEvent => true;

	public override string Build(EventBase eventBase, ref string nextLabel, ref int orIndex, Dictionary<string, object>? parameters = null, string ifStatement = "if (")
	{
		int button = ((Click)eventBase.Items[0].Loader).Button;
		if (button == 0)
			button = 1;
		else if (button == 1)
			button = 4;
		else if (button == 4)
			button = 1;

		bool isDouble = ((Click)eventBase.Items[0].Loader).IsDouble != 0;
		Zone zone = (Zone)eventBase.Items[1].Loader;

		return $"{ifStatement} (Application::Instance().GetInput()->IsMouseButtonPressed({button}, {isDouble.ToString().ToLower()}))) goto {nextLabel};\n" +
			$"{ifStatement} (GetMouseX() >= {zone.X1} && GetMouseX() <= {zone.X2} && GetMouseY() >= {zone.Y1} && GetMouseY() <= {zone.Y2})) goto {nextLabel};";
	}
}
