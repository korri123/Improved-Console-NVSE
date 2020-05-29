#pragma once

#include "console_variables.h"
#include <memory>
#include <sstream>
#include <iomanip>
#include <regex>

#include "command_printer.h"
#include "console_arrays.h"
#include "declarations.h"
#include "GameUI.h"
#include "SafeWrite.h"

using namespace ImprovedConsole;

std::map<std::string, std::shared_ptr<ConsoleVariable>> g_consoleVarNameMap;
std::map<UInt32, std::shared_ptr<ConsoleVariable>> g_consoleVarIdMap;
std::string g_lastConsoleInput;
UInt32 g_idIncrementer = 0;
std::shared_ptr<ConsoleVariable> g_lastVariableSet;

TESForm* LookupForm(ConsoleVariable& variable)
{
	return LookupFormByID(*reinterpret_cast<UInt32*>(&variable.value));
}

std::string ConsoleVarToString(ConsoleVariable& variable)
{
	std::stringstream stream;
	switch (variable.type)
	{
	case kRetnType_Default:
	{
		stream << std::fixed << std::setprecision(2) << variable.value;
		return stream.str();
	}
	case kRetnType_Form:
	{
		stream << std::hex << *reinterpret_cast<UInt32*>(&variable.value);
		return stream.str();
	}
	case kRetnType_Array:
	{
		stream << "id " << std::fixed << std::setprecision(0) << variable.value;
		return stream.str();
	}
	case kRetnType_String:
	{
		stream << "\"" << std::string(GetStringVar(static_cast<UInt32>(variable.value))) << "\"";
		return stream.str();
	}
	}
	return "Unknown value";
}

std::string RemoveBrackets(std::string& input, std::string* bracketContents)
{
	const auto indexLBracket = input.find('[');
	const auto indexRBracket = input.find(']');

	if (indexLBracket == std::string::npos || indexRBracket == std::string::npos ||
		indexRBracket < indexLBracket || input.size() - 1 != indexRBracket)
	{
		return input;
	}
	*bracketContents = input.substr(indexLBracket + 1, indexRBracket - indexLBracket - 1);
	return input.substr(0, indexLBracket);
}

Script::ScriptVarType GetVarType(const ConsoleVariable& variable)
{
	switch (variable.type) {
	case kRetnType_Default:
		return Script::eVarType_Float;
	case kRetnType_Form:
		return Script::eVarType_Ref;
	case kRetnType_String:
		return Script::eVarType_String;
	case kRetnType_Array:
		return Script::eVarType_Array;
	case kRetnType_ArrayIndex:
	case kRetnType_Ambiguous:
	case kRetnType_Max:
	default:
		return Script::eVarType_Invalid;
	}
}

std::shared_ptr<ConsoleVariable> GetConsoleVariablePtr(const std::string& name)
{
	const auto consoleVarEntry = g_consoleVarNameMap.find(name);
	if (consoleVarEntry == g_consoleVarNameMap.end())
	{
		throw std::invalid_argument("could not find console variable " + name);
	}
	return consoleVarEntry->second;
}

ConsoleVariable& GetConsoleVariable(const std::string& name)
{
	return *GetConsoleVariablePtr(name);
}

Script::RefVariable* CreateRefVar(ConsoleVariable& variable)
{
	auto* refVar = reinterpret_cast<Script::RefVariable*>(GameHeapAlloc(sizeof(Script::RefVariable)));
	MemZero(refVar, sizeof(Script::RefVariable));
	refVar->form = LookupForm(variable);
	refVar->name.Set(variable.name.c_str());
	refVar->varIdx = variable.id;
	return refVar;
}

VariableInfo* CreateVariable(std::shared_ptr<ConsoleVariable> variable)
{
	auto* varInfo = reinterpret_cast<VariableInfo*>(GameHeapAlloc(sizeof(VariableInfo)));
	MemZero(varInfo, sizeof(VariableInfo));
	varInfo->idx = variable->id;
	varInfo->name.Set(variable->name.c_str());
	if (variable->type == kRetnType_Array)
	{
		variable->value = ConsoleArrays::InjectConsoleArray(variable->name);
	}
	varInfo->data = variable->value;
	varInfo->type = GetVarType(*variable);
	return varInfo;
}

