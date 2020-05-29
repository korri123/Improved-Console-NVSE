#include "command_printer.h"

#include <iomanip>
#include <sstream>
#include "declarations.h"
#include "console_arrays.h"
#include "console_variables.h"
#include "SafeWrite.h"

using namespace ImprovedConsole;

double g_lastResultValue = 0.0;
CommandReturnType g_lastReturnType = kRetnType_Ambiguous;
bool g_commandWasFunction = false;
bool g_lastCommandWasSet = false;
int g_commandPrints = 0;

std::string RetnTypeToString(CommandReturnType type)
{
	switch (type)
	{
	case kRetnType_Default: return "Float";
	case kRetnType_Form: return "Form";
	case kRetnType_String: return "String";
	case kRetnType_Array: return "Array";
	}
	return "Unknown type";
}

std::string FormatElement(NVSEArrayVarInterface::Element* element) {
	std::stringstream stream;
	
	switch (element->GetType())
	{
	case ConsoleArrays::kDataType_Numeric:
	{
		stream << std::fixed << std::setprecision(2) << element->Number();
		return stream.str();
	}
	case ConsoleArrays::kDataType_Form:
	{
		auto form = element->Form();
		const char* fullName = GetFullName(form);
		const char* editorId = form->GetName();
		UInt32 refId = form->refID;

		stream << "(FormID: " << std::hex << form->refID;
		if (strlen(editorId) > 0) {
			stream << ", EditorID: " << std::string(editorId);
		}

		if (strlen(fullName) > 0) {
			stream << ", Name: " << std::string(fullName);
		}
		stream << ")";
		return stream.str();
	}
	case ConsoleArrays::kDataType_String:
		stream << "\"" << element->String() << "\"";
		return stream.str();
	case ConsoleArrays::kDataType_Array:
		stream << "Array (ID " << (UInt32)element->Array() << ")";
		return stream.str();
	}
	return "Invalid";

}

void PrintArray(NVSEArrayVar* arrayPtr)
{
	if (arrayPtr == nullptr) {
		Console_Print("Could not get array");
		return;
	}

	const int size = GetArraySize(arrayPtr);

	if (size == 0) {
		Console_Print("Empty array");
		return;
	}

	std::vector<NVSEArrayVarInterface::Element> keys(size);// <NVSEArrayVarInterface::Element>(size);
	std::vector<NVSEArrayVarInterface::Element> values(size);// <NVSEArrayVarInterface::Element>(size);
	GetElements(arrayPtr, &values[0], &keys[0]);

	for (int i = 0; i < size; i++) {
		NVSEArrayVarInterface::Element key = keys[i];
		NVSEArrayVarInterface::Element value = values[i];

		auto keyStr = FormatElement(&key);
		auto valStr = FormatElement(&value);

		Console_Print("(Key) >> %s; (Value) >> %s", keyStr.c_str(), valStr.c_str());
	}
	Console_Print("Array Size >> %d", size);
}

void PrintArray(UInt32 arrayId)
{
	NVSEArrayVarInterface::Array* arrayPtr = LookupArrayByID(arrayId);
	PrintArray(arrayPtr);
}

void PrintForm(TESForm* form)
{
	if (!form) {
		Console_Print("Invalid Form");
		return;
	}

	Console_Print("(Form ID) >> %X", form->refID);

	const auto* editorId = form->GetName();
	const auto* fullName = GetFullName(form);
	if (strlen(editorId) > 0) {
		Console_Print("(Editor ID) >> %s", editorId);
	}
	if (strlen(fullName) > 0 && strcmp(fullName, "<no name>") != 0) {
		Console_Print("(Name) >> %s", fullName);
	}
}

void PrintForm(const UInt32 formId) {
	TESForm* form = LookupFormByID(formId);
	PrintForm(form);
}

void PrintFloat(double value)
{
	Console_Print("(Float) >> %.2f", value);
}

void PrintString(const char* stringVar)
{
	Console_Print("(String) >> \"%s\"", stringVar);
}

void PrintUnknown(double value)
{
	const auto formId = *reinterpret_cast<UInt32*>(&value);
	Console_Print("(Unknown) >> %.2f / %d", value, formId);
}

void PrintVar(double value, CommandReturnType type)
{
	switch (type)
	{
	case kRetnType_Default:
	{
		PrintFloat(value);
		break;
	}
	case kRetnType_Form:
	{
		const auto formId = *reinterpret_cast<UInt32*>(&value);
		PrintForm(formId);
		break;
	}
	case kRetnType_String:
	{
		const auto* stringVar = GetStringVar(static_cast<UInt32>(value));
		PrintString(stringVar);
		break;
	}
	case kRetnType_Array:
	{
		const auto arrayId = static_cast<UInt32>(value);
		PrintArray(arrayId);
		break;
	}
	case kRetnType_ArrayIndex:
	case kRetnType_Ambiguous:
	case kRetnType_Max:
	default:
		PrintUnknown(value);
		break;
	}
}

