using CTFAK.CCN.Chunks.Frame;

public class ACBase
{
	public virtual int[] ObjectType { get; set; } = [];
	public virtual int Num { get; set; }

	//if statement and next label are only used for conditions
	public virtual string Build(EventBase eventBase, ref string nextLabel, ref int orIndex, Dictionary<string, object>? parameters = null, string ifStatement = "if (")
	{
		return $"//{GetType().Name} not implemented. ObjectType: {ObjectType}, Num: {Num}";
	}

	public string GetSelector(int objectInfo, int objectType)
	{
		return ExpressionConverter.GetSelector(objectInfo, objectType);
	}

	public override bool Equals(object? obj)
	{
		if (obj.GetType().IsSubclassOf(typeof(EventBase)))
		{
			EventBase eventBase = obj as EventBase;
			return (ObjectType.Contains(eventBase.ObjectType) || (eventBase.ObjectType >= 32 && ObjectType.Any(x => x >= 32))) && eventBase.Num == Num;
		}
		return base.Equals(obj);
	}
}
