#include "dinput.h"

namespace ImprovedConsole
{

	__declspec(naked) bool DIHookControl::IsKeyPressed(UInt32 keycode, UInt8 flags)
	{
		__asm
		{
			mov		eax, [esp + 4]
			cmp		eax, kMaxMacros
			jb		proceed
			xor		al, al
			retn	8
		proceed:
			mov		dl, [esp + 8]
			test	dl, dl
			jnz		doneFlag
			mov		dl, 1
		doneFlag :
			 lea		ecx, [ecx + eax * 8 + 4]
			 sub		ecx, eax
			 mov		al, [ecx + 2]
			 shl		al, 1
			 or		al, [ecx]
			 shl		al, 1
			 or		al, [ecx + 1]
			 test	al, dl
			 setnz	al
			 retn	8
		}
	}

	bool DIHookControl::IsKeyPressedRaw(UInt32 keycode)
	{
		return (keycode < kMaxMacros) && m_keys[keycode].rawState;
	}

	bool DIHookControl::IsLMBPressed()
	{
		return m_keys[0x100].rawState;
	}

	bool DIHookControl::IsKeyDisabled(UInt32 keycode)
	{
		return (keycode < kMaxMacros) && (m_keys[keycode].userDisable || m_keys[keycode].scriptDisable);
	}

	bool DIHookControl::IsKeyHeld(UInt32 keycode)
	{
		return (keycode < kMaxMacros) && m_keys[keycode].hold;
	}

	bool DIHookControl::IsKeyTapped(UInt32 keycode)
	{
		return (keycode < kMaxMacros) && m_keys[keycode].tap;
	}

	void DIHookControl::SetKeyDisableState(UInt32 keycode, bool bDisable, UInt8 mask)
	{
		if (!mask) mask = kDisable_All;
		if (keycode < kMaxMacros)
		{
			KeyInfo *info = &m_keys[keycode];
			if (mask & kDisable_User)
				info->userDisable = bDisable;
			if (mask & kDisable_Script)
				info->scriptDisable = bDisable;
		}
	}

	void DIHookControl::SetLMBDisabled(bool bDisable)
	{
		m_keys[0x100].userDisable = bDisable;
	}

	void DIHookControl::SetKeyHeldState(UInt32 keycode, bool bHold)
	{
		if (keycode < kMaxMacros)
			m_keys[keycode].hold = bHold;
	}

	void DIHookControl::TapKey(UInt32 keycode)
	{
		if (keycode < kMaxMacros)
			m_keys[keycode].tap = true;
	}
	
}