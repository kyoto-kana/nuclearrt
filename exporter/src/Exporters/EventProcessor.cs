using System.Reflection;
using System.Text;
using CTFAK.CCN.Chunks.Frame;
using CTFAK.CCN.Chunks.Objects;
using CTFAK.MFA;
using CTFAK.MMFParser.EXE.Loaders.Events.Expressions;
using CTFAK.MMFParser.EXE.Loaders.Events.Parameters;
using CTFAK.Utils;

public class EventProcessor
{
	private readonly Exporter _exporter;

	public enum EventLoopType
	{
		Animation,
		Timer,
		Normal,
	}

	private readonly Dictionary<int, HashSet<string>> _literalSampleNamesByFrame = new();

	public IReadOnlyCollection<string> GetLiteralSampleNamesUsed(int frameIndex)
	{
		return _literalSampleNamesByFrame.TryGetValue(frameIndex, out var set)
			? set
			: Array.Empty<string>();
	}

	private void RegisterLiteralSampleName(int frameIndex, string name)
	{
		if (!_literalSampleNamesByFrame.TryGetValue(frameIndex, out var set))
		{
			set = new HashSet<string>();
			_literalSampleNamesByFrame[frameIndex] = set;
		}

		set.Add(name.Replace("\0", ""));
	}

	public EventProcessor(Exporter exporter)
	{
		_exporter = exporter;
	}

	public bool ShouldSkipEvent(EventGroup evt)
	{
		if ((evt.Flags & 16384) != 0) return true; // Inactive line

		foreach (var condition in evt.Conditions)
		{
			if (new CommentCondition().Equals(condition) || new GroupStartCondition().Equals(condition) || new GroupEndCondition().Equals(condition)) return true;
		}

		return false;
	}

	public string BuildEventUpdateLoop(int frameIndex, EventLoopType eventLoopType)
	{
		var result = new StringBuilder();

		for (int j = 0; j < _exporter.GameData.Frames[frameIndex].events.Items.Count; j++)
		{
			var evt = _exporter.GameData.Frames[frameIndex].events.Items[j];
			string eventName = GetEventName(evt);

			for (int k = 0; k < evt.RestrictCpt; k++) //TODO: check if this is correct
			{
				result.Append("\t");
			}

			// TODO: if a group is empty, don't include it.
			if (ShouldSkipEvent(evt))
			{
				if (new GroupStartCondition().Equals(evt.Conditions[0])) //if this event is a group start, don't include it in the main event update loop
				{
					result.Append($"if (IsGroupActive({(evt.Conditions[0].Items[0].Loader as Group).Id})) {{\n");
				}
				else if (new GroupEndCondition().Equals(evt.Conditions[0])) //if this event is a group end, don't include it in the main event update loop, just close the current group
				{
					result.Remove(result.Length - 1, 1); //Remove the last tab
					result.Append("}\n");
				}

				continue;
			}

			//only add event to normal event loop if it doesn't have a loop condition
			if (DoesEventHaveLoop(evt) == null)
			{
				if (eventLoopType == EventLoopType.Timer)
				{
					if (!IsTimerEvent(evt)) continue;
				}
				else if (eventLoopType == EventLoopType.Animation)
				{
					if (!IsAnimationEvent(evt)) continue;
				}
				else if (eventLoopType == EventLoopType.Normal)
				{
					if (IsTimerEvent(evt) || IsAnimationEvent(evt)) continue;
				}

				result.Append($"{eventName}();\n");
			}
		}
		return result.ToString();
	}

