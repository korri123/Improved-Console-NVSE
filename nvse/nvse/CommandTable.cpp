#include "CommandTable.h"
#include "SafeWrite.h"
#include "GameAPI.h"
#include "GameData.h"
#include "GameObjects.h"
#include "GameEffects.h"
#include "GameExtraData.h"
#include "GameForms.h"
#include "GameProcess.h"
#include "GameRTTI.h"
#include "GameSettings.h"
#include "GameUI.h"
#include <string>
#include "Utilities.h"
#include "PluginManager.h"

CommandTable g_consoleCommands;
CommandTable g_scriptCommands;

#if RUNTIME

#if RUNTIME_VERSION == RUNTIME_VERSION_1_4_0_525

// 1.4.0.525 runtime
UInt32 g_offsetConsoleCommandsStart = 0x0118E8E0;
UInt32 g_offsetConsoleCommandsLast = 0x011908C0;
UInt32 g_offsetScriptCommandsStart = 0x01190910;
UInt32 g_offsetScriptCommandsLast = 0x01196D10;
static const Cmd_Parse g_defaultParseCommand = (Cmd_Parse)0x005B1BA0;

#elif RUNTIME_VERSION == RUNTIME_VERSION_1_4_0_525ng

// 1.4.0.525 nogore runtime
UInt32 g_offsetConsoleCommandsStart = 0x0118E8E0;
UInt32 g_offsetConsoleCommandsLast = 0x011908C0;
UInt32 g_offsetScriptCommandsStart = 0x01190910;
UInt32 g_offsetScriptCommandsLast = 0x01196D10;
static const Cmd_Parse g_defaultParseCommand = (Cmd_Parse)0x005B1C40;

#else
#error
#endif

#else

// 1.4.0.518 editor
UInt32 g_offsetConsoleCommandsStart = 0x00E9DB88;
UInt32 g_offsetConsoleCommandsLast = 0x00E9FB90;
UInt32 g_offsetScriptCommandsStart = 0x00E9FBB8;
UInt32 g_offsetScriptCommandsLast = 0x00EA5FB8;
static const Cmd_Parse g_defaultParseCommand = (Cmd_Parse)0x005C67E0;

#endif

struct PatchLocation
{
	UInt32	ptr;
	UInt32	offset;
	UInt32	type;
};

#if RUNTIME

#if RUNTIME_VERSION == RUNTIME_VERSION_1_4_0_525

static const PatchLocation kPatch_ScriptCommands_Start[] =
{
	{	0x005B1172, 0x00 },
	{	0x005B19B1, 0x00 },
	{	0x005B19CE, 0x04 },
	{	0x005B19F8, 0x08 },
	{	0x005BCC0A, 0x0C },
	{	0x005BCC2D, 0x00 },
	{	0x005BCC50, 0x04 },
	{	0x005BCC70, 0x0C },
	{	0x005BCC86, 0x0C },
	{	0x005BCCA6, 0x04 },
	{	0x005BCCB8, 0x04 },
	{	0x005BCCD4, 0x0C },
	{	0x005BCCE4, 0x04 },
	{	0x005BCCF4, 0x00 },
	{	0x005BCD13, 0x0C },
	{	0x005BCD23, 0x00 },
	{	0x005BCD42, 0x04 },
	{	0x005BCD54, 0x04 },
	{	0x005BCD70, 0x04 },
	{	0x005BCD80, 0x00 },
	{	0x005BCD9F, 0x00 },
	{	0x0068170B, 0x20 },
	{	0x00681722, 0x10 },
	{	0x00681752, 0x20 },
	{	0x00681AEB, 0x20 },
	{	0x00681CDE, 0x00 },
	{	0x006820FF, 0x14 },
	{	0x0068228D, 0x12 },
	{	0x006822FF, 0x14 },
	{	0x00682352, 0x14 },
	{	0x006823B2, 0x14 },
	{	0x0087A909, 0x12 },
	{	0x0087A948, 0x14 },
	{	0 },
};

static const PatchLocation kPatch_ScriptCommands_End[] =
{
	{	0x005AEA59, 0x08 },
	{	0 },
};

