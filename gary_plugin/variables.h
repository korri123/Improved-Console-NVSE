#pragma once

extern std::string g_variableDeclarations;
__declspec(dllexport) extern bool g_isInConsole;

void PatchVariables();

VariableInfo* GetVariableByName(const std::string& name);

struct ScriptAndScriptBuffer
{
	Script* script;
	ScriptBuffer* scriptBuffer;
};

void PostScriptCompile(ScriptAndScriptBuffer* scriptBuffer);
void PreScriptCompile(ScriptAndScriptBuffer* scriptBuffer);