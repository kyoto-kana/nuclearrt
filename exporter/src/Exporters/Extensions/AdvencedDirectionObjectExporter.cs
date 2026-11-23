using CTFAK.Memory;
using CTFAK.CCN.Chunks.Frame;
using System.Text;
using CTFAK.MMFParser.EXE.Loaders.Events.Expressions;
using CTFAK.MMFParser.EXE.Loaders.Events.Parameters;

public class AdvencedDirectionObjectExporter : ExtensionExporter
{
	public override string ObjectIdentifier => "CgnA";
	public override string ExtensionName => "AdvDir";
	public override string CppClassName => "AdvancedDirectionObjectExtension";

	const int CND_COMPDIST = 0;
	const int CND_COMPDIR = 1;
	const int ACT_SETNUMDIR = 0;
	const int ACT_GETOBJECTS = 1;
	const int ACT_ADDOBJECTS = 2;
	const int ACT_RESET = 3;
	const int EXP_GETNUMDIR = 0;
	const int EXP_DIRECTION = 1;
	const int EXP_DISTANCE = 2;
	const int EXP_DIRECTIONLONG = 3;
	const int EXP_DISTANCELONG = 4;
	const int EXP_ROTATE = 5;
	const int EXP_DIRDIFFABS = 6;
	const int EXP_DIRDIFF = 7;
	const int EXP_GETFIXEDOBJ = 8;
	const int EXP_GETDISTOBJ = 9;
	const int EXP_XMOV = 10;
	const int EXP_YMOV = 11;
	const int EXP_DIRBASE = 12;

	public override string ExportExtension(byte[] extensionData)
	{
		ByteReader reader = new ByteReader(extensionData);

        int NumDir = 0;
		file.Skip(8);
		
		string rawNumDir = file.ReadAscii(32).Split('\0')[0];
		if (int.TryParse(rawNumDir, out int parsedDir))
		{
			NumDir = parsedDir;
		}

		return CreateExtension($"{NumDir}");
	}

	public override string ExportCondition(EventBase eventBase, int conditionNum, ref string nextLabel, ref int orIndex, Dictionary<string, object>? parameters = null, string ifStatement = "if (", bool isGlobal = false)
	{
		StringBuilder result = new();

		switch (conditionNum)
		{
			case CND_COMPDIST:
				result.AppendLine($"{ifStatement} {GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->CompDist({((Position)eventBase.Items[0].Loader).X}, {((Position)eventBase.Items[0].Loader).Y}, {((Position)eventBase.Items[1].Loader).X}, {((Position)eventBase.Items[1].Loader).Y}, {EvaluateExpression(eventBase, 2)})) goto {nextLabel};");
				break;
			case CND_COMPDIR:
				result.AppendLine($"{ifStatement} {GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->CompDir({((Position)eventBase.Items[0].Loader).X}, {((Position)eventBase.Items[0].Loader).Y}, {((Position)eventBase.Items[1].Loader).X}, {((Position)eventBase.Items[1].Loader).Y}, {EvaluateExpression(eventBase, 2)}, {EvaluateExpression(eventBase, 3)})) goto {nextLabel};");
				break;
			default:
				result.AppendLine($"// AdvDir condition {conditionNum} not implemented");
				result.AppendLine($"goto {nextLabel};");
				break;
		}

		return result.ToString();
	}

	public override string ExportAction(EventBase eventBase, int actionNum, ref string nextLabel, ref int orIndex, Dictionary<string, object>? parameters = null, bool isGlobal = false)
	{
		StringBuilder result = new();

		switch (actionNum)
		{
			case ACT_SETNUMDIR:
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->SetNumDir({EvaluateExpression(eventBase, 0)});");
				break;
			case ACT_GETOBJECTS:
				result.AppendLine($"for (ObjectIterator it({GetSelector(((ParamObject)eventBase.Items[0].Loader).ObjectInfo, ((ParamObject)eventBase.Items[0].Loader).ObjectType)}); !it.end(); ++it) {{");
				result.AppendLine($"    auto instance = *it;");
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->GetObjects(instance, {EvaluateExpression(eventBase, 0)}, {((Position)eventBase.Items[1].Loader).X}, {((Position)eventBase.Items[1].Loader).Y});");
				result.AppendLine("}");
				break;
			case ACT_ADDOBJECTS:
				result.AppendLine($"for (ObjectIterator it({GetSelector(((ParamObject)eventBase.Items[0].Loader).ObjectInfo, ((ParamObject)eventBase.Items[0].Loader).ObjectType)}); !it.end(); ++it) {{");
				result.AppendLine($"    auto instance = *it;");
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->AddObjects(instance, {EvaluateExpression(eventBase, 0)});");
				result.AppendLine("}");
				break;
			case ACT_RESET:
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->CurrentObject = 0;");
				break;
			default:
				result.AppendLine($"// AdvDir action {actionNum} not implemented");
				break;
		}

		return result.ToString();
	}

	public override string ExportExpression(Expression expression, EventBase eventBase = null)
	{
		string result;

		switch (expression.Num)
		{
			case EXP_GETNUMDIR:
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->GetNumDir()"
				break;
			case EXP_DIRECTION:
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->Direction("
				break;
			case EXP_DISTANCE:
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->getDistance("
				break;
			case EXP_DIRECTIONLONG:
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->LongDir("
				break;
			case EXP_DISTANCELONG:
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->LongDist("
				break;
			case EXP_ROTATE:
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->Rotate("
				break;
			case EXP_DIRDIFFABS:
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->DirDiffAbs("
				break;
			case EXP_DIRDIFF:
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->DirDiff("
				break;
			case EXP_GETFIXEDOBJ:
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->GetFixedObj("
				break;
			case EXP_GETDISTOBJ:
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->GetDistObj("
				break;
			case EXP_XMOV:
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->XMov("
				break;
			case EXP_YMOV:
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->YMov("
				break;
			case EXP_DIRBASE:
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->DirBase("
				break;
			default:
				result = $"0 /* AdvDir expression {expression.Num} not implemented */";
				break;
		}

		return result;
	}
}