	public string BuildEventFunctions(int frameIndex)
	{
		var result = new StringBuilder();

		for (int j = 0; j < _exporter.GameData.Frames[frameIndex].events.Items.Count; j++)
		{
			var evt = _exporter.GameData.Frames[frameIndex].events.Items[j];

			if (ShouldSkipEvent(evt)) continue;

			result.AppendLine($"void GeneratedFrame{frameIndex}::{GetEventName(evt)}()");
			result.AppendLine("{");

			if (DoesEventHaveOneActionLoop(evt))
			{
				result.AppendLine($"bool allConditionsMet = false;");
			}

			int numberOfOrConditions = NumberOfOrConditions(evt);
			int orConditionIndex = 0;
			string nextLabel = GenerateEventNextLabel(evt, 0, numberOfOrConditions);
			string idName = GetEventBaseName(evt);

			List<Tuple<int, int, string>> usedSelectors = new List<Tuple<int, int, string>>(); // if a selector has already been reset during this event, don't reset it again

			foreach (var condition in evt.Conditions)
			{
				//reset any selectors used in this condition if it wasn't reset in a previous condition
				foreach (var obj in GetRelevantObjectInfos(condition).Distinct().ToList())
				{
					if (usedSelectors.Any(x => x.Item1 == obj.Item1 && x.Item2 == obj.Item2)) continue;
					usedSelectors.Add(obj);
					result.AppendLine($"{StringUtils.SanitizeObjectName(obj.Item3)}_{obj.Item1}_selector->Reset();");
				}

				var acBaseTypes = Assembly.GetExecutingAssembly().GetTypes().Where(t => t.IsSubclassOf(typeof(ConditionBase))).ToList();
				var acBaseType = acBaseTypes.FirstOrDefault(t =>
				{
					var instance = Activator.CreateInstance(t) as ConditionBase;
					return instance != null && (instance.ObjectType.Contains(condition.ObjectType) || (condition.ObjectType >= 32 && instance.ObjectType.Any(x => x >= 32))) && instance.Num == condition.Num;
				});

				if (acBaseType == null && condition.ObjectType >= 32)
				{
					acBaseType = typeof(ExtensionConditionBase);
				}

				if (acBaseType == null)
				{
					result.AppendLine($"//Condition ({condition.ObjectType}, {condition.Num}) not found");
					result.AppendLine($"goto {nextLabel};");
					continue;
				}

				Dictionary<string, object> parameters = new Dictionary<string, object>()
				{
					{ "eventIndex", j },
					{ "eventBaseName", idName }, // used for labels
					{ "frameIndex", frameIndex },
					{ "eventGroup", evt },
					{ "numOfOrs", numberOfOrConditions }
				};

				var instance = Activator.CreateInstance(acBaseType) as ConditionBase;
				//instance.IsGlobal = evt.IsGlobal;
				string ifStatement = (condition.OtherFlags & 1) == 0 ? "if (!" : "if (";
				result.AppendLine(instance?.Build(condition, ref nextLabel, ref orConditionIndex, parameters, ifStatement));
			}

			string endLabel = $"{idName}_end";
			result.AppendLine($"{idName}_actions:;");

			if (DoesEventHaveOneActionLoop(evt))
			{
				result.AppendLine($"allConditionsMet = true;");
				result.AppendLine($"if ({idName}_actions_executed_last_frame) goto {endLabel};");
				result.AppendLine($"{idName}_actions_executed_last_frame = true;");
			}

			foreach (var action in evt.Actions)
			{
				// scan actions for anything sample related, so we can preload them at runtime
				if (SoundPreloadingHelper.IsSampleAction(action) &&
					SoundPreloadingHelper.TryGetLiteralSample(action, out var sample))
				{
					RegisterLiteralSampleName(frameIndex, sample.Name);
				}

				//reset any selectors used in this action if it wasn't reset in a previous action
				foreach (var obj in GetRelevantObjectInfos(action).Distinct().ToList())
				{
					if (usedSelectors.Any(x => x.Item1 == obj.Item1 && x.Item2 == obj.Item2)) continue;
					usedSelectors.Add(obj);
					result.AppendLine($"{StringUtils.SanitizeObjectName(obj.Item3)}_{obj.Item1}_selector->Reset();");
				}

				var acBaseTypes = Assembly.GetExecutingAssembly().GetTypes().Where(t => t.IsSubclassOf(typeof(ActionBase))).ToList();
				var acBaseType = acBaseTypes.FirstOrDefault(t =>
				{
					var instance = Activator.CreateInstance(t) as ActionBase;
					return instance != null && (instance.ObjectType.Contains(action.ObjectType) || (action.ObjectType >= 32 && instance.ObjectType.Any(x => x >= 32))) && instance.Num == action.Num;
				});

				if (acBaseType == null && action.ObjectType >= 32)
				{
					acBaseType = typeof(ExtensionActionBase);
				}

				if (acBaseType == null)
				{
					result.AppendLine($"//Action ({action.ObjectType}, {action.Num}) not found");
					continue;
				}

				Dictionary<string, object> parameters = new Dictionary<string, object>()
				{
					{ "eventIndex", j },
					{ "frameIndex", frameIndex },
					{ "eventGroup", evt },
					{ "numOfOrs", numberOfOrConditions }
				};

				var instance = Activator.CreateInstance(acBaseType) as ActionBase;
				//instance.IsGlobal = evt.IsGlobal;
				result.AppendLine(instance?.Build(action, ref nextLabel, ref orConditionIndex, parameters, ""));
			}

			result.AppendLine($"{endLabel}:;");

			if (DoesEventHaveOneActionLoop(evt))
			{
				string eventBaseName = GetEventBaseName(evt);
				result.AppendLine($"if (!allConditionsMet) {eventBaseName}_actions_executed_last_frame = false;");
			}

			result.AppendLine("}");
			result.AppendLine("");
		}

		//Create any loop functions
		if (HasAnyLoopEvents(frameIndex))
		{
			result.AppendLine($"void GeneratedFrame{frameIndex}::OnLoop(const std::string& loopName)");
			result.AppendLine("{");
			for (int j = 0; j < _exporter.GameData.Frames[frameIndex].events.Items.Count; j++)
			{
				var evt = _exporter.GameData.Frames[frameIndex].events.Items[j];
				if (ShouldSkipEvent(evt)) continue;

				foreach (var condition in evt.Conditions)
				{
					if (!new LoopCondition().Equals(condition)) continue;

					string loopNameExpr = ExpressionConverter.ConvertExpression(condition.Items[0]?.Loader as ExpressionParameter);
					result.AppendLine($"\tif (LoopNameEquals(loopName, {loopNameExpr})) {GetEventName(evt)}();");
				}
			}

			result.AppendLine("}");
		}

		return result.ToString();
	}

