#include "main.h"

#include "init_patches.h"
#include "command_printer.h"
#include "console_scroll.h"
#include "nvse_hooks.h"
#include "declarations.h"
#include "interpreter.h"
#include "SimpleINI.h"
#include "utils.h"
#include "variables.h"

using namespace ImprovedConsole;

#define REG_CMD(name) nvse->RegisterCommand(&kCommandInfo_##name);

bool g_scrollWheelSupport;



void MessageHandler(NVSEMessagingInterface::Message* msg)
{
	if (msg->type == NVSEMessagingInterface::kMessage_MainGameLoop && g_scrollWheelSupport)
	{
		CheckForScrollWheelCallback();
	}
	else if (msg->type == NVSEMessagingInterface::kMessage_Precompile)
	{
		PreScriptCompile(static_cast<ScriptAndScriptBuffer*>(msg->data));
	}
	else if (msg->type == NVSEMessagingInterface::kMessage_ScriptCompile)
	{
		PostScriptCompile(static_cast<ScriptAndScriptBuffer*>(msg->data));
	}
}

void InitInterfaces(const NVSEInterface *nvse)
{
	const auto* cmdTable = static_cast<NVSECommandTableInterface*>(nvse->QueryInterface(kInterface_CommandTable));
	GetCmdByOpcode = cmdTable->GetByOpcode;
	GetCmdByName = cmdTable->GetByName;
	GetCmdReturnType = cmdTable->GetReturnType;
	GetCmdTblStart = cmdTable->Start;
	GetCmdTblEnd = cmdTable->End;
	const auto* strInterface = static_cast<NVSEStringVarInterface*>(nvse->QueryInterface(kInterface_StringVar));
	GetStringVar = strInterface->GetString;
	AssignString = strInterface->Assign;
	const auto* arrInterface = static_cast<NVSEArrayVarInterface*>(nvse->QueryInterface(kInterface_ArrayVar));
	CreateArray = arrInterface->CreateArray;
	CreateStringMap = arrInterface->CreateStringMap;
	CreateMap = arrInterface->CreateMap;
	AssignCommandResult = arrInterface->AssignCommandResult;
	SetElement = arrInterface->SetElement;
	AppendElement = arrInterface->AppendElement;
	GetArraySize = arrInterface->GetArraySize;
	LookupArrayByID = arrInterface->LookupArrayByID;
	GetElement = arrInterface->GetElement;
	GetElements = arrInterface->GetElements;
	g_arrayInterface = arrInterface;

	const auto* nvseData = static_cast<NVSEDataInterface*>(nvse->QueryInterface(kInterface_Data));
	g_DIHookCtrl = static_cast<DIHookControl*>(nvseData->GetSingleton(NVSEDataInterface::kNVSEData_DIHookControl));
	g_consoleInterface = static_cast<NVSEConsoleInterface*>(nvse->QueryInterface(kInterface_Console));

	const auto* msgInterface = static_cast<NVSEMessagingInterface*>(nvse->QueryInterface(kInterface_Messaging));
	msgInterface->RegisterListener(nvse->GetPluginHandle(), "NVSE", MessageHandler);
}

bool NVSEPlugin_Query(const NVSEInterface *nvse, PluginInfo *info)
{
	info->infoVersion = PluginInfo::kInfoVersion;
	info->name = "Improved Console";
	info->version = IMPROVED_CONSOLE_PLUGIN_VERSION;

	s_log.OpenWrite("ImprovedConsole.log");
	if (nvse->runtimeVersion != RUNTIME_VERSION_1_4_0_525)
	{
		PrintLog("ERROR: Unsupported runtime version (%08X).", nvse->runtimeVersion);
		return false;
	}
	auto s_nvseVersion = (nvse->nvseVersion >> 24) + (((nvse->nvseVersion >> 16) & 0xFF) * 0.1) + (((nvse->nvseVersion & 0xFF) >> 4) * 0.01);
	if (nvse->nvseVersion < PACKED_NVSE_VERSION)
	{
		const auto* msg = "IMPROVED CONSOLE: NVSE version is outdated (v%.2f). This plugin requires v6.2.3 minimum. Download newest xNVSE from www.github.com/xNVSE/NVSE";
		char buffer[512];
		snprintf(buffer, sizeof(buffer), msg, s_nvseVersion);
		PrintLog(buffer);
		ShowErrorMessageBox(buffer);
		return false;
	}
	if (nvse->isEditor)
		return false;

	PrintLog("Improved Console initialized");
	return true;
}

bool NVSEPlugin_Load(const NVSEInterface *nvse)
{
	const auto iniPath = GetCurPath() + R"(\Data\NVSE\Plugins\improved_console.ini)";

	CSimpleIniA ini;
	ini.SetUnicode();
	const auto errVal = ini.LoadFile(iniPath.c_str());

	g_scrollWheelSupport = ini.GetOrCreate("General", "bConsoleScrollWheelSupport", 1, "; Enable console scroll wheel support");

	ini.SaveFile(iniPath.c_str(), false);

	// PatchRemoteDesktop();
	//if (!PatchNVSE()) return false;

	InitInterfaces(nvse);
	// PatchInjectVariables();
	PatchPrintAnything();
	PatchVariables();
	// PatchConsoleVariables();
	PatchIsAlpha();
	PatchTokenTypeDefs();
	PatchInterpreter();

	//PrepareForHell();
	//REG_CMD(Assign);
	//InitStewie(nvse);
	//PatchStupidBug();
	//PatchMCM();

	
	
	return true;
}

