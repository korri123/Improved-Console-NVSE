#include "main.h"
#include "init_patches.h"
#include "command_printer.h"
#include "console_scroll.h"
#include "console_variables.h"
#include "string_args_hooks.h"
#include "declarations.h"

using namespace ImprovedConsole;

#define REG_CMD(name) nvse->RegisterCommand(&kCommandInfo_##name);

void MessageHandler(NVSEMessagingInterface::Message* msg)
{
	if (msg->type == NVSEMessagingInterface::kMessage_PostLoad)
	{
		auto* const jip = GetModuleHandle("jip_nvse.dll");
		if (jip != nullptr)
		{
			const auto JIPMainLoopAddCallback = reinterpret_cast<void(*)(void*, void*, UInt32, UInt32)>(GetProcAddress(jip, "MainLoopAddCallbackEx"));
			if (JIPMainLoopAddCallback != nullptr)
			{
				PrintLog("Registered JIP callback");
				JIPMainLoopAddCallback(static_cast<void*>(CheckForScrollWheelCallback), nullptr, 0, 1);
			}
			else
			{
				PrintLog("Could not register JIP callback");
			}
		}
		else
		{
			PrintLog("jip is nullptr");
		}
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

	s_log.Create("ImprovedConsole.log");
	if (nvse->runtimeVersion != RUNTIME_VERSION_1_4_0_525)
	{
		PrintLog("ERROR: Unsupported runtime version (%08X).", nvse->runtimeVersion);
		return false;
	}
	auto s_nvseVersion = (nvse->nvseVersion >> 24) + (((nvse->nvseVersion >> 16) & 0xFF) * 0.1) + (((nvse->nvseVersion & 0xFF) >> 4) * 0.01);
	if (nvse->nvseVersion < 0x5010040)
	{
		PrintLog("ERROR: NVSE version is outdated (v%.2f). This plugin requires v5.14 minimum.", s_nvseVersion);
		return false;
	}
	PrintLog("Improved Console Initialized");
	return true;
}

bool NVSEPlugin_Load(const NVSEInterface *nvse)
{
	
	PatchRemoteDesktop();
	if (!PatchNVSE()) return false;

	InitInterfaces(nvse);
	PatchInjectVariables();
	PatchPrintAnything();
	PatchConsoleVariables();
	PatchIsAlpha();

	//PrepareForHell();
	//REG_CMD(Assign);
	//InitStewie(nvse);
	//PatchStupidBug();
	//PatchMCM();
	
	return true;
}

