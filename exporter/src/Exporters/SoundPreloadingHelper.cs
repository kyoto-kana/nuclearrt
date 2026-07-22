using CTFAK.CCN.Chunks.Frame;
using CTFAK.MMFParser.EXE.Loaders.Events.Parameters;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

public static class SoundPreloadingHelper
{
	public static bool TryGetLiteralSample(EventBase eventBase, out Sample sample)
	{
        sample = null!;

        if (eventBase.Items.Count == 0)
            return false;

        if (eventBase.Items[0].Loader is not Sample parsedSample)
            return false;

        sample = parsedSample;
        return true;
	}

	public static bool IsSampleAction(EventBase action)
	{
		if (action.ObjectType != -2)
			return false;

		return action.Num switch
		{
			0 or 4 or 6 or 7 or 8 or 11 or 12 or 19 or 21 or 23 or 33 or 36 => true,
			_ => false
		};
	}
}