void PrintElement(NVSEArrayElement& element)
{
	switch (static_cast<ConsoleArrays::ElementType>(element.GetType())) {
	case ConsoleArrays::kDataType_Numeric:
	{
		PrintFloat(element.Number());
		break;
	}
	case ConsoleArrays::kDataType_Form:
	{
		PrintForm(element.Form());
		break;
	}
	case ConsoleArrays::kDataType_String:
	{
		PrintString(element.String());
		break;
	}
	case ConsoleArrays::kDataType_Array:
	{
		PrintArray(element.Array());
		break;
	}
	case ConsoleArrays::kDataType_Invalid:
	default:
	{
		Console_Print("Invalid value");
		break;
	}
	}
}

bool PrintArrayIndex(std::string& varName, std::string& index)
{
	try
	{
		auto elem = ConsoleArrays::GetElementAtIndex(varName, index);
		PrintElement(elem);
		return true;
	}
	catch (std::invalid_argument& e)
	{
		PrintLog(e.what());
		return false;
	}
}

void __stdcall PrintResult(char** commandNamePtr, double* resultPtr) {
	if (commandNamePtr == nullptr || resultPtr == nullptr)
	{
		return;
	}

	const auto resultValue = *resultPtr;
	const auto* commandName = *commandNamePtr;
	const auto* commandInfo = GetCmdByName(commandName);
	auto returnType = static_cast<CommandReturnType>(GetCmdReturnType(commandInfo));

	g_commandWasFunction = true;
	g_lastReturnType = returnType;
	g_lastCommandWasSet = false;
	g_lastResultValue = resultValue;

	if (g_commandPrints != 0)
	{
		PrintLog("something already printed, aborting");
		return;
	}

	const auto formId = *reinterpret_cast<UInt32*>(resultPtr);
	if (LookupFormByID(formId)) {
		returnType = kRetnType_Form;
	}

	Console_Print("<Improved Console> %s", commandName);
	PrintVar(resultValue, returnType);

	
	/*auto end = GetCmdTblEnd();
	for (auto i = GetCmdTblStart(); i != end; i++) {
	PrintLog("%s %s", i->shortName, i->longName);;
	}*/
}

void __stdcall HookHandleVariableChanges(ScriptEventList *eventList)
{
	for (const auto& consoleVarEntry : g_consoleVarIdMap)
	{
		const auto consoleVar = consoleVarEntry.second;
		const auto* eventListVar = eventList->GetVariable(consoleVar->id);
		if (consoleVar->value != eventListVar->data)
		{
			consoleVar->value = eventListVar->data;
		}
	}
}

__declspec(naked) void PrintAnythingHook() {
	static const UInt32 return_address = 0x5E234E;
	__asm {
			call IsConsoleMode
			test al, al
			jz originalFunc

			mov ecx, [ebp + 0x8]
			push ecx
			call InjectScriptVariables

			mov edx, [ebp - 0xED0]
			mov ecx, [edx + 0x8]
			push ecx // eventList
			call SetEventListVariables

		originalFunc:
			mov g_commandPrints, 0
			call dword ptr ss : [ebp - 0xEB8]
			add esp, 0x20
			push eax
			test al, al
			jz retnFunc

			call IsConsoleMode
			test al, al
			jz retnFunc

			lea ecx, [ebp - 0xEC4] // double* result
			push ecx
			mov ecx, [ebp - 0x30] // command name
			push ecx
			call PrintResult

			mov edx, [ebp - 0xED0]
			mov ecx, [edx + 0x8]
			push ecx // eventList
			call HookHandleVariableChanges
		retnFunc:
			pop eax
			jmp return_address
	}
}

__declspec(naked) void PrintAnythingHookSet() // like above but for set
{
	__asm
	{
			mov edx, [ebp + 0x20] // event list
			push edx
			call SetEventListVariables

			call [ebp - 0x34] // run script line
			add esp, 0x20
			movzx eax, al
			push eax
			test eax, eax
			jz funcEnd

			call IsConsoleMode
			test al, al
			jz funcEnd

			mov g_commandPrints, 0
		
			mov ecx, [ebp + 0x8]// result
			push ecx
			mov ecx, [ebp - 0x24]
			push ecx
			call PrintResult

			mov edx, [ebp + 0x20] // event list
			push edx
			call HookHandleVariableChanges

		funcEnd:
			pop eax
			mov esp, ebp
			pop ebp
			ret
	}
}

__declspec(naked) void HookConsolePrint()
{
	__asm
	{
		inc g_commandPrints
		ret 0x8
	}
}

void PatchPrintAnything() {
	const static UInt32 hook = 0x5E2345;
	WriteRelJump(hook, UInt32(PrintAnythingHook));
	PatchMemoryNop(hook + 0x5, 1);

	WriteRelJump(0x5ACBB5, UInt32(PrintAnythingHookSet));
	WriteRelJump(0x71D376, UInt32(HookConsolePrint));
}