// 127B / 027B
// 1280 / 0280
static const PatchLocation kPatch_ScriptCommands_MaxIdx[] =
{
	{	0x00593909 + 1, 0 },
	{	0x005AEA57 + 6, 0 },
	{	0x005B115B + 3, 0 },
	{	0x005B19A0 + 3, (UInt32)(-0x1000) },
	{	0x005BCBDD + 6, (UInt32)(-0x1000) },

#if 0
	// ### investigate this function
	{	0x0067F0B8 + 3,	0 },
	{	0x0067F0D5 + 3, (UInt32)(-0x1000) },
	{	0x0067F0F4 + 3, (UInt32)(-0x1000) },
#endif

	{	0 },
};

#elif RUNTIME_VERSION == RUNTIME_VERSION_1_4_0_525ng

static const PatchLocation kPatch_ScriptCommands_Start[] =
{
	{	0x005B1212, 0x00 },
	{	0x005B1A51, 0x00 },
	{	0x005B1A6E, 0x04 },
	{	0x005B1A98, 0x08 },
	{	0x005BCC7A, 0x0C },
	{	0x005BCC9D, 0x00 },
	{	0x005BCCC0, 0x04 },
	{	0x005BCCE0, 0x0C },
	{	0x005BCCF6, 0x0C },
	{	0x005BCD16, 0x04 },
	{	0x005BCD28, 0x04 },
	{	0x005BCD44, 0x0C },
	{	0x005BCD54, 0x04 },
	{	0x005BCD64, 0x00 },
	{	0x005BCD83, 0x0C },
	{	0x005BCD93, 0x00 },
	{	0x005BCDB2, 0x04 },
	{	0x005BCDC4, 0x04 },
	{	0x005BCDE0, 0x04 },
	{	0x005BCDF0, 0x00 },
	{	0x005BCE0F, 0x00 },
	{	0x0068128B, 0x20 },
	{	0x006812A2, 0x10 },
	{	0x006812D2, 0x20 },
	{	0x0068166B, 0x20 },
	{	0x0068185E, 0x00 },
	{	0x00681C7F, 0x14 },
	{	0x00681E0D, 0x12 },
	{	0x00681E7F, 0x14 },
	{	0x00681ED2, 0x14 },
	{	0x00681F32, 0x14 },
	{	0x0087A469, 0x12 },
	{	0x0087A4A8, 0x14 },
	{	0 },
};

static const PatchLocation kPatch_ScriptCommands_End[] =
{
	{	0x005AEAF9, 0x08 },
	{	0 },
};

// 127B / 027B
// 1280 / 0280
static const PatchLocation kPatch_ScriptCommands_MaxIdx[] =
{
	{	0x00593AF9 + 1, 0 },
	{	0x005AEAF7 + 6, 0 },
	{	0x005B11FB + 3, 0 },
	{	0x005B1A40 + 3, (UInt32)(-0x1000) },
	{	0x005BCC4D + 6, (UInt32)(-0x1000) },

	{	0 },
};

#else
#error
#endif

void ApplyPatchEditorOpCodeDataList(void) {
}

#else

static const PatchLocation kPatch_ScriptCommands_Start[] =
{
	{	0x004072AF, 0x00 },
	{	0x004073FA, 0x00 },
	{	0x004A2374, 0x24 },
	{	0x004A23E8, 0x24 },
	{	0x004A2B9B, 0x00 },
	{	0x004A3CE2, 0x20 },
	{	0x004A3CF2, 0x10 },
	{	0x004A431A, 0x00 },
	{	0x004A474A, 0x20 },
	{	0x004A485F, 0x00 },
	{	0x004A4ED1, 0x00 },
	{	0x004A5134, 0x00 },
	{	0x004A58B4, 0x12 },
	{	0x004A58F5, 0x12 },
	{	0x004A5901, 0x14 },
	{	0x004A593E, 0x12 },
	{	0x004A5949, 0x14 },
	{	0x004A5A26, 0x12 },
	{	0x004A5A6D, 0x12 },
	{	0x004A5A79, 0x14 },
	{	0x004A5AD6, 0x12 },
	{	0x004A5B1D, 0x12 },
	{	0x004A5B29, 0x14 },
	{	0x004A5B7C, 0x12 },
	{	0x004A5BD9, 0x12 },
	{	0x004A5C28, 0x12 },
	{	0x004A5C34, 0x14 },
	{	0x004A600C, 0x14 },
	{	0x004A6704, 0x12 },
	{	0x004A6749, 0x12 },
	{	0x004A6755, 0x14 },
	{	0x004A684C, 0x12 },
	{	0x004A6A8F, 0x12 },
	{	0x004A6A9F, 0x14 },
	{	0x004A6BDF, 0x12 },
	{	0x004A6D30, 0x14 },
	{	0x004A6D74, 0x14 },
	{	0x004A703B, 0x12 },
	{	0x004A716D, 0x12 },
	{	0x004A71B5, 0x14 },
	{	0x004A7268, 0x14 },
	{	0x004A735A, 0x12 },
	{	0x004A7536, 0x14 },
	{	0x0059C532, 0x20 },
	{	0x0059C53B, 0x24 },
	{	0x0059C6BA, 0x24 },
	{	0x005C53F4, 0x04 },
	{	0x005C548D, 0x08 },
	{	0x005C6636, 0x00 },
	{	0x005C9499, 0x00 },
	{	0 },
};

