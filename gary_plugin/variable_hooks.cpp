#include "GameTypes.h"
#include "GameScript.h"
#include "declarations.h"
#include "SafeWrite.h"

Script::VarInfoList g_varList;
Script::RefVarList g_refVars;
UInt32 g_varCount = 0;
UInt32 g_numRefs = 0;
ScriptEventList g_eventList;

void __fastcall PostScriptCompile(ScriptBuffer* scriptBuffer)
{
	// clear our var lists
	g_varList.RemoveAll();
	g_refVars.RemoveAll();
	
	// copy variables from newly compiled script buf to our lists
	Game::CopyVarList(&scriptBuffer->vars, &g_varList);
	Game::CopyRefList(&scriptBuffer->refVars, &g_refVars);
	g_varCount = scriptBuffer->varCount;
	g_numRefs = scriptBuffer->numRefs;
}

__declspec(naked) void PostScriptCompileHook()
{
	const static UInt32 destroyScriptBuf = 0x5AE5C0;
	const static UInt32 returnAddress = 0x5AEEB9;
	__asm
	{
		// my stuff
		push ecx
		call PostScriptCompile // ScriptBuffer* is in ecx, func is fastcall
		pop ecx

		// original asm
		call destroyScriptBuf
		jmp returnAddress
	}
}

void __fastcall PreScriptCompile(ScriptBuffer* scriptBuffer, Script* script)
{
	// copy from my copy to scriptbuf which is about to be compiled
	Game::CopyVarList(&g_varList, &scriptBuffer->vars);
	Game::CopyRefList(&g_refVars, &scriptBuffer->refVars);

	script->info.varCount = g_varCount;
	script->info.numRefs = g_numRefs;
	scriptBuffer->varCount = g_varCount;
	scriptBuffer->numRefs = g_numRefs;
}

__declspec(naked) void PreScriptCompileHook()
{
	const static UInt32 getScriptText = 0x55B980;
	const static UInt32 returnAddress = 0x5AEE8E;
	__asm
	{
		// original asm
		call getScriptText
		
		// my stuff
		push eax

		mov edx, [ebp + 0x8] // Script*
		lea ecx, [ebp - 0x64] // ScriptBuffer*
		call PreScriptCompile
		
		pop eax
		jmp returnAddress
	}
}

void __fastcall InsertEventList(ScriptEventList* eventList)
{
	
	
}

__declspec(naked) void InsertEventListHook()
{
	const static UInt32 returnAddress = 0x5AC48E;
	__asm
	{
		mov ecx, [ebp - 0x14] // ScriptEventList*
		call InsertEventList
		jmp returnAddress
	}
}

void PatchVariables()
{
	WriteRelCall(0x5AEE89, UInt32(PreScriptCompileHook));
	WriteRelCall(0x5AEEB4, UInt32(PostScriptCompileHook));
	WriteRelCall(0x5AC487, UInt32(InsertEventListHook));
}