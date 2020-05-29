#pragma once
#include "memscan.h"
#include "SafeWrite.h"
#include <fstream>


#include "console_arrays.h"
#include "console_variables.h"
#include "ScriptUtils.h"


void PatchRemoteDesktop() {
	// 0x86A8AB jz -> jmp
	SafeWrite8(0x86A8AB, 0xEB);
}

__declspec(naked) void ResetQuestHook()
{
	static const UInt32 retnAddr = 0x60D881;
	__asm
	{
		mov ecx, [ecx + 0x5C]
		mov eax, 0x5A8BC0
		call eax
		mov ecx, [ebp - 0x2C]
		mov eax, [ecx + 0x5C]
		mov ecx, [ecx + 0x1C]
		mov[eax], ecx
		mov eax, 0x5AC020
		call eax
		mov ecx, [ebp - 0x2C]
		mov ecx, [ecx + 0x5C]
		mov[ecx + 0xC], eax
		jmp retnAddr
	}
}


void PatchMCM()
{
	WriteRelJump(0x60D841, (UInt32)ResetQuestHook);
}

bool __cdecl HookCmd_Expression_Evaluate(UInt32 numParams, ParamInfo* paramInfo, ScriptLineBuffer* lineBuf, ScriptBuffer* scriptBuf)
{
	InjectScriptBufVariables(scriptBuf);
	ExpressionParser parser(scriptBuf, lineBuf);
	return parser.ParseArgs(paramInfo, numParams);
}

bool PatchCmd_Expression_Evaluate()
{
	const auto candidates = scan_memory("nvse_1_4.dll", "This command cannot be called from the console.");

	if (candidates.size() != 1)
	{
		PrintLog("Unable to find Cmd_Expression_Parse in NVSE, aborting...");
		return false;
	}

	const void* stringAddress = candidates.at(0);

	const auto patchCandidates = scan_memory_pointer("nvse_1_4.dll", stringAddress);

	if (patchCandidates.size() != 1)
	{
		PrintLog("Unable to find referenced Cmd_Expression_Parse string in NVSE, aborting...");
		return false;
	}

	const auto* patchAddress = patchCandidates.at(0);

	PrintLog("address: %p", patchCandidates.at(0));
	
	const BYTE* addressVerify = static_cast<const BYTE*>(patchAddress) - 0x1;

	const static UInt8 OPCODE_PUSH = 0x68;

	if (*addressVerify != OPCODE_PUSH)
	{
		PrintLog("Unable to hook Cmd_Expression_Parse, expected opcode %X but got %X", OPCODE_PUSH, *addressVerify);
		return false;
	}

	//const static UInt8 OPCODE_RET = 0xC3;
	const static UInt8 OPCODE_INT3 = 0xCC;
	
	/*auto* addressToPatch = const_cast<BYTE*>(addressVerify);
	auto numInt3s = 0;
	while (numInt3s < 5) // reach head of function
	{
		if (*addressToPatch == OPCODE_INT3)
		{
			numInt3s++;
		}
		else
		{
			numInt3s = 0;
		}
		addressToPatch--;
	}*/
	
	WriteRelJump(UInt32(addressVerify), UInt32(HookCmd_Expression_Evaluate));

	return true;
}

bool PatchAr_Iter()
{
	auto candidates = scan_memory("nvse_1_4.dll", "ar_First/Last must be called within an OBSE expression.");
	// 15B4C53F
	if (candidates.size() != 1) {
		PrintLog("Unable to hook Ar_Iter");
		return false;
	}
	const void* stringAddress = candidates.at(0);

	auto patchAddresses = scan_memory_pointer("nvse_1_4.dll", stringAddress);

	if (patchAddresses.size() != 1) {
		PrintLog("Unable to hook Ar_Iter");
		return false;
	}

	PrintLog("address: %p", patchAddresses.at(0));
	const BYTE* addressToPatch = static_cast<const BYTE*>(patchAddresses.at(0)) - 0x3;

	PrintLog("string opcode: %X", *addressToPatch);

	if (*addressToPatch != 0x75) {
		PrintLog("Unable to hook Ar_Iter");
		return false;
	}

	const static UInt8 OPCODE_JMP = 0xEB;
	SafeWrite8((UInt32)addressToPatch, OPCODE_JMP);
	return true;
}