static const PatchLocation kPatch_ScriptCommands_End[] =
{
	{	0x004A433B, 0x00 },
	{	0x0059C710, 0x24 },
	{	0x005C5372, 0x08 },
	{	0x005C541F, 0x04 },
	{	0 },
};

// 280 / 1280 / 27F
static const PatchLocation kPatch_ScriptCommands_MaxIdx[] =
{
	{	0x004A2B87 + 2,	(UInt32)(-0x1000) },
	{	0x0059C576 + 2,	(UInt32)(-0x1000) },
	{	0x005B1817 + 1,	0 },
	{	0x005C5370 + 6,	0 },

	{	0x004A439D + 2, (UInt32)(-0x1000) - 1 },
	{	0x004A43AD + 1, (UInt32)(-0x1000) - 1 },
	{	0x004A43B9 + 2, (UInt32)(-0x1000) - 1 },

	{	0x005C6625 + 1,	(UInt32)(-0x1000) - 1 },
	{	0x005C948B + 2,	(UInt32)(-0x1000) - 1 },

	{	0 },
};

#define OpCodeDataListAddress	0x004A4390	// The function containing the fith element of array kPatch_ScriptCommands_MaxIdx
#define hookOpCodeDataListAddress		0x004A5562	// call hookOpCodeDataList. This is the second reference to the function.

static void* OpCodeDataListFunc = (void*)OpCodeDataListAddress;

int __fastcall hookOpCodeDataList(UInt32 ECX, UInt32 EDX, UInt32 opCode) {  // Replacement for the vanilla version that truncate opCode by 1000
	_asm {
		mov eax, opCode
		add eax, 0x01000
		push eax
		call OpCodeDataListFunc	//	baseOpCodeDataList	// ecx and edx are still in place
	}
}

void ApplyPatchEditorOpCodeDataList(void) {
	SInt32 RelativeAddress = (UInt32)(&hookOpCodeDataList) - hookOpCodeDataListAddress - 5 /* EIP after instruction that we modify*/;
	SafeWrite32(hookOpCodeDataListAddress+1, (UInt32)RelativeAddress);
}

#endif

static void ApplyPatch(const PatchLocation * patch, UInt32 newData)
{
	for(; patch->ptr; ++patch)
	{
		switch(patch->type)
		{
		case 0:
			SafeWrite32(patch->ptr, newData + patch->offset);
			break;

		case 1:
			SafeWrite16(patch->ptr, newData + patch->offset);
			break;
		}
	}
}

bool Cmd_Default_Execute(COMMAND_ARGS)
{
	return true;
}

bool Cmd_Default_Eval(COMMAND_ARGS_EVAL)
{
	return true;
}

bool Cmd_Default_Parse(UInt32 numParams, ParamInfo * paramInfo, ScriptLineBuffer * lineBuf, ScriptBuffer * scriptBuf)
{
	return g_defaultParseCommand(numParams, paramInfo, lineBuf, scriptBuf);
}

#if RUNTIME

bool Cmd_GetNVSEVersion_Eval(COMMAND_ARGS_EVAL)
{
	*result = NVSE_VERSION_INTEGER;
	if (IsConsoleMode()) {
		Console_Print("NVSE version: %d", NVSE_VERSION_INTEGER);
	}
	return true;
}

bool Cmd_GetNVSEVersion_Execute(COMMAND_ARGS)
{
	return Cmd_GetNVSEVersion_Eval(thisObj, 0, 0, result);
}

