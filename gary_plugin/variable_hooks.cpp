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

__declspec(naked) void ScriptCompileHook()
{
	const static UInt32 scriptCompile = 0x5AEB90;
	const static UInt32 returnAddress = 0x5AEEA1;
	__asm
	{
		push ecx
		mov edx, [ebp + 0x8] // Script*
		lea ecx, [ebp - 0x64] // ScriptBuffer*
		call PreScriptCompile
		pop ecx
		call scriptCompile // original asm
		push eax
		mov edx, [ebp + 0x8] // Script*
		lea ecx, [ebp - 0x64] // ScriptBuffer*
		call PostScriptCompile
		pop eax
		jmp returnAddress
	}
}

ScriptEventList* g_scriptEventList = nullptr;

void __fastcall PreScriptExecute(Script* script, ScriptEventList** eventList)
{
	if (*eventList)
		// If a ref with a script attached is selected it uses that event list
		return;
	*eventList = script->CreateEventList();
}

void __fastcall PostScriptExecute(ScriptEventList* eventList)
{
	auto* eventListVarIter = eventList->m_vars->Head();
	auto* cachedVarIter = g_varList.Head();
	
	while (eventListVarIter && cachedVarIter)
	{
		// save new values after script has executed
		cachedVarIter->data->data = eventListVarIter->data->data;
		eventListVarIter = eventListVarIter->next; cachedVarIter = cachedVarIter->next;
	}
	GameHeapFree(eventList);
}

__declspec(naked) void ScriptExecuteHook()
{
	const static UInt32 scriptExecute = 0x5AC1E0;
	const static UInt32 returnAddress = 0x5AC4B3;
	__asm
	{
		push ecx
		lea edx, [ebp - 0x14] // ScriptEventList**
		call PreScriptExecute // ecx already contains Script*

		// event list has already been pushed to stack, edit it
		mov ecx, [ebp - 0x14]
		mov [esp + 0x8], ecx

		pop ecx
		call scriptExecute // original asm
		mov ecx, [ebp - 0x14]
		call PostScriptExecute
		jmp returnAddress
	}
}

void PatchVariables()
{
	WriteRelJump(0x5AEE9C, UInt32(ScriptCompileHook));
	WriteRelJump(0x5AC4AE, UInt32(ScriptExecuteHook));
}