bool PatchAr_Find()
{
	auto candidates = scan_memory("nvse_1_4.dll", "ar_Find must be called within an OBSE expression.");
	// 15B4C53F
	if (candidates.size() != 1) {
		PrintLog("Unable to hook Ar_Find");
		return false;
	}
	const void* stringAddress = candidates.at(0);

	auto patchAddresses = scan_memory_pointer("nvse_1_4.dll", stringAddress);

	if (patchAddresses.size() != 1) {
		PrintLog("Unable to hook Ar_Find");
		return false;
	}

	PrintLog("address: %p", patchAddresses.at(0));
	const BYTE* addressToPatch = static_cast<const BYTE*>(patchAddresses.at(0)) - 0x3;

	PrintLog("string opcode: %X", *addressToPatch);

	if (*addressToPatch != 0x75) {
		PrintLog("Unable to hook Ar_Find");
		return false;
	}

	const static UInt8 OPCODE_JMP = 0xEB;
	SafeWrite8((UInt32)addressToPatch, OPCODE_JMP);
	return true;
}

bool PatchNVSEStrings() {
	auto candidates = scan_memory("nvse_1_4.dll", "Function must be used within a Set statement or NVSE expression");
	// 15B4C53F
	if (candidates.size() != 1) {
		PrintLog("Unable to hook NVSE");
		return false;
	}
	const void* stringAddress = candidates.at(0);

	auto patchAddresses = scan_memory_pointer("nvse_1_4.dll", stringAddress);

	if (patchAddresses.size() != 1) {
		PrintLog("Unable to hook NVSE");
		return false;
	}

	PrintLog("address: %p", patchAddresses.at(0));
	const BYTE* addressToPatch = static_cast<const BYTE*>(patchAddresses.at(0)) - 0x3;

	PrintLog("string opcode: %X", *addressToPatch);

	if (*addressToPatch != 0x75) {
		PrintLog("Unable to hook NVSE");
		return false;
	}

	const static UInt8 OPCODE_JMP = 0xEB;
	SafeWrite8((UInt32)addressToPatch, OPCODE_JMP);
	return true;
}

bool PatchNVSEAsserts() {
	auto candidates = scan_memory("nvse_1_4.dll", "_ReturnAddress() == kOpHandlerRetnAddr");

	if (candidates.size() != 1) {
		PrintLog("Unable to hook NVSE; too many/no candidates for assert strings (%d)", candidates.size());
		return false;
	}

	auto stringAddress = candidates.at(0);

	auto patchAddresses = scan_memory_pointer("nvse_1_4.dll", stringAddress);

	if (patchAddresses.size() != 2) {
		PrintLog("Unable to hook NVSE; too many/no addresses that reference assert string (%d)", patchAddresses.size());
		return false;
	}

	for (auto patchAddress : patchAddresses) {
		PrintLog("address: %p", patchAddress);

		const BYTE* addressToPatch = static_cast<const BYTE*>(patchAddress) - 0x3;
		PrintLog("assert opcode: %X", *addressToPatch);

		if (*addressToPatch != 0x74) {
			PrintLog("Unable to hook NVSE");
			return false;
		}

		const static UInt8 OPCODE_JMP = 0xEB;
		SafeWrite8((UInt32)addressToPatch, OPCODE_JMP);
	}
	return true;
}

bool PatchNVSE() {
	std::ifstream ifile;
	ifile.open("nvse_1_4.dll", std::ios::binary);
	if (!ifile.good()) {
		PrintLog("Could not find nvse_1_4.dll");
		ifile.close();
		return false;
	}
	ifile.close();

	if (!PatchNVSEStrings()) return false;
	if (!PatchNVSEAsserts()) return false;
	if (!PatchCmd_Expression_Evaluate()) return false;
	if (!PatchAr_Find()) return false;
	if (!PatchAr_Iter()) return false;

	PrintLog("NVSE Patched");
	// 15D661B8
	return true;
}

void PatchStupidBug() {
	//PatchMemoryNop(0x467A45, 7);
	WriteRelJump(0x467A15, 0x467A4E);
}