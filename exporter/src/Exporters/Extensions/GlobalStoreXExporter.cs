using System.Diagnostics.Contracts;
using System.Text;
using CTFAK.CCN.Chunks.Frame;
using CTFAK.Memory;
using CTFAK.MMFParser.EXE.Loaders.Events.Expressions;
using CTFAK.MMFParser.EXE.Loaders.Events.Parameters;
using CTFAK.Utils;

public class GlobalStoreXExporter : ExtensionExporter
{
	public override string ObjectIdentifier => "RTSD";
	public override string ExtensionName => "gstorex";
	public override string CppClassName => "GlobalStoreXExtension";

	public override string ExportExtension(byte[] extensionData)
	{
		ByteReader reader = new ByteReader(extensionData);
		reader.Skip(8); // 8 bytes unknown

		uint IntegerCount = reader.ReadUInt32();
		uint StringCount = reader.ReadUInt32();
		uint BoolCount = reader.ReadUInt32();
		uint ShortCount = reader.ReadUInt32();

		int flags = 0;
		if (!reader.ReadBoolean()) flags |= 0x01; // integer
		if (!reader.ReadBoolean()) flags |= 0x02; // string
		if (!reader.ReadBoolean()) flags |= 0x04; // bool
		if (!reader.ReadBoolean()) flags |= 0x08; // short

		return CreateExtension($"{IntegerCount}, {StringCount}, {BoolCount}, {ShortCount}, {flags}");
	}