bool Cmd_GetNVSERevision_Eval(COMMAND_ARGS_EVAL)
{
	*result = NVSE_VERSION_INTEGER_MINOR;
	if (IsConsoleMode()) {
		Console_Print("NVSE revision: %d", NVSE_VERSION_INTEGER_MINOR);
	}
	return true;
}

bool Cmd_GetNVSERevision_Execute(COMMAND_ARGS)
{
	return Cmd_GetNVSERevision_Eval(thisObj, 0, 0, result);
}

bool Cmd_GetNVSEBeta_Eval(COMMAND_ARGS_EVAL)
{
	*result = NVSE_VERSION_INTEGER_BETA;
	if (IsConsoleMode()) {
		Console_Print("NVSE beta: %d", NVSE_VERSION_INTEGER_BETA);
	}
	return true;
}

bool Cmd_GetNVSEBeta_Execute(COMMAND_ARGS)
{
	return Cmd_GetNVSEBeta_Eval(thisObj, 0, 0, result);
}

bool Cmd_DumpDocs_Execute(COMMAND_ARGS)
{
	if (IsConsoleMode()) {
		Console_Print("Dumping Command Docs");
	}
	g_scriptCommands.DumpCommandDocumentation();
	if (IsConsoleMode()) {
		Console_Print("Done Dumping Command Docs");
	}
	return true;
}

bool Cmd_tcmd_Execute(COMMAND_ARGS)
{
	_MESSAGE("tcmd");

	Console_Print("hello world");

	Debug_DumpMenus();
	Debug_DumpTraits();

	*result = 0;

	return true;
}

bool Cmd_tcmd2_Execute(COMMAND_ARGS)
{
	UInt32	arg;

	_MESSAGE("tcmd2");

	if(ExtractArgs(EXTRACT_ARGS, &arg))
	{
		Console_Print("hello args: %d", arg);
	}
	else
	{
		Console_Print("hello args: failed");
	}

	*result = 0;

	return true;
}

class Dumper {
	UInt32 m_sizeToDump;
public:
	Dumper(UInt32 sizeToDump = 512) : m_sizeToDump(sizeToDump) {}
	bool Accept(void *addr) {
		if (addr) {
			DumpClass(addr, m_sizeToDump);
		}
		return true;
	}
};

bool Cmd_tcmd3_Execute(COMMAND_ARGS)
{
	
	return true;
}

#endif

static ParamInfo kTestArgCommand_Params[] =
{
	{	"int", kParamType_Integer, 0 },
};

static ParamInfo kTestDumpCommand_Params[] = 
{
	{	"form", kParamType_ObjectID, 1},
};

DEFINE_CMD_COND(GetNVSEVersion, returns the installed version of NVSE, 0, NULL);
DEFINE_CMD_COND(GetNVSERevision, returns the numbered revision of the installed version of NVSE, 0, NULL);
DEFINE_CMD_COND(GetNVSEBeta, returns the numbered beta of the installed version of NVSE, 0, NULL);
DEFINE_COMMAND(DumpDocs, , 0, 0, NULL);
DEFINE_COMMAND(tcmd, test, 0, 0, NULL);
DEFINE_CMD_ALT(tcmd2, testargcmd ,test, 0, 1, kTestArgCommand_Params);
DEFINE_CMD_ALT(tcmd3, testdump, dump info, 0, 1, kTestDumpCommand_Params);

#define ADD_CMD(command) Add(&kCommandInfo_ ## command )
#define ADD_CMD_RET(command, rtnType) Add(&kCommandInfo_ ## command, rtnType )
#define REPL_CMD(command) Replace(GetByName(command)->opcode, &kCommandInfo_ ## command )

CommandTable::CommandTable() { }
CommandTable::~CommandTable() { }

void CommandTable::Init(void)
{

}

void CommandTable::Read(CommandInfo * start, CommandInfo * end)
{
	UInt32	numCommands = end - start;
	m_commands.reserve(m_commands.size() + numCommands);

	for(; start != end; ++start)
		Add(start);
}

void CommandTable::Add(CommandInfo * info, CommandReturnType retnType, UInt32 parentPluginOpcodeBase)
{
	UInt32	backCommandID = m_baseID + m_commands.size();	// opcode of the next command to add

	info->opcode = m_curID;

	if(m_curID == backCommandID)
	{
		// adding at the end?
		m_commands.push_back(*info);
	}
	else if(m_curID < backCommandID)
	{
		// adding to existing data?
		ASSERT(m_curID >= m_baseID);

		m_commands[m_curID - m_baseID] = *info;
	}
	else
	{
		HALT("CommandTable::Add: adding past the end");
	}

	m_curID++;

	CommandMetadata * metadata = &m_metadata[info->opcode];

	metadata->parentPlugin = parentPluginOpcodeBase;
	metadata->returnType = retnType;
}

