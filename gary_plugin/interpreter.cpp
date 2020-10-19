#include "interpreter.h"
#include "GameScript.h"
#include "SafeWrite.h"
#include "ScriptUtils.h"
#include "declarations.h"

void __fastcall Interpret(ScriptBuffer* scriptBuffer, ScriptLineBuffer* lineBuffer, Script* script)
{
	ExpressionParser parser(scriptBuffer, lineBuffer);
	auto type = parser.ParseSubExpression(strlen(scriptBuffer->scriptText));
	if (type == kTokenType_Empty || type == kTokenType_Invalid)
	{
		Console_Print("<Improved Console> Could not parse expression");
		return;
	}
	auto* eventList = script->CreateEventList();
	auto* scriptData = lineBuffer->dataBuf;
	script->data = scriptData;
	double result = 0;
	UInt32 opcodeOffset = 0;
	ExpressionEvaluator eval(nullptr, scriptData, nullptr, nullptr, script, eventList, &result, &opcodeOffset);
	auto* token = eval.Evaluate();
	GameHeapFree(eventList);
}

__declspec(naked) void InterpretHook()
{
	const static UInt32 returnAddress = 0x5AF10F;
	__asm
	{
		mov edx, [ebp]
		mov edx, [edx + 0x8] // Script*
		push edx
		mov edx, [ebp - 0x47] // ScriptLineBuffer*
		mov ecx, [ebp + 0x80] // ScriptBuffer*
		call Interpret
		jmp returnAddress
	}
}

void PatchInterpreter()
{
	WriteRelJump(0x5AF10A, UInt32(InterpretHook)); // Script command \"%s\" not found.	
}