	public override string ExportCondition(EventBase eventBase, int conditionNum, ref string nextLabel, ref int orIndex, Dictionary<string, object>? parameters = null, string ifStatement = "if (", bool isGlobal = false)
	{
		StringBuilder result = new();

		switch (conditionNum)
		{
			case 1: // Compare Integer
				result.AppendLine($"{ifStatement} ({GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->GetInteger({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)}) {ExpressionConverter.GetComparisonSymbol(((ExpressionParameter)eventBase.Items[1].Loader).Comparsion)} {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[1].Loader, eventBase)})) goto {nextLabel};");
				break;
			case 2: // Compare Bool
				result.AppendLine($"{ifStatement} ({GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->GetBool({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)}) {ExpressionConverter.GetComparisonSymbol(((ExpressionParameter)eventBase.Items[1].Loader).Comparsion)} {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[1].Loader, eventBase)})) goto {nextLabel};");
				break;
			case 3: // Compare Short
				result.AppendLine($"{ifStatement} ({GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->GetShort({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)}) {ExpressionConverter.GetComparisonSymbol(((ExpressionParameter)eventBase.Items[1].Loader).Comparsion)} {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[1].Loader, eventBase)})) goto {nextLabel};");
				break;
			default:
				result.AppendLine($"// Global Store X condition {conditionNum} not implemented");
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
			case 0: // Set Integer
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->SetInteger({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)}, {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[1].Loader, eventBase)});");
				break;
			case 1: // Set Bool
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->SetBool({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)}, {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[1].Loader, eventBase)});");
				break;
			case 2: // Set String
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->SetString({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)}, {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[1].Loader, eventBase)});");
				break;
			case 3: // Add Integer
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->AddInteger({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)}, {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[1].Loader, eventBase)});");
				break;
			case 4: // Set Bool Range
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->SetRangeBool({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)}, {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[1].Loader, eventBase)}, {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[2].Loader, eventBase)});");
				break;
			case 5: // Add String
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->AddString({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)}, {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[1].Loader, eventBase)});");
				break;
			case 6: // Subtract Integer
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->SubtractInteger({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)}, {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[1].Loader, eventBase)});");
				break;
			case 7: // Clear Bools
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->ClearBools();");
				break;
			case 8: // Set Integer Range
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->SetRangeInteger({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)}, {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[1].Loader, eventBase)}, {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[2].Loader, eventBase)});");
				break;
			case 9: // Set String Range
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->SetRangeString({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)}, {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[1].Loader, eventBase)}, {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[2].Loader, eventBase)});");
				break;
			case 10: // Clear Integers
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->ClearIntegers();");
				break;
			case 11: // Resize Integers
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->ResizeIntegers({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)});");
				break;
			case 12: // Expand Integers
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->ExpandIntegers({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)});");
				break;
			case 13: // Clear Strings
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->ClearStrings();");
				break;
			case 14: // Resize Strings
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->ResizeStrings({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)});");
				break;
			case 15: // Expand Strings
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->ExpandStrings({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)});");
				break;
			case 16: // Resize Bools
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->ResizeBools({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)});");
				break;
			case 17: // Expand Bools
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->ExpandBools({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)});");
				break;
			case 18: // Save Integer INI
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->SaveIntegerINI({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)}, {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[1].Loader, eventBase)});");
				break;
			case 19: // Save String INI
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->SaveStringINI({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)}, {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[1].Loader, eventBase)});");
				break;
			case 20: // Save Bool INI
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->SaveBoolINI({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)}, {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[1].Loader, eventBase)});");
				break;
			case 21: // Save All INI
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->SaveAllINI({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)}, {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[1].Loader, eventBase)});");
				break;
			case 22: // Load Integer INI
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->LoadIntegerINI({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)}, {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[1].Loader, eventBase)});");
				break;
			case 23: // Load String INI
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->LoadStringINI({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)}, {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[1].Loader, eventBase)});");
				break;
			case 24: // Load Bool INI
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->LoadBoolINI({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)}, {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[1].Loader, eventBase)});");
				break;
			case 25: // Load All INI
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->LoadAllINI({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)}, {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[1].Loader, eventBase)});");
				break;
			case 26: // Save Integer Binary
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->SaveIntegerBinary({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)});");
				break;
			case 27: // Save String Binary
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->SaveStringBinary({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)});");
				break;
			case 28: // Save Bool Binary
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->SaveBoolBinary({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)});");
				break;
			case 29: // Save All Binary
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->SaveAllBinary({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)});");
				break;
			case 30: // Load Integer Binary
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->LoadIntegerBinary({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)});");
				break;
			case 31: // Load String Binary
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->LoadStringBinary({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)});");
				break;
			case 32: // Load Bool Binary
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->LoadBoolBinary({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)});");
				break;
			case 33: // Load All Binary
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->LoadAllBinary({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)});");
				break;
			case 34: // Set Short
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->SetShort({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)}, {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[1].Loader, eventBase)});");
				break;
			case 35: // Add Short
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->AddShort({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)}, {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[1].Loader, eventBase)});");
				break;
			case 36: // Subtract Short
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->SubtractShort({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)}, {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[1].Loader, eventBase)});");
				break;
			case 37: // Set Short Range
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->SetRangeShort({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)}, {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[1].Loader, eventBase)}, {ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[2].Loader, eventBase)});");
				break;
			case 38: // Clear Shorts
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->ClearShorts();");
				break;
			case 39: // Resize Shorts
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->ResizeShorts({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)});");
				break;
			case 40: // Expand Shorts
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->ExpandShorts({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)});");
				break;
			case 41: // Save Short INI
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->SaveShortINI({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)});");
				break;
			case 42: // Load Short INI
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->LoadShortINI({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)});");
				break;
			case 43: // Save Short Binary
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->SaveShortBinary({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)});");
				break;
			case 44: // Load Short Binary
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->LoadShortBinary({ExpressionConverter.ConvertExpression((ExpressionParameter)eventBase.Items[0].Loader, eventBase)});");
				break;
			default:
				result.AppendLine($"// Global Store X action {actionNum} not implemented");
				break;
		}

		return result.ToString();
	}

	public override string ExportExpression(Expression expression, EventBase eventBase = null)
	{
		string result;

		switch (expression.Num)
		{
			case 0: // Get Integer
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->GetInteger(";
				break;
			case 1: // Get Bool
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->GetBool(";
				break;
			case 2: // Get String
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->GetString(";
				break;
			case 3: // Get Integer Array Size
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->GetIntegerArraySize()";
				break;
			case 4: // Get String Array Size
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->GetStringArraySize()";
				break;
			case 5: // Get Integer Base
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->GetIntegerBase()";
				break;
			case 6: // Get String Base
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->GetStringBase()";
				break;
			case 7: // Get Bool Size
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->GetBoolArraySize()";
				break;
			case 8: // Get Bool Base
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->GetBoolBase()";
				break;
			case 9: // Get Short
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->GetShort(";
				break;
			case 10: // Get Short Size
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->GetShortArraySize()";
				break;
			case 11: // Get Short Base
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->GetShortBase()";
				break;
			default:
				result = $"0 /* Global Store X expression {expression.Num} not implemented */";
				break;
		}

		return result;
	}
}