bool CommandTable::Replace(UInt32 opcodeToReplace, CommandInfo* replaceWith)
{
	for (CommandList::iterator iter = m_commands.begin(); iter != m_commands.end(); ++iter)
	{
		if (iter->opcode == opcodeToReplace)
		{
			*iter = *replaceWith;
			iter->opcode = opcodeToReplace;
			return true;
		}
	}

	return false;
}

static CommandInfo kPaddingCommand =
{
	"", "",
	0,
	"command used for padding",
	0,
	0,
	NULL,

	Cmd_Default_Execute,
	Cmd_Default_Parse,
	NULL,
	NULL
};

void CommandTable::PadTo(UInt32 id, CommandInfo * info)
{
	if(!info) info = &kPaddingCommand;

	while(m_baseID + m_commands.size() < id)
	{
		info->opcode = m_baseID + m_commands.size();
		m_commands.push_back(*info);
	}

	m_curID = id;
}

void CommandTable::Dump(void)
{
	for(CommandList::iterator iter = m_commands.begin(); iter != m_commands.end(); ++iter)
	{
		_DMESSAGE("%08X %04X %s %s", iter->opcode, iter->needsParent, iter->longName, iter->shortName);
		gLog.Indent();

#if 0
		for(UInt32 i = 0; i < iter->numParams; i++)
		{
			ParamInfo	* param = &iter->params[i];
			_DMESSAGE("%08X %08X %s", param->typeID, param->isOptional, param->typeStr);
		}
#endif

		gLog.Outdent();
	}
}


void CommandTable::DumpAlternateCommandNames(void)
{
	for (CommandList::iterator iter= m_commands.begin(); iter != m_commands.end(); ++iter)
	{
		if (iter->shortName)
			_MESSAGE("%s", iter->shortName);
	}
}

const char* SimpleStringForParamType(UInt32 paramType)
{
	switch(paramType) {
		case kParamType_String: return "string";
		case kParamType_Integer: return "integer";
		case kParamType_Float: return "float";
		case kParamType_ObjectID: return "ref";
		case kParamType_ObjectRef: return "ref";
		case kParamType_ActorValue: return "actorValue";
		case kParamType_Actor: return "ref";
		case kParamType_SpellItem: return "ref";
		case kParamType_Axis: return "axis";
		case kParamType_Cell: return "ref";
		case kParamType_AnimationGroup: return "animGroup";
		case kParamType_MagicItem: return "ref";
		case kParamType_Sound: return "ref";
		case kParamType_Topic: return "ref";
		case kParamType_Quest: return "ref";
		case kParamType_Race: return "ref";
		case kParamType_Class: return "ref";
		case kParamType_Faction: return "ref";
		case kParamType_Sex: return "sex";
		case kParamType_Global: return "global";
		case kParamType_Furniture: return "ref";
		case kParamType_TESObject: return "ref";
		case kParamType_VariableName: return "string";
		case kParamType_QuestStage: return "short";
		case kParamType_MapMarker: return "ref";
		case kParamType_ActorBase: return "ref";
		case kParamType_Container: return "ref";
		case kParamType_WorldSpace: return "ref";
		case kParamType_CrimeType: return "crimeType";
		case kParamType_AIPackage: return "ref";
		case kParamType_CombatStyle: return "ref";
		case kParamType_MagicEffect: return "ref";
		case kParamType_FormType: return "formType";
		case kParamType_WeatherID: return "ref";
		case kParamType_NPC: return "ref";
		case kParamType_Owner: return "ref";
		case kParamType_EffectShader: return "ref";
		case kParamType_FormList:			return "ref";
		case kParamType_MenuIcon:			return "ref";
		case kParamType_Perk:				return "ref";
		case kParamType_Note:				return "ref";
		case kParamType_MiscellaneousStat:	return "ref";
		case kParamType_ImageSpaceModifier:	return "ref";
		case kParamType_ImageSpace:			return "ref";
		case kParamType_EncounterZone:		return "ref";
		case kParamType_Message:			return "ref";
		case kParamType_InvObjOrFormList:	return "ref";
		case kParamType_Alignment:			return "ref";
		case kParamType_EquipType:			return "ref";
		case kParamType_NonFormList:		return "ref";
		case kParamType_SoundFile:			return "ref";
		case kParamType_CriticalStage:		return "ref";
		case kParamType_LeveledOrBaseChar:	return "ref";
		case kParamType_LeveledOrBaseCreature:	return "ref";
		case kParamType_LeveledChar:		return "ref";
		case kParamType_LeveledCreature:	return "ref";
		case kParamType_LeveledItem:		return "ref";
		case kParamType_AnyForm:			return "ref";
		case kParamType_Reputation:			return "ref";
		case kParamType_Casino:				return "ref";
		case kParamType_CasinoChip:			return "ref";
		case kParamType_Challenge:			return "ref";
		case kParamType_CaravanMoney:		return "ref";
		case kParamType_CaravanCard:		return "ref";
		case kParamType_CaravanDeck:		return "ref";
		case kParamType_Region:				return "ref";

		// custom NVSE types
		// kParamType_StringVar is same as Integer
		case kParamType_Array:				return "array_var";

		default: return "<unknown>";
	}
}