	public static List<Tuple<int, int, string>> GetRelevantObjectInfos(EventGroup eventGroup)
	{
		List<Tuple<int, int, string>> relevantObjectInfos = [];

		foreach (var condition in eventGroup.Conditions)
		{
			relevantObjectInfos.AddRange(GetRelevantObjectInfos(condition));
		}

		foreach (var action in eventGroup.Actions)
		{
			relevantObjectInfos.AddRange(GetRelevantObjectInfos(action));
		}

		return relevantObjectInfos.Distinct().ToList();
	}

	//TODO: this is a mess
	//returns list of all OIs used in this event condition or action
	public static List<Tuple<int, int, string>> GetRelevantObjectInfos(EventBase eventBase)
	{
		List<Tuple<int, int, string>> relevantObjectInfos = [];

		//object info, object type
		List<Tuple<int, int>> objectInfos = [];

		if (eventBase.ObjectType > 0)
		{
			objectInfos.Add(new Tuple<int, int>(eventBase.ObjectInfo, eventBase.ObjectType));
		}

		foreach (var expression in eventBase.Items)
		{
			if (expression.Loader is ExpressionParameter)
			{
				foreach (var exp in (expression.Loader as ExpressionParameter).Items)
				{
					if (exp.ObjectType > 0)
					{
						objectInfos.Add(new Tuple<int, int>(exp.ObjectInfo, exp.ObjectType));
					}
				}
			}
			else if (expression.Loader is Position)
			{
				if ((expression.Loader as Position).ObjectInfoParent != ushort.MaxValue)
				{
					objectInfos.Add(new Tuple<int, int>((int)(expression.Loader as Position).ObjectInfoParent, (expression.Loader as Position).TypeParent));
				}
			}
			else if (expression.Loader is ParamObject)
			{
				objectInfos.Add(new Tuple<int, int>((expression.Loader as ParamObject).ObjectInfo, (expression.Loader as ParamObject).ObjectType));
			}
			else if (expression.Loader is Create)
			{
				objectInfos.Add(new Tuple<int, int>((expression.Loader as Create).ObjectInfo, (expression.Loader as Create).Position.TypeParent));

				if ((expression.Loader as Create).Position.ObjectInfoParent != ushort.MaxValue)
				{
					objectInfos.Add(new Tuple<int, int>((int)(expression.Loader as Create).Position.ObjectInfoParent, (expression.Loader as Create).Position.TypeParent));
				}
			}
			else if (expression.Loader is Shoot)
			{
				objectInfos.Add(new Tuple<int, int>((expression.Loader as Shoot).ObjectInfo, (expression.Loader as Shoot).ShootPos.TypeParent));

				if ((expression.Loader as Shoot).ShootPos.ObjectInfoParent != ushort.MaxValue)
				{
					objectInfos.Add(new Tuple<int, int>((int)(expression.Loader as Shoot).ShootPos.ObjectInfoParent, (expression.Loader as Shoot).ShootPos.TypeParent));
				}
			}
		}

		foreach (var objectInfo in objectInfos)
		{
			if (objectInfo.Item1 > short.MaxValue) // qualifier
			{
				string qualifierName = Utilities.GetQualifierName(objectInfo.Item1 & 0x7FFF, objectInfo.Item2);
				relevantObjectInfos.Add(new Tuple<int, int, string>(objectInfo.Item1, objectInfo.Item2, qualifierName));
			}
			else
			{
				ObjectInfo evtObj = Exporter.Instance.GameData.frameitems[objectInfo.Item1];
				string objectName = evtObj.name;

				relevantObjectInfos.Add(new Tuple<int, int, string>(objectInfo.Item1, objectInfo.Item2, objectName));
			}
		}

		return [.. relevantObjectInfos.Distinct().ToList()];
	}


