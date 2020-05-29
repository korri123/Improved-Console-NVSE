#include "declarations.h"

#include "GameTiles.h"
#include "internal/dinput.h"

namespace ImprovedConsole
{
	DebugLog s_log, s_debug;

	bool(*WriteRecord)(UInt32 type, UInt32 version, const void *buffer, UInt32 length) = nullptr;
	bool(*WriteRecordData)(const void *buffer, UInt32 length) = nullptr;
	bool(*GetNextRecordInfo)(UInt32 *type, UInt32 *version, UInt32 *length) = nullptr;
	UInt32(*ReadRecordData)(void *buffer, UInt32 length) = nullptr;
	bool(*ResolveRefID)(UInt32 refID, UInt32 *outRefID) = nullptr;
	const char* (*GetSavePath)(void) = nullptr;
	const CommandInfo* (*GetCmdByOpcode)(UInt32 opcode) = nullptr;
	const CommandInfo* (*GetCmdByName)(const char* name) = nullptr;
	const CommandInfo* (*GetCmdTblStart)(void) = nullptr;
	const CommandInfo* (*GetCmdTblEnd)(void) = nullptr;
	UInt32(*GetCmdReturnType)(const CommandInfo* cmd) = nullptr;
	const char* (*GetStringVar)(UInt32 stringID) = nullptr;
	bool(*AssignString)(COMMAND_ARGS, const char *newValue) = nullptr;
	NVSEArrayVar* (*CreateArray)(const NVSEArrayElement *data, UInt32 size, Script *callingScript) = nullptr;
	NVSEArrayVar* (*CreateStringMap)(const char **keys, const NVSEArrayElement *values, UInt32 size, Script *callingScript) = nullptr;
	NVSEArrayVar* (*CreateMap)(const double* keys, const NVSEArrayElement* values, UInt32 size, Script *callingScript) = nullptr;
	bool(*AssignCommandResult)(NVSEArrayVar *arr, double *dest) = nullptr;
	void(*SetElement)(NVSEArrayVar *arr, const NVSEArrayElement &key, const NVSEArrayElement &value) = nullptr;
	void(*AppendElement)(NVSEArrayVar *arr, const NVSEArrayElement &value) = nullptr;
	UInt32(*GetArraySize)(NVSEArrayVar *arr) = nullptr;
	NVSEArrayVar* (*LookupArrayByID)(UInt32 id) = nullptr;
	bool(*GetElement)(NVSEArrayVar *arr, const NVSEArrayElement &key, NVSEArrayElement &outElement) = nullptr;
	bool(*GetElements)(NVSEArrayVar *arr, NVSEArrayElement *elements, NVSEArrayElement *keys) = nullptr;
	bool(*ExtractFormatStringArgs)(UInt32 fmtStringPos, char *buffer, COMMAND_ARGS_EX, UInt32 maxParams, ...) = nullptr;
	bool(*CallFunction)(Script *funcScript, TESObjectREFR *callingObj, TESObjectREFR *container, NVSEArrayElement *result, UInt8 numArgs, ...) = nullptr;

	DIHookControl *g_DIHookCtrl = nullptr;
	void *g_NVSEArrayMap = nullptr, *g_NVSEStringMap = nullptr;
	UInt8 *g_numPreloadMods = nullptr;
	void(*DelArrayVar)(void *varMap, UInt32 varID);
	void(*DelStringVar)(void *varMap, UInt32 varID);

	TESForm* (*LookupFormByName)(const char* name) = reinterpret_cast<TESForm* (*)(const char*)>(0x483A00);

	NVSEConsoleInterface* g_consoleInterface;

	Script* g_consoleScript;

	bool IsConsoleOpen()
	{
		auto* consoleManager = ConsoleManager::GetSingleton();
		return static_cast<bool>(ThisStdCall_B(0x4A4020, static_cast<void*>(consoleManager)));
	}
}

enum
{
	kAddr_TileGetFloat = 0xA011B0,
	kAddr_TileGetString = 0xA011F0,
	kAddr_TileSetFloat = 0xA012D0,
	kAddr_TileSetString = 0xA01350,
};

__declspec(naked) float Tile::GetValueFloat(UInt32 id)
{
	static const UInt32 procAddr = kAddr_TileGetFloat;
	__asm	jmp		procAddr
}

__declspec(naked) void Tile::SetFloat(UInt32 id, float fltVal, bool bPropagate)
{
	static const UInt32 procAddr = kAddr_TileSetFloat;
	__asm	jmp		procAddr
}

__declspec(naked) void Tile::SetString(UInt32 id, const char* strVal, bool bPropagate)
{
	static const UInt32 procAddr = kAddr_TileSetString;
	__asm	jmp		procAddr
}