const char* StringForParamType(UInt32 paramType)
{
	switch(paramType) {
		case kParamType_String:				return "String";
		case kParamType_Integer:			return "Integer";
		case kParamType_Float:				return "Float";
		case kParamType_ObjectID:			return "ObjectID";
		case kParamType_ObjectRef:			return "ObjectRef";
		case kParamType_ActorValue:			return "ActorValue";
		case kParamType_Actor:				return "Actor";
		case kParamType_SpellItem:			return "SpellItem";
		case kParamType_Axis:				return "Axis";
		case kParamType_Cell:				return "Cell";
		case kParamType_AnimationGroup:		return "AnimationGroup";
		case kParamType_MagicItem:			return "MagicItem";
		case kParamType_Sound:				return "Sound";
		case kParamType_Topic:				return "Topic";
		case kParamType_Quest:				return "Quest";
		case kParamType_Race:				return "Race";
		case kParamType_Class:				return "Class";
		case kParamType_Faction:			return "Faction";
		case kParamType_Sex:				return "Sex";
		case kParamType_Global:				return "Global";
		case kParamType_Furniture:			return "Furniture";
		case kParamType_TESObject:			return "Object";
		case kParamType_VariableName:		return "VariableName";
		case kParamType_QuestStage:			return "QuestStage";
		case kParamType_MapMarker:			return "MapMarker";
		case kParamType_ActorBase:			return "ActorBase";
		case kParamType_Container:			return "Container";
		case kParamType_WorldSpace:			return "WorldSpace";
		case kParamType_CrimeType:			return "CrimeType";
		case kParamType_AIPackage:			return "AIPackage";
		case kParamType_CombatStyle:		return "CombatStyle";
		case kParamType_MagicEffect:		return "MagicEffect";
		case kParamType_FormType:			return "FormType";
		case kParamType_WeatherID:			return "WeatherID";
		case kParamType_NPC:				return "NPC";
		case kParamType_Owner:				return "Owner";
		case kParamType_EffectShader:		return "EffectShader";
		case kParamType_FormList:			return "FormList";
		case kParamType_MenuIcon:			return "MenuIcon";
		case kParamType_Perk:				return "Perk";
		case kParamType_Note:				return "Note";
		case kParamType_MiscellaneousStat:	return "MiscStat";
		case kParamType_ImageSpaceModifier:	return "ImageSpaceModifier";
		case kParamType_ImageSpace:			return "ImageSpace";
		case kParamType_Unhandled2D:		return "unk2D";
		case kParamType_Unhandled2E:		return "unk2E";
		case kParamType_EncounterZone:		return "EncounterZone";
		case kParamType_Unhandled30:		return "unk30";
		case kParamType_Message:			return "Message";
		case kParamType_InvObjOrFormList:	return "InvObjectOrFormList";
		case kParamType_Alignment:			return "Alignment";
		case kParamType_EquipType:			return "EquipType";
		case kParamType_NonFormList:		return "NonFormList";
		case kParamType_SoundFile:			return "SoundFile";
		case kParamType_CriticalStage:		return "CriticalStage";
		case kParamType_LeveledOrBaseChar:	return "LeveledOrBaseChar";
		case kParamType_LeveledOrBaseCreature:	return "LeveledOrBaseCreature";
		case kParamType_LeveledChar:		return "LeveledChar";
		case kParamType_LeveledCreature:	return "LeveledCreature";
		case kParamType_LeveledItem:		return "LeveledItem";
		case kParamType_AnyForm:			return "AnyForm";
		case kParamType_Reputation:			return "Reputation";
		case kParamType_Casino:				return "Casino";
		case kParamType_CasinoChip:			return "CasinoChip";
		case kParamType_Challenge:			return "Challenge";
		case kParamType_CaravanMoney:		return "CaravanMoney";
		case kParamType_CaravanCard:		return "CaravanCard";
		case kParamType_CaravanDeck:		return "CaravanDeck";
		case kParamType_Region:				return "Region";

		// custom NVSE types
		// kParamType_StringVar is same as Integer
		case kParamType_Array:				return "ArrayVar";

		default: return "<unknown>";
	}
}