void __stdcall InjectScriptVariables(Script* scriptObj)
{
	if (scriptObj == nullptr)
	{
		return;
	}
	//if (!IsConsoleMode()) return;

	g_consoleScript = scriptObj;

	scriptObj->modIndex = 0;

	for (const auto& variablePair : g_consoleVarIdMap)
	{
		const auto variable = variablePair.second;
		if (variable->type == kRetnType_Form)
		{
			auto* refVar = CreateRefVar(*variable);
			scriptObj->refList.Append(refVar) + 1;
			scriptObj->info.numRefs++;
		}
		auto* varInfo = scriptObj->GetVariableByName(variable->name.c_str());
		if (varInfo != nullptr)
		{
			continue;
		}
		varInfo = CreateVariable(variable);
		
		scriptObj->info.varCount++;
		scriptObj->varList.Append(varInfo);

		PrintLog("Appended to script %s -> %d", variable->name.c_str(), varInfo->idx);
	}
}

void InjectScriptBufVariables(ScriptBuffer* scriptBuffer)
{
	for (const auto& variablePair : g_consoleVarIdMap)
	{
		const auto variable = variablePair.second;

		if (variable->type == kRetnType_Form)
		{
			auto* refVar = CreateRefVar(*variable);
			scriptBuffer->refVars.Append(refVar);
			scriptBuffer->numRefs++;
		}

		auto* varInfo = CreateVariable(variable);
		scriptBuffer->vars.Append(varInfo);
		scriptBuffer->varCount++;
		PrintLog("Appended to scriptBuf %s -> %d", variable->name.c_str(), varInfo->idx);
	}
}


void __stdcall SetEventListVariables(ScriptEventList *eventList)
{
	if (!IsConsoleMode())
	{
		return;
	}
	if (eventList == nullptr) {
		PrintLog("eventList was null");
		return;
	}

	for (const auto& variablePair : g_consoleVarIdMap)
	{
		const auto variable = variablePair.second;
		auto* eventListVar = eventList->GetVariable(variable->id);
		if (eventListVar != nullptr)
		{
			continue;
		}
		eventListVar = reinterpret_cast<ScriptVar*>(GameHeapAlloc(sizeof(ScriptVar)));
		eventListVar->id = variable->id;
		eventListVar->next = nullptr;
		if (variable->type == kRetnType_Array)
		{
			eventListVar->data = ConsoleArrays::InjectConsoleArray(variable->name);
		}
		else if (variable->type == kRetnType_Form)
		{
			*reinterpret_cast<UInt64*>(&eventListVar->data) = *reinterpret_cast<UInt32*>(&variable->value);
		}
		else
		{
			eventListVar->data = variable->value;
		}
		eventList->m_vars->Append(eventListVar);

		PrintLog("Set eventlist var value of id %d to %.2f", eventListVar->id, eventListVar->data);
	}
}

bool __stdcall PrintConsoleVariable(char* consoleInputCStr)
{
	std::string arrayIndex;
	auto consoleInput = std::string(consoleInputCStr);
	consoleInput = RemoveBrackets(consoleInput, &arrayIndex);

	std::shared_ptr<ConsoleVariable> consoleVar;
	try
	{
		consoleVar = GetConsoleVariablePtr(consoleInput);
	}
	catch (std::invalid_argument& e)
	{
		PrintLog(e.what());
		return false;
	}
	
	Console_Print("<Improved Console> Variable '%s'", consoleInput.c_str());
	if (consoleVar->type == kRetnType_Array)
	{
		consoleVar->value = ConsoleArrays::InjectConsoleArray(consoleVar->name);
		
		if (!arrayIndex.empty())
		{
			if (!PrintArrayIndex(consoleVar->name, arrayIndex))
			{
				Console_Print("Array index is invalid! Could not print variable.");
			}
			return true;
		}
	}
	
	PrintVar(consoleVar->value, consoleVar->type);
	return true;
}

