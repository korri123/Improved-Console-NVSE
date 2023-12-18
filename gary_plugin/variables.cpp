#include "GameTypes.h"
#include "GameScript.h"
#include "declarations.h"
#include "SafeWrite.h"
#include "variables.h"

#include <intrin.h>
#include <sstream>
#include <unordered_set>

#include "nvse_hooks.h"
#include "utils.h"

Script::VarInfoList g_varList;
Script::RefVarList g_refVars;
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

bool g_isInConsole = false;

void* __cdecl IsInConsoleHook()
{
	g_isInConsole = true;
	return CdeclCall<void*>(0x4B7210);
}

void __fastcall OutOfConsoleHook(Script* script)
{
	ThisStdCall(0x5AA1A0, script);
	g_isInConsole = false;
}

UInt8* GetParentBasePtr(void* addressOfReturnAddress, bool lambda = false)
{
	auto* basePtr = static_cast<UInt8*>(addressOfReturnAddress) - 4;
#if _DEBUG
	if (lambda) // in debug mode, lambdas are wrapped inside a closure wrapper function, so one more step needed
		basePtr = *reinterpret_cast<UInt8**>(basePtr);
#endif
	return *reinterpret_cast<UInt8**>(basePtr);
}
void CopyVariableList(Script::VarInfoList* from, Script::VarInfoList* to)
{
	std::unordered_set<UInt32> existingVarIds;

	for (auto* iter : *to)
	{
		if (!iter)
			continue;
		existingVarIds.insert(iter->idx);
	}

	for (auto* iter : *from)
	{
		if (!iter || existingVarIds.find(iter->idx) != existingVarIds.end())
			continue;
		existingVarIds.insert(iter->idx);
		auto* newVar = New<VariableInfo>();
		newVar->name.Set(iter->name.CStr());
		newVar->idx = iter->idx;
		newVar->type = iter->type;
		newVar->data = iter->data;
		to->Append(newVar);
	}
}

void CopyReferenceList(Script::RefVarList* from, Script::RefVarList* to)
{
	std::unordered_set<UInt32> existingRefIds;

	for (auto* iter : *to)
	{
		if (!iter)
			continue;
		existingRefIds.insert(iter->varIdx);
	}

	for (auto* iter : *from)
	{
		if (!iter || existingRefIds.find(iter->varIdx) != existingRefIds.end())
			continue;
		existingRefIds.insert(iter->varIdx);
		auto* newVar = New<Script::RefVariable>();
		newVar->name.Set(iter->name.CStr());
		newVar->varIdx = iter->varIdx;
		newVar->form = iter->form;
		to->Append(newVar);
	}
}

std::unordered_map<std::string, UInt8> ParseVariables(const std::string& input)
{
	std::unordered_map<std::string, UInt8> resultMap;
	std::istringstream stream(input);
	std::string line;

	while (std::getline(stream, line))
	{
		std::istringstream lineStream(line);
		std::vector<std::string> words;
		std::string word;

		while (lineStream >> word)
		{
			words.push_back(word);
		}

		if (words.size() == 2)
		{
			const auto& varType = words[0];
			const auto& varName = words[1];
			if (StringEqualsCI(varType, "string_var"))
			{
				resultMap[varName] = Script::eVarType_String;
			}
			else if (StringEqualsCI(varType, "array_var"))
			{
				resultMap[varName] = Script::eVarType_Array;
			}
			else if (StringEqualsCI(varType, "ref") || StringEqualsCI(varType, "reference"))
			{
				resultMap[varName] = Script::eVarType_Ref;
			}
			else if (StringEqualsCI(varType, "float"))
			{
				resultMap[varName] = Script::eVarType_Float;
			}
			else if (StringEqualsCI(varType, "int") || StringEqualsCI(varType, "integer") || StringEqualsCI(varType, "short"))
			{
				resultMap[varName] = Script::eVarType_Integer;
			}
		}
	}

	return resultMap;
}

std::unordered_map<std::string, UInt8> g_lastVarTypes;

std::string GetScriptVarText(ScriptBuffer* scriptBuf)
{
	auto varTypes = ParseVariables(scriptBuf->scriptText);
	varTypes.insert(g_lastVarTypes.begin(), g_lastVarTypes.end());
	g_lastVarTypes = varTypes;

	std::string result;
	for (auto* var : scriptBuf->vars)
	{
		if (!var) continue;
		auto varType = var->type;
		if (!var)
			continue;
		if (const auto it = varTypes.find(var->name.CStr()); it != varTypes.end())
			varType = it->second;
		switch (varType)
		{
		case Script::eVarType_Float:
			result += "float";
			break;
		case Script::eVarType_Integer:
			result += "int";
			break;
		case Script::eVarType_String:
			result += "string_var";
			break;
		case Script::eVarType_Array:
			result += "array_var";
			break;
		case Script::eVarType_Ref:
			result += "ref";
			break;
		default:
			break;
		}
		result += " ";
		result += var->name.CStr();
		result += "\r\n";
	}
	return result;
}

