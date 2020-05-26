#include <string>

#include "CommandTable.h"

extern double g_lastResultValue;
extern CommandReturnType g_lastReturnType;
extern bool g_commandWasFunction;
extern bool g_lastCommandWasSet;

std::string RetnTypeToString(CommandReturnType type);

void PrintVar(double value, CommandReturnType type);
bool PrintArrayIndex(std::string& varName, std::string& index);

void PatchPrintAnything();