	public string BuildEventIncludes(int frameIndex)
	{
		var result = new StringBuilder();

		for (int j = 0; j < _exporter.GameData.Frames[frameIndex].events.Items.Count; j++)
		{
			var evt = _exporter.GameData.Frames[frameIndex].events.Items[j];

			if (ShouldSkipEvent(evt)) continue;

			result.AppendLine($"void {GetEventName(evt)}();");
		}

		return result.ToString();
	}

	public List<string> GetAllLoopNames(int frameIndex)
	{
		List<string> loopNames = new();

		foreach (var evt in _exporter.GameData.Frames[frameIndex].events.Items)
		{
			string? loopName = DoesEventHaveLoop(evt);
			if (loopName != null)
			{
				loopNames.Add(StringUtils.SanitizeObjectName(loopName));
			}
		}

		return loopNames.Distinct().ToList();
	}

	//returns the loop name if the event has a loop condition, otherwise returns null
	string? DoesEventHaveLoop(EventGroup evtGroup)
	{
		foreach (var condition in evtGroup.Conditions)
		{
			if (new LoopCondition().Equals(condition)) return ExpressionConverter.ConvertExpression(condition.Items[0]?.Loader as ExpressionParameter).ToString() ?? "";
		}

		return null;
	}

	bool DoesEventHaveRunOnce(EventGroup evtGroup)
	{
		foreach (var condition in evtGroup.Conditions)
		{
			if (new RunOnceCondition().Equals(condition)) return true;
		}

		return false;
	}

