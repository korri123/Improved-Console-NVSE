#include "interpreter.h"
#include "GameScript.h"
#include "SafeWrite.h"
#include "ScriptUtils.h"
#include "declarations.h"
#include "variables.h"

void __fastcall Interpret(ScriptBuffer* scriptBuffer, ScriptLineBuffer* lineBuffer, Script* script)
{
	Console_Print("<Improved Console> (NVSE expression) >>");
	// huge hack but i don't give a shit :D
	std::string interpretCmd = "print $(" + std::string(scriptBuffer->scriptText) + ")";
	Script::RunScriptLine(interpretCmd.c_str());
}

__declspec(naked) void InterpretHook()
{
	const static UInt32 returnAddress = 0x5AF10F;
	__asm
	{
		mov edx, [ebp]
		mov edx, [edx + 0x8] // Script*
		push edx
		mov edx, [ebp - 0x23C] // ScriptLineBuffer*
		mov ecx, [ebp + 0x8] // ScriptBuffer*
		call Interpret
		jmp returnAddress
	}
}

void PatchInterpreter()
{
	WriteRelJump(0x5AF10A, UInt32(InterpretHook)); // Script command \"%s\" not found.	
}