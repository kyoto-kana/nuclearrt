using CTFAK.CCN.Chunks.Frame;
using System.Text;
using CTFAK.MMFParser.EXE.Loaders.Events.Expressions;

public class XBOXGamepadExporter : ExtensionExporter
{
	public override string ObjectIdentifier => "JXBX";
	public override string ExtensionName => "XBOXGamepad";
	public override string CppClassName => "XBOXGamepadExtension";

	public override string ExportExtension(byte[] extensionData)
	{
		return CreateExtension("");
	}

	public override string ExportCondition(EventBase eventBase, int conditionNum, ref string nextLabel, ref int orIndex, Dictionary<string, object>? parameters = null, string ifStatement = "if (", bool isGlobal = false)
	{
		StringBuilder result = new();

		switch (conditionNum)
		{
			case 1: // Gamepad is connected
				result.AppendLine($"{ifStatement} {GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->GamepadIsConnected({EvaluateExpression(eventBase, 0)})) goto {nextLabel};");
				break;
			case 2: // Button A pressed
				result.AppendLine($"{ifStatement} {GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->ButtonAPressed({EvaluateExpression(eventBase, 0)})) goto {nextLabel};");
				break;
			case 3: // Button B pressed
				result.AppendLine($"{ifStatement} {GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->ButtonBPressed({EvaluateExpression(eventBase, 0)})) goto {nextLabel};");
				break;
			case 4: // Button X pressed
				result.AppendLine($"{ifStatement} {GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->ButtonXPressed({EvaluateExpression(eventBase, 0)})) goto {nextLabel};");
				break;
			case 5: // Button Y pressed
				result.AppendLine($"{ifStatement} {GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->ButtonYPressed({EvaluateExpression(eventBase, 0)})) goto {nextLabel};");
				break;
			case 6: // Button Back pressed
				result.AppendLine($"{ifStatement} {GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->ButtonBackPressed({EvaluateExpression(eventBase, 0)})) goto {nextLabel};");
				break;
			case 8: // Button Left Shoulder pressed
				result.AppendLine($"{ifStatement} {GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->ButtonLeftShoulderPressed({EvaluateExpression(eventBase, 0)})) goto {nextLabel};");
				break;
			case 9: // Button Left Stick pressed
				result.AppendLine($"{ifStatement} {GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->ButtonLeftStickPressed({EvaluateExpression(eventBase, 0)})) goto {nextLabel};");
				break;
			case 10: // Button Right Shoulder pressed
				result.AppendLine($"{ifStatement} {GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->ButtonRightShoulderPressed({EvaluateExpression(eventBase, 0)})) goto {nextLabel};");
				break;
			case 11: // Button Right Stick pressed
				result.AppendLine($"{ifStatement} {GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->ButtonRightStickPressed({EvaluateExpression(eventBase, 0)})) goto {nextLabel};");
				break;
			case 12: // Button Start pressed
				result.AppendLine($"{ifStatement} {GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->ButtonStartPressed({EvaluateExpression(eventBase, 0)})) goto {nextLabel};");
				break;
			case 13: // DPad Up pressed
				result.AppendLine($"{ifStatement} {GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->DPadUpPressed({EvaluateExpression(eventBase, 0)})) goto {nextLabel};");
				break;
			case 14: // DPad Down pressed
				result.AppendLine($"{ifStatement} {GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->DPadDownPressed({EvaluateExpression(eventBase, 0)})) goto {nextLabel};");
				break;
			case 15: // DPad Left pressed
				result.AppendLine($"{ifStatement} {GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->DPadLeftPressed({EvaluateExpression(eventBase, 0)})) goto {nextLabel};");
				break;
			case 16: // DPad Right pressed
				result.AppendLine($"{ifStatement} {GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->DPadRightPressed({EvaluateExpression(eventBase, 0)})) goto {nextLabel};");
				break;
			case 17: // Button pressed
				result.AppendLine($"{ifStatement} {GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->ButtonPressed({EvaluateExpression(eventBase, 0)}, {EvaluateExpression(eventBase, 1)})) goto {nextLabel};");
				break;
			case 18: // Any button pressed
				result.AppendLine($"{ifStatement} {GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->AnyButtonPressed({EvaluateExpression(eventBase, 0)})) goto {nextLabel};");
				break;
			default:
				result.AppendLine($"// XBOX Gamepad condition {conditionNum} not implemented");
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
			case 0: // Vibrate
				result.AppendLine($"{GetExtensionInstance(eventBase.ObjectInfo, eventBase.ObjectType)}->Vibrate({EvaluateExpression(eventBase, 0)}, {EvaluateExpression(eventBase, 1)}, {EvaluateExpression(eventBase, 2)}, {EvaluateExpression(eventBase, 3)});");
				break;
			default:
				result.AppendLine($"// XBOX Gamepad action {actionNum} not implemented");
				break;
		}

		return result.ToString();
	}

	public override string ExportExpression(Expression expression, EventBase eventBase = null)
	{
		string result;

		switch (expression.Num)
		{
			case 0: // StickLeftH
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->StickLeftH(";
				break;
			case 1: // StickLeftV
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->StickLeftV(";
				break;
			case 2: // StickRightH
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->StickRightH(";
				break;
			case 3: // StickRightV
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->StickRightV(";
				break;
			case 4: // TriggerLeft
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->TriggerLeft(";
				break;
			case 5: // TriggerRight
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->TriggerRight(";
				break;
			case 6: // Button A
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->ButtonPressedSpecificExpression(0,";
				break;
			case 7: // Button B
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->ButtonPressedSpecificExpression(1,";
				break;
			case 8: // Button X
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->ButtonPressedSpecificExpression(2,";
				break;
			case 9: // Button Y
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->ButtonPressedSpecificExpression(3,";
				break;
			case 10: // Button Back
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->ButtonPressedSpecificExpression(6,";
				break;
			case 12: // Left shoulder
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->ButtonPressedSpecificExpression(4,";
				break;
			case 13: // Left stick
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->ButtonPressedSpecificExpression(8,";
				break;
			case 14: // Right shoulder
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->ButtonPressedSpecificExpression(5,";
				break;
			case 15: // Right stick
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->ButtonPressedSpecificExpression(9,";
				break;
			case 16: // Button Start
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->ButtonPressedSpecificExpression(7,";
				break;
			case 17: // DPad Up
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->ButtonPressedSpecificExpression(10,";
				break;
			case 18: // DPad Down
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->ButtonPressedSpecificExpression(11,";
				break;
			case 19: // DPad Left
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->ButtonPressedSpecificExpression(12,";
				break;
			case 20: // DPad Right
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->ButtonPressedSpecificExpression(13,";
				break;
			case 21: // Button pressed
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->ButtonPressedExpression(";
				break;
			case 22: // Find pressed button
				result = $"{GetExtensionInstance(expression.ObjectInfo, expression.ObjectType)}->FindPressedButton(";
				break;
			default:
				result = $"(/* XBOX Gamepad expression {expression.Num} not implemented */";
				break;
		}

		return result;
	}
}