	int NumberOfOrConditions(EventGroup evtGroup)
	{
		int count = 0;
		foreach (var condition in evtGroup.Conditions)
		{
			if (new OrLogicalCondition().Equals(condition)) count++;
		}

		return count;
	}

	public string BuildLoopIncludes(int frameIndex)
	{
		if (!HasAnyLoopEvents(frameIndex)) return "";

		return "void OnLoop(const std::string& loopName) override;\n";
	}

	private bool HasAnyLoopEvents(int frameIndex)
	{
		foreach (var evt in _exporter.GameData.Frames[frameIndex].events.Items)
		{
			if (DoesEventHaveLoop(evt) != null) return true;
		}
		return false;
	}

	public string BuildRunOnceCondition(int frameIndex)
	{
		StringBuilder result = new();

		for (int i = 0; i < _exporter.GameData.Frames[frameIndex].events.Items.Count; i++)
		{
			var evt = _exporter.GameData.Frames[frameIndex].events.Items[i];
			if (DoesEventHaveRunOnce(evt))
			{
				string idName = GetEventBaseName(evt);
				result.AppendLine($"bool {idName}_run_once = false;");
			}
		}

		return result.ToString();
	}

	public bool DoesEventHaveOneActionLoop(EventGroup evtGroup)
	{
		foreach (var condition in evtGroup.Conditions)
		{
			if (new OneActionLoopCondition().Equals(condition)) return true;
		}

		return false;
	}

	public string BuildOneActionLoop(int frameIndex)
	{
		StringBuilder result = new();

		for (int i = 0; i < _exporter.GameData.Frames[frameIndex].events.Items.Count; i++)
		{
			var evt = _exporter.GameData.Frames[frameIndex].events.Items[i];
			if (DoesEventHaveOneActionLoop(evt))
			{
				string idName = GetEventBaseName(evt);
				result.AppendLine($"bool {idName}_actions_executed_last_frame = false;");
			}
		}

		return result.ToString();
	}

	// if true, this event should be called before the normal events
	public bool IsTimerEvent(EventGroup evtGroup)
	{
		// TODO: Verify if any other conditions should be considered a timer event, I got these from 2006 documentation: https://www.clickteam.com/creation_materials/tutorials/download/Fusion_runtime.pdf
		if (new StartOfFrameCondition().Equals(evtGroup.Conditions[0])
			|| new TimerComparisonLessThanCondition().Equals(evtGroup.Conditions[0])
			|| new TimerComparisonGreaterThanCondition().Equals(evtGroup.Conditions[0])
			|| new TimerComparisonEqualToCondition().Equals(evtGroup.Conditions[0]))
		{
			return true;
		}

		return false;
	}

	public bool IsAnimationEvent(EventGroup evtGroup)
	{
		return new AnimationOverCondition().Equals(evtGroup.Conditions[0]);
	}

	public void PreProcessFrame(int frameIndex)
	{
		var frame = _exporter.GameData.Frames[frameIndex];

		// was used for global events, probably no longer needed
	}

	// utilities
	public static string GetEventBaseName(EventGroup evt)
	{
		int index = Exporter.Instance.GameData.Frames[Exporter.Instance.CurrentFrame].events.Items.IndexOf(evt);
		return $"event_{index}";
	}

	public static string GetEventName(EventGroup evt)
	{
		int index = Exporter.Instance.GameData.Frames[Exporter.Instance.CurrentFrame].events.Items.IndexOf(evt);
		return $"Event_{index}";
	}

	public static string GenerateEventNextLabel(EventGroup evt, int orConditionIndex, int totalOrs)
	{
		if (orConditionIndex < totalOrs) return $"{GetEventBaseName(evt)}_or_{orConditionIndex}";
		return $"{GetEventBaseName(evt)}_end";
	}
}