void CommandTable::DumpCommandDocumentation(UInt32 startWithID)
{
	_MESSAGE("NVSE Commands from: %#x", startWithID);

	_MESSAGE("<br><b>Function Quick Reference</b>");
	CommandList::iterator itEnd = m_commands.end();
	for (CommandList::iterator iter = m_commands.begin();iter != itEnd; ++iter) {
		if (iter->opcode >= startWithID) {
			iter->DumpFunctionDef();
		}
	}

	_MESSAGE("<hr><br><b>Functions In Detail</b>");
	for (CommandList::iterator iter = m_commands.begin();iter != itEnd; ++iter) {
		if (iter->opcode >= startWithID) {
			iter->DumpDocs();
		}
	}
}

void CommandInfo::DumpDocs() const
{
	_MESSAGE("<p><a name=\"%s\"></a><b>%s</b> ", longName, longName);
	_MESSAGE("<br><b>Alias:</b> %s<br><b>Parameters:</b>%d", (strlen(shortName) != 0) ? shortName : "none", numParams);
	if (numParams > 0) {
		for(UInt32 i = 0; i < numParams; i++)
		{
			ParamInfo	* param = &params[i];
			const char* paramTypeName = StringForParamType(param->typeID);
			if (param->isOptional != 0) {
				_MESSAGE("<br>&nbsp;&nbsp;&nbsp;<i>%s:%s</i> ", param->typeStr, paramTypeName);
			} else {
				_MESSAGE("<br>&nbsp;&nbsp;&nbsp;%s:%s ", param->typeStr, paramTypeName);
			}
		}
	}
	_MESSAGE("<br><b>Return Type:</b> FixMe<br><b>Opcode:</b> %#4x (%d)<br><b>Condition Function:</b> %s<br><b>Description:</b> %s</p>", opcode, opcode, eval ? "Yes" : "No",helpText);
}

void CommandInfo::DumpFunctionDef() const
{
	_MESSAGE("<br>(FixMe) %s<a href=\"#%s\">%s</a> ", needsParent > 0 ? "reference." : "", longName, longName);
	if (numParams > 0) {
		for(UInt32 i = 0; i < numParams; i++)
		{
			ParamInfo	* param = &params[i];
			const char* paramTypeName = StringForParamType(param->typeID);
			if (param->isOptional != 0) {
				_MESSAGE("<i>%s:%s</i> ", param->typeStr, paramTypeName);
			} else {
				_MESSAGE("%s:%s ", param->typeStr, paramTypeName);
			}
		}
	}
}


CommandInfo * CommandTable::GetByName(const char * name)
{
	for(CommandList::iterator iter = m_commands.begin(); iter != m_commands.end(); ++iter)
		if(!_stricmp(name, iter->longName) || (iter->shortName && !_stricmp(name, iter->shortName)))
			return &(*iter);

	return NULL;
}


CommandInfo* CommandTable::GetByOpcode(UInt32 opcode)
{
	// could do binary search here but padding command has opcode 0
	for (CommandList::iterator iter = m_commands.begin(); iter != m_commands.end(); ++iter)
		if (iter->opcode == opcode)
			return &(*iter);

	return NULL;
}

CommandReturnType CommandTable::GetReturnType(const CommandInfo* cmd)
{
	CommandMetadata * metadata = NULL;
	if (cmd)
		metadata = &m_metadata[cmd->opcode];
	if (metadata)
		return metadata->returnType;
	return kRetnType_Default;
}

