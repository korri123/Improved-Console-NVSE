#include "nvse_hooks.h"
#include "GameAPI.h"
#include "SafeWrite.h"

bool g_rigIsAlpha = false;

static void __declspec(naked) ScriptIsAlphaNumHook(void)
{
	static auto defaultBehavior = 0xEC89EE;
	__asm
	{
		push ebp
		mov  ebp, esp
		mov al, g_rigIsAlpha
		test al, al
		jz DefaultBehavior
		mov al, [ebp + 0x8]
		cmp al, '['
		jz AcceptChar
		cmp al, ']'
		jz AcceptChar
		cmp al, '$'
		jz AcceptChar
	DefaultBehavior:
		jmp defaultBehavior
	AcceptChar:
		mov eax, 1
		pop ebp
		ret
	}
}

const auto* g_arrayVar = "array_var";
const auto* g_stringVar = "string_var";

void PatchIsAlpha()
{
	WriteRelJump(0xEC89E9, UInt32(ScriptIsAlphaNumHook));
}

void PatchTokenTypeDefs()
{
	const auto tokenAliasFloat = 0x118CBF4;
	const auto tokenAliasLong = 0x118CBCC;
	SafeWrite32(tokenAliasFloat, UInt32(g_arrayVar));
	SafeWrite32(tokenAliasLong, UInt32(g_stringVar));
}
