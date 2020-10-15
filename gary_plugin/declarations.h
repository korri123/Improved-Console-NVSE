#pragma once

#include <internal/dinput.h>

#include "nvse/PluginAPI.h"

namespace ImprovedConsole
{
	extern DebugLog s_log, s_debug;

	extern bool(*WriteRecord)(UInt32 type, UInt32 version, const void *buffer, UInt32 length);
	extern bool(*WriteRecordData)(const void *buffer, UInt32 length);
	extern bool(*GetNextRecordInfo)(UInt32 *type, UInt32 *version, UInt32 *length);
	extern UInt32(*ReadRecordData)(void *buffer, UInt32 length);
	extern bool(*ResolveRefID)(UInt32 refID, UInt32 *outRefID);
	extern const char* (*GetSavePath)(void);
	extern const CommandInfo* (*GetCmdByOpcode)(UInt32 opcode);
	extern const CommandInfo* (*GetCmdByName)(const char* name);
	extern const CommandInfo* (*GetCmdTblStart)(void);
	extern const CommandInfo* (*GetCmdTblEnd)(void);
	extern UInt32(*GetCmdReturnType)(const CommandInfo* cmd);
	extern const char* (*GetStringVar)(UInt32 stringID);
	extern bool(*AssignString)(COMMAND_ARGS, const char *newValue);
	extern NVSEArrayVar* (*CreateArray)(const NVSEArrayElement *data, UInt32 size, Script *callingScript);
	extern NVSEArrayVar* (*CreateStringMap)(const char **keys, const NVSEArrayElement *values, UInt32 size, Script *callingScript);
	extern NVSEArrayVar* (*CreateMap)(const double* keys, const NVSEArrayElement* values, UInt32 size, Script *callingScript);
	extern bool(*AssignCommandResult)(NVSEArrayVar *arr, double *dest);
	extern void(*SetElement)(NVSEArrayVar *arr, const NVSEArrayElement &key, const NVSEArrayElement &value);
	extern void(*AppendElement)(NVSEArrayVar *arr, const NVSEArrayElement &value);
	extern UInt32(*GetArraySize)(NVSEArrayVar *arr);
	extern NVSEArrayVar* (*LookupArrayByID)(UInt32 id);
	extern bool(*GetElement)(NVSEArrayVar *arr, const NVSEArrayElement &key, NVSEArrayElement &outElement);
	extern bool(*GetElements)(NVSEArrayVar *arr, NVSEArrayElement *elements, NVSEArrayElement *keys);
	extern bool(*ExtractFormatStringArgs)(UInt32 fmtStringPos, char *buffer, COMMAND_ARGS_EX, UInt32 maxParams, ...);
	extern bool(*CallFunction)(Script *funcScript, TESObjectREFR *callingObj, TESObjectREFR *container, NVSEArrayElement *result, UInt8 numArgs, ...);

	extern DIHookControl* g_DIHookCtrl;
	
	extern void *g_NVSEArrayMap, *g_NVSEStringMap;
	extern UInt8 *g_numPreloadMods;
	extern void(*DelArrayVar)(void *varMap, UInt32 varID);
	extern void(*DelStringVar)(void *varMap, UInt32 varID);

	extern TESForm* (*LookupFormByName)(const char* name);

	extern NVSEConsoleInterface* g_consoleInterface;

	extern Script* g_consoleScript;

	bool IsConsoleOpen();

	extern const NVSEArrayVarInterface* g_arrayInterface;
}

namespace Game
{
	extern void (__cdecl *CopyVarList)(Script::VarInfoList* from, Script::VarInfoList* to);
	extern void (__cdecl *CopyRefList)(Script::RefVarList* from, Script::RefVarList* to);
}