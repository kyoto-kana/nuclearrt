using CTFAK.CCN;
using CTFAK.CCN.Chunks.Frame;
using CTFAK.Memory;
using CTFAK.MMFParser.EXE.Loaders.Events.Expressions;
using CTFAK.MMFParser.EXE.Loaders.Events.Parameters;
using CTFAK.Utils;
using System.Text;
using System.Text.Json;

public class SelectObjectExporter : ExtensionExporter
{
	// Original extension made by Dynasoft, source published by LB--
	public override string ObjectIdentifier => "SOSD";
	public override string ExtensionName => "SelectObject";
	public override string CppClassName => "SelectObjectExtension";

	const int CND_SelectFirst = 1; //IMPL
	const int CND_SelectHighestAltV = 2; //IMPL
	const int CND_SelectLowestAltV = 3; //IMPL
	const int CND_SelectLast = 4; //IMPL
	const int CND_SelectAll = 5; //IMPL
	const int CND_SelectFrontmost = 6; //IMPL
	const int CND_SelectRearmost = 7; //IMPL
	const int CND_DiscardFirst = 8; //IMPL
	const int CND_DiscardLast = 9; //IMPL
	const int CND_CompareSelected = 10; //IMPL

    // Apparently, these just never got implemented in the original extension, so these have no exports
	const int CND_SelectRandom = 11;
	const int CND_DiscardRandom = 12;
	const int CND_SelectTiedHighestAltV = 13;
	const int CND_SelectTiedLowestAltV = 14;
    
	const int CND_SelectIndex = 15; //IMPL
    
	const int EXP_GetSelectedByName = 0;
	const int EXP_GetSelectedByFixed = 1;
	const int EXP_GetNameByFixed = 2;

	public override string ExportExtension(byte[] extensionData)
	{
		// No props
		return string.Empty;
	}