void CommandTable::SetReturnType(UInt32 opcode, CommandReturnType retnType)
{
	CommandInfo* cmdInfo = GetByOpcode(opcode);
	if (!cmdInfo)
		_MESSAGE("CommandTable::SetReturnType() - cannot locate command with opcode %04X", opcode);
	else {
		CommandMetadata * metadata = &m_metadata[opcode];
		if (metadata)
			metadata->returnType = retnType;
	}
}

void CommandTable::RecordReleaseVersion(void)
{
	m_opcodesByRelease.push_back(GetCurID());
}

UInt32 CommandTable::GetRequiredNVSEVersion(const CommandInfo* cmd)
{
	UInt32  ver = 0;
	if (cmd) {
		if (cmd->opcode < m_opcodesByRelease[0])	// vanilla cmd
			ver = 0;
		else if (cmd->opcode >= kNVSEOpcodeTest)	// plugin cmd, we have no way of knowing
			ver = -1;
		else {
			for (UInt32 i = 0; i < m_opcodesByRelease.size(); i++) {
				if (cmd->opcode >= m_opcodesByRelease[i]) {
					ver = i;
				}
				else {
					break;
				}
			}
		}
	}

	return ver;
}

void CommandTable::RemoveDisabledPlugins(void)
{
	for (CommandList::iterator iter = m_commands.begin(); iter != m_commands.end(); ++iter)
	{
		CommandMetadata * metadata = &m_metadata[iter->opcode];
		if (metadata)
			// plugin failed to load but still registered some commands?
			// realistically the game is going to go down hard if this happens anyway
			if(g_pluginManager.LookupHandleFromBaseOpcode(metadata->parentPlugin) == kPluginHandle_Invalid)
				Replace(iter->opcode, &kPaddingCommand);
	}
}

static char * kNVSEname = "NVSE";

static PluginInfo g_NVSEPluginInfo =
{
	PluginInfo::kInfoVersion,
	kNVSEname,
	NVSE_VERSION_INTEGER,
};

PluginInfo * CommandTable::GetParentPlugin(const CommandInfo * cmd)
{
	if (!cmd->opcode || cmd->opcode<kNVSEOpcodeStart)
		return NULL;

	if (cmd->opcode < kNVSEOpcodeTest)
		return &g_NVSEPluginInfo;

	CommandMetadata * metadata = &m_metadata[cmd->opcode];
	if (metadata) {
		PluginInfo	* info = g_pluginManager.GetInfoFromBase(metadata->parentPlugin);
		if (info)
			return info;
	}
	return NULL;
}

void ImportConsoleCommand(const char * name)
{
	CommandInfo	* info = g_consoleCommands.GetByName(name);
	if(info)
	{
		CommandInfo	infoCopy = *info;

		std::string	newName;

		newName = std::string("con_") + name;

		infoCopy.shortName = "";
		infoCopy.longName = _strdup(newName.c_str());

		g_scriptCommands.Add(&infoCopy);

//		_MESSAGE("imported console command %s", name);
	}
	else
	{
		_WARNING("couldn't find console command (%s)", name);

		// pad it
		g_scriptCommands.Add(&kPaddingCommand);
	}
}

// internal commands added at the end
void CommandTable::AddDebugCommands()
{
	
}

void CommandTable::AddCommandsV1()
{
}

void CommandTable::AddCommandsV3s()
{
}

namespace PluginAPI {
	const CommandInfo* GetCmdTblStart() { return g_scriptCommands.GetStart(); }
	const CommandInfo* GetCmdTblEnd() { return g_scriptCommands.GetEnd(); }
	const CommandInfo* GetCmdByOpcode(UInt32 opcode) { return g_scriptCommands.GetByOpcode(opcode); }
	const CommandInfo* GetCmdByName(const char* name) { return g_scriptCommands.GetByName(name); }
	UInt32 GetCmdRetnType(const CommandInfo* cmd) { return g_scriptCommands.GetReturnType(cmd); }
	UInt32 GetReqVersion(const CommandInfo* cmd) { return g_scriptCommands.GetRequiredNVSEVersion(cmd); }
	const PluginInfo* GetCmdParentPlugin(const CommandInfo* cmd) { return g_scriptCommands.GetParentPlugin(cmd); }
}