__declspec(naked) void HookPrintConsoleVariables()
{
	static const UInt32 return_address = 0x5AF215;
	static const UInt32 PrintScriptCompileError = 0x5AEA90;
	__asm
	{
		lea ecx, [ebp - 0x22C] // console input
		push ecx
		call PrintConsoleVariable
		test al, al
		jnz returnToFunc
		call PrintScriptCompileError
		add esp, 0xC
		returnToFunc:
		jmp return_address
	}
}

void PatchConsoleVariables()
{
	WriteRelJump(0x5AF10A, UInt32(HookPrintConsoleVariables));
}

__declspec(naked) void InjectScriptVariablesHook()
{
	const static UInt32 return_address = 0x5AC43E;
	const static UInt32 sub_404EB0 = 0x404EB0;
	__asm
	{
		call sub_404EB0 // original asm
		mov edx, [ebp - 0x18] // scriptObj
		push edx
		call InjectScriptVariables
		jmp return_address
	}
}

bool __stdcall SetConsoleVar(UInt32 variableId, double variableValue)
{
	if (!IsConsoleMode())
	{
		return false;
	}

	PrintLog("VariableID = %d", variableId);
	PrintLog("VariableValue = %.2f", variableValue);
	const auto variableEntry = g_consoleVarIdMap.find(variableId);

	if (variableEntry == g_consoleVarIdMap.end())
	{
		PrintLog("big fail");
		return false;
	}

	auto variable = variableEntry->second;

	variable->value = variableValue;

	if (g_commandWasFunction)
	{
		variable->type = g_lastReturnType;
	}
	else
	{
		if (LookupFormByID(*reinterpret_cast<UInt32*>(&variable->value)) != nullptr)
		{
			variable->type = kRetnType_Form;
		}
		else
		{
			variable->type = kRetnType_Default;
		}
	}

	if (variable->type == kRetnType_Array)
	{
		ConsoleArrays::StoreArray(variable->name, variable->value);
	}

	PrintLog("%s -> %s -> %s", variable->name.c_str(), RetnTypeToString(variable->type).c_str(), ConsoleVarToString(*variable).c_str());

	g_lastCommandWasSet = true;
	g_lastVariableSet = variable;

	Console_Print("<Improved Console> set '%s' >> (%s) %s", variable->name.c_str(),
		RetnTypeToString(variable->type).c_str(), ConsoleVarToString(*variable).c_str());
	return true;
}

__declspec(naked) void HookSetConsoleVarMap()
{
	static const UInt32 return_location = 0x5E1BD8;
	static const UInt32 return_success = 0x5E1C0D;
	static const UInt32 set_var_data = 0x5A9290;
	__asm {
		push ecx
		sub esp, 8
		fld qword ptr[ebp - 0x3C]
		fstp qword ptr[esp]
		movsx ecx, [ebp - 0x44]
		push ecx
		call SetConsoleVar
		test al, al
		jz returnLabel
		add esp, 4 // remove ecx from stack
		jmp return_success
	returnLabel :
		pop ecx
		call set_var_data
		jmp return_location
	}
}

void __stdcall AddConsoleVarToMap(char* varNameCstr)
{
	const auto varName = std::string(varNameCstr);
	if (g_consoleVarNameMap.count(varName) != 0)
	{
		return;
	}

	g_idIncrementer++;

	const auto nextVariable = std::make_shared<ConsoleVariable>(ConsoleVariable{
		varName,
		0,
		g_idIncrementer,
		kRetnType_Default
	});

	g_consoleVarIdMap[nextVariable->id] = nextVariable;
	g_consoleVarNameMap[nextVariable->name] = nextVariable;
	g_lastCommandWasSet = true;

	PrintLog("Added console var %s with id %d", nextVariable->name.c_str(), nextVariable->id);

	g_consoleInterface->RunScriptLine(g_lastConsoleInput.c_str(), InterfaceManager::GetSingleton()->debugSelection);
}

