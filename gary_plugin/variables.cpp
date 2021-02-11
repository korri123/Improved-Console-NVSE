#include "GameTypes.h"
#include "GameScript.h"
#include "declarations.h"
#include "SafeWrite.h"
#include "variables.h"
#include "nvse_hooks.h"

Script::VarInfoList g_varList;
Script::RefVarList g_refVars;
UInt32 g_varCount = 0;
UInt32 g_numRefs = 0;
ScriptEventList g_eventList;
std::string g_variableDeclarations;

VariableInfo* GetVariableByName(const std::string& name)
{
	auto* cachedVarIter = g_varList.Head();

	while (cachedVarIter)
	{
		if (cachedVarIter->Data() && _stricmp(cachedVarIter->Data()->name.CStr(), name.c_str()) == 0)
			return cachedVarIter->Data();
		cachedVarIter = cachedVarIter->Next();
	}
	return nullptr;
}

void __fastcall PostScriptCompile(ScriptBuffer* scriptBuffer)
{
	if (g_varCount != scriptBuffer->varCount)
	{
		// clear our var lists
		g_varList.RemoveAll();
		g_refVars.RemoveAll();

		// copy variables from newly compiled script buf to our lists
		Game::CopyVarList(&scriptBuffer->vars, &g_varList);
		Game::CopyRefList(&scriptBuffer->refVars, &g_refVars);

		// new variable
		g_variableDeclarations += scriptBuffer->scriptText;
		g_variableDeclarations += "\r\n";

		g_varCount = scriptBuffer->varCount;
		g_numRefs = scriptBuffer->numRefs;
	}
	g_rigIsAlpha = false;
}

void __fastcall PreScriptCompile(ScriptBuffer* scriptBuffer, Script* script)
{
	// copy from my cache to scriptbuf which is about to be compiled
	Game::CopyVarList(&g_varList, &scriptBuffer->vars);
	Game::CopyRefList(&g_refVars, &scriptBuffer->refVars);

	scriptBuffer->varCount = scriptBuffer->vars.Count();
	scriptBuffer->numRefs = scriptBuffer->refVars.Count();

	g_rigIsAlpha = true;
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
		test al, al
		jz returnToGame

		push eax
		mov edx, [ebp + 0x8] // Script*
		lea ecx, [ebp - 0x64] // ScriptBuffer*
		call PostScriptCompile
		pop eax
	returnToGame:
		jmp returnAddress
	}
}

bool g_createdEventList = false;

void __fastcall PreScriptExecute(Script* script, ScriptEventList** eventList)
{
	if (*eventList)
	{
		// If a ref with a script attached is selected it uses that event list
		g_createdEventList = false;
		return;
	}
		
	*eventList = script->CreateEventList();
	g_createdEventList = true;
}

void __fastcall PostScriptExecute(ScriptEventList* eventList)
{
	if (!g_createdEventList)
		return;
	// save new values after script has executed
	auto* eventListVarIter = eventList->m_vars->Head();
	auto* cachedVarIter = g_varList.Head();
	
	while (eventListVarIter && cachedVarIter)
	{
		if (eventListVarIter->Data() && cachedVarIter->Data())
		{
			cachedVarIter->Data()->data = eventListVarIter->Data()->data;
		}
		eventListVarIter = eventListVarIter->Next();
		cachedVarIter = cachedVarIter->Next();
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

const char* g_lastScriptTextData = nullptr;

void __fastcall AddVariableDeclarations(ScriptBuffer* buffer, UInt32 cmdParse)
{
	// NVSE depends on variable declarations being in script text so when the expression parser is used, load the declarations.
	const static UInt32 s_defaultParseAddress = 0x5B1BA0;

	g_lastScriptTextData = buffer->scriptText;
	if (cmdParse == s_defaultParseAddress)
		// default parser, can't be NVSE
		return;
	buffer->scriptText = g_variableDeclarations.c_str();
}

void __fastcall RemoveVariableDeclarations(ScriptBuffer* buffer)
{
	buffer->scriptText = g_lastScriptTextData;
}

__declspec(naked) void AddVariableDeclarationsHook()
{
	const static UInt32 returnAddress = 0x5B0F09;
	__asm
	{
		mov edx, [ebp - 0x13C8] // parse function
		mov ecx, [ebp + 0xC] // ScriptBuffer*
		call AddVariableDeclarations
		
		// original asm
		call [ebp - 0x13C8]
		add esp, 10
		push eax

		mov ecx, [ebp + 0xC] // ScriptBuffer*
		call RemoveVariableDeclarations

		pop eax
		jmp returnAddress
	}
}

void PatchVariables()
{
	WriteRelJump(0x5AEE9C, UInt32(ScriptCompileHook));
	WriteRelJump(0x5AC4AE, UInt32(ScriptExecuteHook));
	WriteRelJump(0x5B0F00, UInt32(AddVariableDeclarationsHook));
	// 0x5AF10A Script command \"%s\" not found.
}