void PostScriptCompile(ScriptAndScriptBuffer* msg)
{
	auto* scriptBuffer = msg->scriptBuffer;
	if (!g_isInConsole)
		return;
	if (g_varList.Count() != scriptBuffer->vars.Count())
	{
		// clear our var lists
		g_varList.DeleteAll();
		g_refVars.DeleteAll();

		// copy variables from newly compiled script buf to our lists
		CopyVariableList(&scriptBuffer->vars, &g_varList);
		CopyReferenceList(&scriptBuffer->refVars, &g_refVars);

		// new variable
		const auto scriptVarText = GetScriptVarText(scriptBuffer);
		if (!scriptVarText.empty())
		{
			g_variableDeclarations = scriptVarText;
		}
		else
		{
			g_variableDeclarations.clear();
		}
	}
	g_rigIsAlpha = false;
}

void PreScriptCompile(ScriptAndScriptBuffer* msg)
{
	if (!g_isInConsole)
		return;
	auto* scriptBuffer = msg->scriptBuffer;
	auto* script = msg->script;
	// copy from my cache to scriptbuf which is about to be compiled
	CopyVariableList(&g_varList, &scriptBuffer->vars);
	CopyReferenceList(&g_refVars, &scriptBuffer->refVars);

	scriptBuffer->varCount = scriptBuffer->vars.Count();
	scriptBuffer->numRefs = scriptBuffer->refVars.Count();
	script->info.varCount = scriptBuffer->varCount;
	script->info.numRefs = scriptBuffer->numRefs;

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
		mov g_isInConsole, 0
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

__declspec(dllexport) ScriptEventList* g_scriptLocals = nullptr;


bool PreScriptExecute(Script* script, ScriptEventList** eventList)
{
	if (!g_isInConsole)
		return false;
	if (*eventList)
	{
		// If a ref with a script attached is selected it uses that event list
		return false;
	}
	if (!g_scriptLocals)
	{
		g_scriptLocals = script->CreateEventList();
	}
	if (g_scriptLocals->m_vars->Count() != g_varList.Count())
	{
		g_scriptLocals->m_vars->DeleteAll();
		auto* scriptVars = ThisStdCall<tList<ScriptVar>*>(0x5AC020, script); // Copy var list from script to event list
		FormHeap_Free(g_scriptLocals->m_vars);
		g_scriptLocals->m_vars = scriptVars;
	}

	*eventList = g_scriptLocals;
	return true;
}

ScriptEventList* __fastcall PreExecuteHook(TESObjectREFR* ref)
{
	auto* _ebp = GetParentBasePtr(_AddressOfReturnAddress());
	auto* script = *reinterpret_cast<Script**>(_ebp - 0x18);
	auto* eventList = ThisStdCall<ScriptEventList*>(0x5673E0, ref);

	PreScriptExecute(script, &eventList);
	// Original call
	return eventList;
}

void PostScriptExecute(ScriptEventList* eventList)
{
	if (!g_isInConsole)
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
	// ThisStdCall(0x41AF70, eventList, true);
}

bool __fastcall ScriptExecuteHook(Script* script, void*, TESObjectREFR* ref, ScriptEventList* eventList, TESObjectREFR* contObj, bool isPartialScript)
{
	const auto createdEventList = PreScriptExecute(script, &eventList);
	const auto result = ThisStdCall<bool>(0x5AC1E0, script, ref, eventList, contObj, isPartialScript);

	if (createdEventList && eventList)
		PostScriptExecute(eventList);

	return result;
}

char* AllocateString(const std::string& str)
{
	const size_t size = str.length() + 1;
	auto* newStr = static_cast<char*>(FormHeap_Allocate(size));
	strcpy_s(newStr, size, str.c_str());
	return newStr;
}

void AddVariableDeclarations(ScriptBuffer* buffer, Script* script, Cmd_Parse cmdParse)
{
	if (!g_isInConsole)
		return;
	// NVSE depends on variable declarations being in script text so when the expression parser is used, load the declarations.
	const auto defaultParseAddress = reinterpret_cast<Cmd_Parse>(0x5B1BA0);

	if (cmdParse == defaultParseAddress)
		// default parser, can't be NVSE
		return;

	if (g_variableDeclarations.empty())
		return;

	const auto newText = g_variableDeclarations + "\r\n" + buffer->scriptText;
	char* scriptText = AllocateString(newText);
	FormHeap_Free(script->text);
	buffer->scriptText = scriptText;
	script->text = scriptText;
}

CommandInfo* LookupCommandByOpcode(UInt32 opcode)
{
	return CdeclCall<CommandInfo*>(0x5B1120, opcode);
}

CommandInfo* __cdecl AddVariableDeclarationsHook(UInt32 opcode)
{
	auto* _ebp = GetParentBasePtr(_AddressOfReturnAddress());
	const auto command = LookupCommandByOpcode(opcode);
	auto* script = *reinterpret_cast<Script**>(_ebp + 0x8);
	auto* scriptBuffer = *reinterpret_cast<ScriptBuffer**>(_ebp + 0xC);
	AddVariableDeclarations(scriptBuffer, script, command->parse);

	return command;
}

void PatchVariables()
{
	//WriteRelJump(0x5AEE9C, UInt32(ScriptCompileHook));
	
	WriteRelCall(0x5AC4AE, UInt32(ScriptExecuteHook));
	WriteRelCall(0x5B0D3B, UInt32(AddVariableDeclarationsHook));
	WriteRelCall(0x71C828, UInt32(IsInConsoleHook));
	WriteRelCall(0x71C855, UInt32(OutOfConsoleHook));
	// 0x5AF10A Script command \"%s\" not found.
}