	public override string ExportCondition(EventBase eventBase, int conditionNum, ref string nextLabel, ref int orIndex, Dictionary<string, object>? parameters = null, string ifStatement = "if (", bool isGlobal = false)
	{
		StringBuilder result = new();
        var objectSelector = GetSelector(((ParamObject)eventBase.Items[0].Loader).ObjectInfo, ((ParamObject)eventBase.Items[0].Loader).ObjectType);

		switch (conditionNum)
		{
            case CND_CompareSelected:
				result.AppendLine($"if (CValue((int){objectSelector}.Count()) {ExpressionConverter.GetOppositeComparison(((ExpressionParameter)eventBase.Items[1].Loader).Comparsion)} {EvaluateExpression(eventBase, 1)}) goto {nextLabel};");
				break;
            case CND_SelectFrontmost:
                result.AppendLine("{");
                result.AppendLine("    int highest = 0;");
                result.AppendLine($"    ObjectInstance* foundHighest = nullptr;");
                result.AppendLine($"    for (ObjectIterator it({objectSelector}); !it.end(); ++it) {{");
                result.AppendLine("        ObjectInstance* obj = *it;");
                result.AppendLine("        if (obj->Type < 2) continue;");
                result.AppendLine("        auto& layerList = Layers[obj->Layer].instances;");
                result.AppendLine("        auto posIt = std::find(layerList.begin(), layerList.end(), obj);");
                result.AppendLine("        if (posIt == layerList.end()) continue;");
                result.AppendLine("        int z = (static_cast<int>(obj->Layer) << 16) | static_cast<int>(std::distance(layerList.begin(), posIt));");
                result.AppendLine("        if (foundHighest == nullptr || z > highest) {");
                result.AppendLine("            highest = z;");
                result.AppendLine("            foundHighest = obj;");
                result.AppendLine("        }");
                result.AppendLine("    }");
                result.AppendLine($"    if (foundHighest != nullptr) {{");
                result.AppendLine($"        {objectSelector}.SelectOnly(foundHighest);");
                result.AppendLine($"    }} else {{");
                result.AppendLine($"        {objectSelector}.DeselectAll();");
                result.AppendLine($"        goto {nextLabel};");
                result.AppendLine($"    }}");
                result.AppendLine("}");
                break;
            case CND_SelectRearmost:
                result.AppendLine("{");
                result.AppendLine("    int lowest = 0;");
                result.AppendLine($"    ObjectInstance* foundLowest = nullptr;");
                result.AppendLine($"    for (ObjectIterator it({objectSelector}); !it.end(); ++it) {{");
                result.AppendLine("        ObjectInstance* obj = *it;");
                result.AppendLine("        if (obj->Type < 2) continue;");
                result.AppendLine("        auto& layerList = Layers[obj->Layer].instances;");
                result.AppendLine("        auto posIt = std::find(layerList.begin(), layerList.end(), obj);");
                result.AppendLine("        if (posIt == layerList.end()) continue;");
                result.AppendLine("        int z = (static_cast<int>(obj->Layer) << 16) | static_cast<int>(std::distance(layerList.begin(), posIt));");
                result.AppendLine("        if (foundLowest == nullptr || z < lowest) {");
                result.AppendLine("            lowest = z;");
                result.AppendLine("            foundLowest = obj;");
                result.AppendLine("        }");
                result.AppendLine("    }");
                result.AppendLine($"    if (foundLowest != nullptr) {{");
                result.AppendLine($"        {objectSelector}.SelectOnly(foundLowest);");
                result.AppendLine($"    }} else {{");
                result.AppendLine($"        {objectSelector}.DeselectAll();");
                result.AppendLine($"        goto {nextLabel};");
                result.AppendLine($"    }}");
                result.AppendLine("}");
                break;
            case CND_SelectHighestAltV:
                result.AppendLine("{");
                result.AppendLine($"    CValue highest;");
                result.AppendLine($"    ObjectInstance* foundHighest = nullptr;");
                result.AppendLine($"    int alterableIndex = {EvaluateExpression(eventBase, 1)}.GetIntValue();");
                result.AppendLine($"    for (ObjectIterator it({objectSelector}); !it.end(); ++it) {{");
                result.AppendLine($"        if (!HasAlterableValues(*it)) continue;");
                result.AppendLine($"        CValue value = GetAlterableValueByIndex(*it, alterableIndex);");
                result.AppendLine($"        if (foundHighest == nullptr || value > highest) {{");
                result.AppendLine($"            highest = value;");
                result.AppendLine($"            foundHighest = *it;");
                result.AppendLine($"        }}");
                result.AppendLine("    }");
                result.AppendLine($"    if (foundHighest != nullptr) {{");
                result.AppendLine($"        {objectSelector}.SelectOnly(foundHighest);");
                result.AppendLine($"    }} else {{");
                result.AppendLine($"        {objectSelector}.DeselectAll();");
                result.AppendLine($"        goto {nextLabel};");
                result.AppendLine($"    }}");
                result.AppendLine("}");
                break;
            case CND_SelectLowestAltV:
                result.AppendLine("{");
                result.AppendLine($"    CValue lowest;");
                result.AppendLine($"    ObjectInstance* foundLowest = nullptr;");
                result.AppendLine($"    int alterableIndex = {EvaluateExpression(eventBase, 1)}.GetIntValue();");
                result.AppendLine($"    for (ObjectIterator it({objectSelector}); !it.end(); ++it) {{");
                result.AppendLine($"        if (!HasAlterableValues(*it)) continue;");
                result.AppendLine($"        CValue value = GetAlterableValueByIndex(*it, alterableIndex);");
                result.AppendLine($"        if (foundLowest == nullptr || value < lowest) {{");
                result.AppendLine($"            lowest = value;");
                result.AppendLine($"            foundLowest = *it;");
                result.AppendLine($"        }}");
                result.AppendLine("    }");
                result.AppendLine($"    if (foundLowest != nullptr) {{");
                result.AppendLine($"        {objectSelector}.SelectOnly(foundLowest);");
                result.AppendLine($"    }} else {{");
                result.AppendLine($"        {objectSelector}.DeselectAll();");
                result.AppendLine($"        goto {nextLabel};");
                result.AppendLine($"    }}");
                result.AppendLine("}");
                break;
            case CND_SelectIndex:
                result.AppendLine("{");
                result.AppendLine($"    int keep = {objectSelector}.Count() - 1 - {EvaluateExpression(eventBase, 1)}.GetIntValue();");
                result.AppendLine($"    ObjectInstance* target = {objectSelector}.At(keep);");
                result.AppendLine($"    if (target != nullptr) {{");
                result.AppendLine($"        {objectSelector}.SelectOnly(target);");
                result.AppendLine($"    }} else {{");
                result.AppendLine($"        {objectSelector}.DeselectAll();");
                result.AppendLine($"        goto {nextLabel};");
                result.AppendLine($"    }}");
                result.AppendLine("}");
                break;
            case CND_SelectLast:
                result.AppendLine("{");
                result.AppendLine($"    int keep = {EvaluateExpression(eventBase, 1)}.GetIntValue();");
                result.AppendLine($"    int index = 0;");
                result.AppendLine($"    for (ObjectIterator it({objectSelector}); !it.end(); ++it) {{");
                result.AppendLine($"        if (index++ >= keep) it.deselect();");
                result.AppendLine("    }");
                result.AppendLine($"    if ({objectSelector}.Count() == 0) goto {nextLabel};");
                result.AppendLine("}");
                break;
            case CND_SelectFirst:
                result.AppendLine("{");
                result.AppendLine($"    int keep = {EvaluateExpression(eventBase, 1)}.GetIntValue();");
                result.AppendLine($"    if (keep <= 0) goto {nextLabel};");
                result.AppendLine($"    auto snapshot = {objectSelector}.SaveSelection();");
                result.AppendLine("    if (keep < (int)snapshot.size()) snapshot.erase(snapshot.begin(), snapshot.end() - keep);");
                result.AppendLine($"    {objectSelector}.RestoreSelection(snapshot);");
                result.AppendLine($"    if ({objectSelector}.Count() == 0) goto {nextLabel};");
                result.AppendLine("}");
                break;
            case CND_SelectAll:
                result.AppendLine("{");
                result.AppendLine($"    {objectSelector}.Reset();");
                result.AppendLine($"    if ({objectSelector}.Count() == 0) goto {nextLabel};");
                result.AppendLine("}");
                break;
            case CND_DiscardLast:
                result.AppendLine("{");
                result.AppendLine($"    int skip = {EvaluateExpression(eventBase, 1)}.GetIntValue();");
                result.AppendLine($"    if (skip <= 0) goto {nextLabel};");
                result.AppendLine($"    auto snapshot = {objectSelector}.SaveSelection();");
                result.AppendLine("    if (skip >= (int)snapshot.size()) snapshot.clear();");
                result.AppendLine("    else snapshot.erase(snapshot.begin(), snapshot.begin() + skip);");
                result.AppendLine($"    {objectSelector}.RestoreSelection(snapshot);");
                result.AppendLine($"    if ({objectSelector}.Count() == 0) goto {nextLabel};");
                result.AppendLine("}");
                break;
            case CND_DiscardFirst:
                result.AppendLine("{");
                result.AppendLine($"    int skip = {EvaluateExpression(eventBase, 1)}.GetIntValue();");
                result.AppendLine($"    if (skip <= 0) goto {nextLabel};");
                result.AppendLine($"    auto snapshot = {objectSelector}.SaveSelection();");
                result.AppendLine("    if (skip >= (int)snapshot.size()) snapshot.clear();");
                result.AppendLine("    else snapshot.erase(snapshot.end() - skip, snapshot.end());");
                result.AppendLine($"    {objectSelector}.RestoreSelection(snapshot);");
                result.AppendLine($"    if ({objectSelector}.Count() == 0) goto {nextLabel};");
                result.AppendLine("}");
                break;
			default:
				result.AppendLine($"// SelectObject condition {conditionNum} not implemented");
				result.AppendLine($"goto {nextLabel};");
				break;
		}

		return result.ToString();
	}

	public override string ExportAction(EventBase eventBase, int actionNum, ref string nextLabel, ref int orIndex, Dictionary<string, object>? parameters = null, bool isGlobal = false)
	{
		// No actions
		return string.Empty;
	}

	public override string ExportExpression(Expression expression, EventBase eventBase = null)
	{
		string result;

		switch (expression.Num)
		{
			case EXP_GetSelectedByName:
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->GetSelectedByName(this, ";
				break;
			case EXP_GetSelectedByFixed:
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->GetSelectedByFixed(this, ";
				break;
			case EXP_GetNameByFixed:
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->GetNameByFixed(this, ";
				break;
			default:
				result = $"0 /* SelectObject expression {expression.Num} not implemented */";
				break;
		}

		return result;
	}
}
