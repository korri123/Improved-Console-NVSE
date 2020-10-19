#pragma once
#include "SafeWrite.h"

static UInt32 ScriptIsAlpha_PatchAddr = 0xEC8A10; // Same starting proc as tokenAlias_Float. Follow failed test for '"', enter first call. Replace call.
static UInt32 IsAlphaCallAddr = 0xEC8993;
static UInt32 ScriptCharTableAddr = 0x10F22D0;		// Table classyfing character for scripts. Replace IsAlpha

static void __declspec(naked) ScriptIsAlphaHook(void)
{
	__asm
	{
			mov al, byte ptr[esp + 4]	// arg to isalpha()

			cmp al, '['					// brackets used for array indexing
			jz AcceptChar
			cmp al, ']'
			jz AcceptChar
			cmp al, '$'					// $ used as prefix for string variables
			jz AcceptChar

			jmp IsAlphaCallAddr			// else use default behavior

		AcceptChar :
			mov eax, 1
			retn
	}
}

void PatchIsAlpha()
{
	WriteRelCall(ScriptIsAlpha_PatchAddr, (UInt32)ScriptIsAlphaHook);
	SafeWrite8(ScriptCharTableAddr + '[' * 2, 2);
	SafeWrite8(ScriptCharTableAddr + ']' * 2, 2);
	SafeWrite8(ScriptCharTableAddr + '$' * 2, 2);
}

const auto* g_arrayVar = "array_var";
const auto* g_stringVar = "string_var";

void PatchTokenTypeDefs()
{
	const auto tokenAliasFloat = 0x118CBF4;
	const auto tokenAliasLong = 0x118CBCC;
	SafeWrite32(tokenAliasFloat, UInt32(g_arrayVar));
	SafeWrite32(tokenAliasLong, UInt32(g_stringVar));
}

