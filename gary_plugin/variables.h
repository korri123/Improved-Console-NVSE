#pragma once

extern std::string g_variableDeclarations;

void PatchVariables();

VariableInfo* GetVariableByName(const std::string& name);

