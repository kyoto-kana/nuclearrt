using CTFAK.Memory;
using CTFAK.CCN.Chunks.Frame;
using System.Text;
using CTFAK.MMFParser.EXE.Loaders.Events.Expressions;
using CTFAK.MMFParser.EXE.Loaders.Events.Parameters;

public class StringParserExporter : ExtensionExporter
{
	public override string ObjectIdentifier => "PSE3";
	public override string ExtensionName => "StringParser";
	public override string CppClassName => "StringParserExtension";

    public const int CASE_INSENSITIVE = 0;
    public const int SEARCH_LITERAL = 0;
    const int CND_ISURLSAFE = 0;
    const int ACT_SETSTRING = 0;
    const int ACT_SAVETOFILE = 1;
    const int ACT_LOADFROMFILE = 2;
    const int ACT_APPENDTOFILE = 3;
    const int ACT_APPENDFROMFILE = 4;
    const int ACT_RESETDELIMS = 5;
    const int ACT_ADDDELIM = 6;
    const int ACT_SETDELIM = 7;
    const int ACT_DELETEDELIMINDEX = 8;
    const int ACT_DELETEDELIM = 9;
    const int ACT_SETDEFDELIMINDEX = 10;
    const int ACT_SETDEFDELIM = 11;
    const int ACT_SAVEASCSV = 12;
    const int ACT_LOADFROMCSV = 13;
    const int ACT_SAVEASMMFARRAY = 14;
    const int ACT_LOADFROMMMFARRAY = 15;
    const int ACT_SAVEASDYNAMICARRAY = 16;
    const int ACT_LOADFROMDYNAMICARRAY = 17;
    const int ACT_CASEINSENSITIVE = 18;
    const int ACT_CASESENSITIVE = 19;
    const int ACT_SEARCHLITERAL = 20;
    const int ACT_SEARCHWILDCARDS = 21;
    const int ACT_SAVEASINI = 22;
    const int ACT_LOADFROMINI = 23;
    const int EXP_GETSTRING = 0;
    const int EXP_GETLENGTH = 1;
    const int EXP_LEFT = 2;
    const int EXP_RIGHT = 3;
    const int EXP_MIDDLE = 4;
    const int EXP_NUMBEROFSUBS = 5;
    const int EXP_INDEXOFSUB = 6;
    const int EXP_INDEXOFFIRSTSUB = 7;
    const int EXP_INDEXOFLASTSUB = 8;
    const int EXP_REMOVE = 9;
    const int EXP_REPLACE = 10;
    const int EXP_INSERT = 11;
    const int EXP_REVERSE = 12;
    const int EXP_UPPERCASE = 13;
    const int EXP_LOWERCASE = 14;
    const int EXP_URLENCODE = 15;
    const int EXP_CHR = 16;
    const int EXP_ASC = 17;
    const int EXP_ASCLIST = 18;
    const int EXP_NUMBEROFDELIMS = 19;
    const int EXP_GETDELIM = 20;
    const int EXP_GETDELIMINDEX = 21;
    const int EXP_GETDEFDELIM = 22;
    const int EXP_GETDEFDELIMINDEX = 23;
    const int EXP_LISTCOUNT = 24;
    const int EXP_LISTSETAT = 25;
    const int EXP_LISTINSERTAT = 26;
    const int EXP_LISTAPPEND = 27;
    const int EXP_LISTPREPEND = 28;
    const int EXP_LISTGETAT = 29;
    const int EXP_LISTFIRST = 30;
    const int EXP_LISTLAST = 31;
    const int EXP_LISTFIND = 32;
    const int EXP_LISTCONTAINS = 33;
    const int EXP_LISTDELETEAT = 34;
    const int EXP_LISTSWAP = 35;
    const int EXP_LISTSORTASC = 36;
    const int EXP_LISTSORTDESC = 37;
    const int EXP_LISTCHANGEDELIMS = 38;
    const int EXP_SETSTRING = 39;
    const int EXP_SETVALUE = 40;
    const int EXP_GETMD5 = 41;

    private String fixString(String input)
    {
        for (int i = 0; i < input.Length; i++)
        {
            if (input[i] < 10)
            {
                return input.Substring(0, i);
            }
        }
        return input;
    }

	public override string ExportExtension(byte[] extensionData)
	{
		ByteReader reader = new ByteReader(extensionData);
		reader.Skip(4);
		
        string source = fixString(reader.ReadAscii(1025).Split('\0')[0]);
        bool caseSensitive;
        bool wildcards;
        short nComparison = reader.ReadInt16();
        if (nComparison == CASE_INSENSITIVE)
        {
            caseSensitive = false;
        }
        else
        {
            caseSensitive = true;
        }
        short nSearchMode = reader.ReadInt16();
        if (nSearchMode == SEARCH_LITERAL)
        {
            wildcards = false;
        }
        else
        {
            wildcards = true;
        }

		string sanitized = $"\"{StringUtils.SanitizeString(source)}\"";
		return CreateExtension($"{sanitized}, {caseSensitive.ToString().ToLower()}, {wildcards.ToString().ToLower()}, {nComparison}");
	}

	public override string ExportCondition(EventBase eventBase, int conditionNum, ref string nextLabel, ref int orIndex, Dictionary<string, object>? parameters = null, string ifStatement = "if (", bool isGlobal = false)
	{
		StringBuilder result = new();

		switch (conditionNum)
		{
			default:
				result.AppendLine($"// StringParser condition {conditionNum} not implemented");
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
			case ACT_SETSTRING:
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->SetString({EvaluateExpression(eventBase, 0)});");
				break;
			default:
				result.AppendLine($"// StringParser action {actionNum} not implemented");
				break;
		}

		return result.ToString();
	}

	public override string ExportExpression(Expression expression, EventBase eventBase = null)
	{
		string result;

		switch (expression.Num)
		{
			case EXP_MIDDLE:
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->SP_middle(";
				break;
			default:
				result = $"0 /* StringParser expression {expression.Num} not implemented */";
				break;
		}

		return result;
	}
}