__declspec(naked) void HookAddConsoleVar()
{
	static const UInt32 return_point = 0x5B10D1;
	__asm
	{
		lea ecx, [ebp - 0x22C]
		push ecx
		call AddConsoleVarToMap
		jmp return_point
	}
}

void __stdcall SaveLastConsoleInput(char* inputStr) {
	g_lastConsoleInput = std::string(inputStr);
	g_commandWasFunction = false; // might as well use this hook
}

__declspec(naked) void SaveConsoleInputHook() {
	static const UInt32 ConsolePrint0 = 0x71D030;
	static const UInt32 return_address = 0x71B506;
	__asm
	{
		call ConsolePrint0
		lea ecx, [ebp - 0x81C]
		push ecx
		call SaveLastConsoleInput
		jmp return_address
	}
}

bool __stdcall ReplaceConsoleVarStringWithRefId(char** consoleVarNamePtr)
{
	if (!IsConsoleMode())
	{
		return false;
	}
	
	const auto* consoleVarName = *consoleVarNamePtr;
	try
	{
		auto& consoleVar = GetConsoleVariable(consoleVarName);
		if (consoleVar.type != kRetnType_Form)
		{
			PrintLog("not form");
			return false;
		}
		std::stringstream ss;
		const auto refId = *reinterpret_cast<UInt32*>(&consoleVar.value);
		ss << '"' << std::hex << refId << '"';
		const auto refIdStr = ss.str();

		const auto scriptLine = std::regex_replace(g_lastConsoleInput, std::regex(consoleVarName), refIdStr);

		PrintLog("scriptline = %s", scriptLine.c_str());

		g_consoleInterface->RunScriptLine(scriptLine.c_str(), InterfaceManager::GetSingleton()->debugSelection);

		return true;
		//PrintLog("hex value %s", newValue.c_str());
		//strcpy(*consoleVarNamePtr, newValue.c_str());
	}
	catch (std::invalid_argument& e)
	{
		PrintLog(e.what());
		return false;
	}
}

__declspec(naked) void HookSetReferences()
{
	const static UInt32 returnAddress = 0x5B0E39;
//	const static UInt32 jmpIfConsoleVar = 0x5B0EAE;
	__asm
	{
		mov ecx, [ebp-0x13C0]
		push ecx
		call ReplaceConsoleVarStringWithRefId
		test al, al
		jnz returnFromFunction
		// replaced assembly
		push 0x11841CC
		jmp returnAddress
	returnFromFunction:
		mov al, 0
		mov esp, ebp
		pop ebp
		ret 0xC
	}
}

__declspec(naked) void HookSetForms()
{
	static const UInt32 hookedCall = 0x5AF310;
	static const UInt32 returnAddress = 0x5B1C3F;
	__asm
	{
		call hookedCall
		mov edx, [ebp + 0x14] // scriptBuffer
		push edx
		call InjectScriptBufVariables
		jmp returnAddress
	}
}

__declspec(naked) void HookSetCallingReference()
{
	const static UInt32 hookedCall = 0x5AF310;
	const static UInt32 returnAddress = 0x5AFF80;
	__asm
	{
		call hookedCall
		mov edx, [ebp + 0xC] // scriptBuffer
		push edx
		call InjectScriptBufVariables
		jmp returnAddress
	}
}

void PatchInjectVariables()
{
	WriteRelJump(0x5AC439, UInt32(InjectScriptVariablesHook));
	WriteRelJump(0x5E1BD3, UInt32(HookSetConsoleVarMap));
	WriteRelJump(0x5B06AE, UInt32(HookAddConsoleVar));
	WriteRelJump(0x71B501, UInt32(SaveConsoleInputHook));
	//WriteRelJump(0x5B0E34, UInt32(HookSetReferences));
	WriteRelJump(0x5AFF7B, UInt32(HookSetCallingReference));
	//WriteRelJump(0x5AEBB1, UInt32(HookSetForms));
	WriteRelJump(0x5B1C3A, UInt32(HookSetForms));
}