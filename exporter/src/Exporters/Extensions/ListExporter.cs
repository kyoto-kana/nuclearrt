using CTFAK.Memory;
using CTFAK.CCN.Chunks.Frame;
using System.Text;
using CTFAK.MMFParser.EXE.Loaders.Events.Expressions;
using CTFAK.MMFParser.EXE.Loaders.Events.Parameters;

public class ListExporter : ExtensionExporter
{
	public override string ObjectIdentifier => "0TSL";
	public override string ExtensionName => "List";
	public override string CppClassName => "ListExtension";

	// Flags
	const int LIST_FREEFLAG = 0x0001;
	const int LIST_VSCROLLBAR = 0x0002;
	const int LIST_SORT = 0x0004;
	const int LIST_BORDER = 0x0008;
	const int LIST_HIDEONSTART = 0x0010;
	const int LIST_SYSCOLOR = 0x0020;
	const int LIST_3DLOOK = 0x0040;
	const int LIST_SCROLLTONEWLINE = 0x0080;
	const int LIST_JUSTCREATED = 0x8000;
    // Condition identifiers
    const int CND_VISIBLE = 0;
    const int CND_ENABLE = 1;
    const int CND_DOUBLECLICKED = 2;
    const int CND_SELECTIONCHANGED = 3;
    const int CND_HAVEFOCUS = 4;
    const int CND_LAST = 5;
    // Action identifiers
    const int ACT_LOADLIST = 0;
    const int ACT_LOADDRIVESLIST = 1;
    const int ACT_LOADDIRECTORYLIST = 2;
    const int ACT_LOADFILESLIST = 3;
    const int ACT_SAVELIST = 4;
    const int ACT_RESET = 5;
    const int ACT_ADDLINE = 6;
    const int ACT_INSERTLINE = 7;
    const int ACT_DELLINE = 8;
    const int ACT_SETCURRENTLINE = 9;
    const int ACT_SHOW = 10;
    const int ACT_HIDE = 11;
    const int ACT_ACTIVATE = 12;
    const int ACT_ENABLE = 13;
    const int ACT_DISABLE = 14;
    const int ACT_SETPOSITION = 15;
    const int ACT_SETXPOSITION = 16;
    const int ACT_SETYPOSITION = 17;
    const int ACT_SETSIZE = 18;
    const int ACT_SETXSIZE = 19;
    const int ACT_SETYSIZE = 20;
    const int ACT_DESACTIVATE = 21;
    const int ACT_SCROLLTOTOP = 22;
    const int ACT_SCROLLTOLINE = 23;
    const int ACT_SCROLLTOEND = 24;
    const int ACT_SETCOLOR = 25;
    const int ACT_SETBKDCOLOR = 26;
    const int ACT_LOADFONTSLIST = 27;
    const int ACT_LOADFONTSIZESLIST = 28;
    const int ACT_SETLINEDATA = 29;
    const int ACT_CHANGELINE = 30;
    const int ACT_LAST = 31;
    // Expression identifiers
    const int EXP_GETSELECTINDEX = 0;
    const int EXP_GETSELECTTEXT = 1;
    const int EXP_GETSELECTDIRECTORY = 2;
    const int EXP_GETSELECTDRIVE = 3;
    const int EXP_GETLINETEXT = 4;
    const int EXP_GETLINEDIRECTORY = 5;
    const int EXP_GETLINEDRIVE = 6;
    const int EXP_GETNBLINE = 7;
    const int EXP_GETXPOSITION = 8;
    const int EXP_GETYPOSITION = 9;
    const int EXP_GETXSIZE = 10;
    const int EXP_GETYSIZE = 11;
    const int EXP_GETCOLOR = 12;
    const int EXP_GETBKDCOLOR = 13;
    const int EXP_FINDSTRING = 14;
    const int EXP_FINDSTRINGEXACT = 15;
    const int EXP_GETLASTINDEX = 16;
    const int EXP_GETLINEDATA = 17;
    const int EXP_LAST = 18;

	public override string ExportExtension(byte[] extensionData)
	{
		ByteReader reader = new ByteReader(extensionData);
		// Only porting the internals for now. so skip all visual stuff
    	reader.Skip(248);

		int flags = reader.ReadInt32();
		short lineNumbers = reader.ReadInt16();
		int indexOffset = reader.ReadInt32() == 1 ? -1 : 0;

    	reader.Skip(12);

		List<string>  items = new List<string>(lineNumbers);
		bool isSorted = (flags & LIST_SORT) != 0;

		while (lineNumbers > 0)
		{
        	items.Add(reader.ReadYuniversal());
			lineNumbers--;
		}

		if (isSorted)
		{
			items.Sort(StringComparer.OrdinalIgnoreCase);
		}

		var sortedItems = new List<string>(items.Count);
		foreach (string item in items)
		{
			sortedItems.Add($"\"{StringUtils.SanitizeString(item)}\"");
		}

    	string cppList = $"{{ {string.Join(", ", sortedItems)} }}";
		return CreateExtension($"{flags}, {indexOffset}, {cppList}");
	}

	public override string ExportCondition(EventBase eventBase, int conditionNum, ref string nextLabel, ref int orIndex, Dictionary<string, object>? parameters = null, string ifStatement = "if (", bool isGlobal = false)
	{
		StringBuilder result = new();

		switch (conditionNum)
		{
			default:
				result.AppendLine($"// List condition {conditionNum} not implemented");
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
			case ACT_LOADLIST:
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->Load({EvaluateExpression(eventBase, 0)});");
				break;
			case ACT_SAVELIST:
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->Save({EvaluateExpression(eventBase, 0)});");
				break;
			case ACT_RESET:
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->Reset();");
				break;
			case ACT_SETLINEDATA:
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->SetLineData({EvaluateExpression(eventBase, 0)}, {EvaluateExpression(eventBase, 1)});");
				break;
			case ACT_CHANGELINE:
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->ChangeLine({EvaluateExpression(eventBase, 0)}, {EvaluateExpression(eventBase, 1)});");
				break;
			case ACT_ADDLINE:
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->AddLine({EvaluateExpression(eventBase, 0)});");
				break;
			case ACT_INSERTLINE:
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->InsertLine({EvaluateExpression(eventBase, 0)}, {EvaluateExpression(eventBase, 1)});");
				break;
			case ACT_DELLINE:
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->DeleteLine({EvaluateExpression(eventBase, 0)});");
				break;
			default:
				result.AppendLine($"// List action {actionNum} not implemented");
				break;
		}

		return result.ToString();
	}

	public override string ExportExpression(Expression expression, EventBase eventBase = null)
	{
		string result;

		switch (expression.Num)
		{
			case EXP_GETSELECTDIRECTORY:
				result = "CValue(\"\")";
				break;
			case EXP_GETSELECTDRIVE:
				result = "CValue(\"\")";
				break;
			case EXP_GETLINETEXT:
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->GetLineText(";
				break;
			case EXP_GETLINEDIRECTORY:
				result = "CValue(\"\")";
				break;
			case EXP_GETLINEDRIVE:
				result = "CValue(\"\")";
				break;
			case EXP_GETNBLINE:
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->GetNbLine()";
				break;
			case EXP_FINDSTRING:
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->FindString(";
				break;
			case EXP_FINDSTRINGEXACT:
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->FindStringExact(";
				break;
			case EXP_GETLASTINDEX:
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->GetLastIndex()";
				break;
			case EXP_GETLINEDATA:
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->GetLineData(";
				break;
			default:
				result = $"0 /* List expression {expression.Num} not implemented */";
				break;
		}

		return result;
	}
}
