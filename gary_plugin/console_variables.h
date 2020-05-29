#pragma once
#include <memory>

#include "CommandTable.h"
#include "GameScript.h"

struct ConsoleVariable
{
	std::string name;
	double value;
	UInt32 id;
	CommandReturnType type;
};

extern std::map<std::string, std::shared_ptr<ConsoleVariable>> g_consoleVarNameMap;
extern std::map<UInt32, std::shared_ptr<ConsoleVariable>> g_consoleVarIdMap;
extern std::string g_lastConsoleInput;
extern UInt32 g_idIncrementer;
extern std::shared_ptr<ConsoleVariable> g_lastVariableSet;

std::string ConsoleVarToString(ConsoleVariable& variable);
ConsoleVariable& GetConsoleVariable(const std::string& name);
Script::ScriptVarType GetVarType(const ConsoleVariable& variable);

void __stdcall SetEventListVariables(ScriptEventList *eventList);
void __stdcall InjectScriptVariables(Script* scriptObj);

void InjectScriptBufVariables(ScriptBuffer* scriptBuffer);
void PatchConsoleVariables();
void PatchInjectVariables();