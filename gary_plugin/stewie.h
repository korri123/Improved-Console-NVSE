#pragma once
#include "GameAPI.h"
#include "CommandTable.h"
#include "PluginAPI.h"

#define GameHeapAlloc(size) ThisStdCall(0xAA3E40, (void*)0x11F6238, size)
#define GameHeapFree(ptr) ThisStdCall(0xAA4060, (void*)0x11F6238, ptr)

NVSECommandTableInterface *cmdTable;
NVSEConsoleInterface *g_con = NULL;
DIHookControl *g_DIHookCtrl;
HMODULE consolePasteHandle;

enum
{
	kSpclChar_Backspace = 0x80000000,
	kSpclChar_LeftArrow = 0x80000001,
	kSpclChar_RightArrow = 0x80000002,
	kSpclChar_Delete = 0x80000007,
	kSpclChar_Enter = 0x80000008,
	kSpclChar_PageUp = 0x80000009,
	kSpclChar_PageDown = 0x8000000A
};


struct _String
{
public:
	char* m_data;
	UInt16		m_dataLen;
	UInt16		m_bufLen;

	bool Set(const char* src);
};

struct TextNode
{
	TextNode	*next;
	TextNode	*prev;
	_String		text;
};

struct TextList
{
	TextNode	*first;
	TextNode	*last;
	UInt32		count;

	void FreeAll() { ThisStdCall(0x71E070, this); };
};

struct RecordedCommand
{
	char buf[100];
};

class _ConsoleManager
{
public:
	void* scriptContext;
	TextList printedLines;
	TextList inputHistory;
	UInt32 historyIndex;
	UInt32 unk020;
	UInt32 printedCount;
	UInt32 unk028;
	UInt32 lineHeight;
	int textXPos;
	int textYPos;
	UInt8 isConsoleOpen;
	UInt8 unk39;
	UInt8 isBatchRecording;
	UInt8 unk3B;
	UInt32 numRecordedCommands;
	RecordedCommand recordedCommands[20];
	char scofPath[260];

	bool HandleInput(int key) { return ThisStdCall(0x71B210, this, key); };
	void AppendToSentHistory(const char* str);
};

/* helper function prototypes */
void handleIniOptions();
void patchOnConsoleInput();
void GetClipboardText(char **buffer);
char *getConsoleInputString();

void hookInputLenCheck();
bool __fastcall CheckHotkeys(_ConsoleManager*, void*, int);

void PrintClipBoardToConsoleInput(_ConsoleManager* consoleMgr);
void ClearInputString(_ConsoleManager* consoleMgr);
void HandleWord(_ConsoleManager* consoleMgr, UInt32 inKey);
void ClearConsoleOutput(_ConsoleManager* consoleMgr);
void CopyInputToClipboard(_ConsoleManager* consoleMgr);
void CheckCommandValid(_ConsoleManager* consoleMgr);

class _DebugText
{
public:
	_DebugText();
	~_DebugText();

	virtual void    Unk_00(void);
	virtual void    Unk_01(UInt32 arg1, UInt32 arg2);
	virtual UInt32    Unk_02(UInt32 arg1, UInt32 arg2, UInt32 arg3, UInt32 arg4, UInt32 arg5, UInt32 arg6);
	virtual UInt32    Unk_03(UInt32 arg1, UInt32 arg2, UInt32 arg3, UInt32 arg4);
	virtual void    Unk_04(UInt32 arg1, UInt32 arg2, UInt32 arg3, UInt32 arg4, UInt32 arg5, UInt32 arg6);
	virtual UInt32    Unk_05(UInt32 arg1, UInt32 arg2, UInt32 arg3, UInt32 arg4, UInt32 arg5);
	virtual void    Unk_06(UInt32 arg1, UInt32 arg2, UInt32 arg3, UInt32 arg4, UInt32 arg5);
	virtual UInt32    Unk_07(UInt32 arg1, UInt32 arg2, UInt32 arg3, UInt32 arg4, UInt32 arg5, UInt32 arg6, UInt32 arg7);
	virtual UInt32    Unk_08(UInt32 arg1, UInt32 arg2, UInt32 arg3, UInt32 arg4, UInt32 arg5);
	virtual UInt32    Unk_09(UInt32 arg1, UInt32 arg2, UInt32 arg3, UInt32 arg4, UInt32 arg5, UInt32 arg6);
	virtual UInt32    Unk_0A(UInt32 arg1);
	virtual void    Unk_0B(UInt32 arg1, UInt32 arg2);

	struct DebugLine
	{
		float            offsetX;    // 00
		float            offsetY;    // 04
		UInt32            isVisible;    // 08
		NiNode            *node;        // 0C
		String            text;        // 10
		float            flt18;        // 18    Always -1.0
		NiColorAlpha    color;        // 1C
	};

	DebugLine        lines[200];        // 0004
	UInt32            unk2264[14];    // 2264

	static _DebugText *GetSingleton();
};
_DebugText *_DebugText::GetSingleton()
{
	return ((_DebugText* (*)(bool))0xA0D9E0)(true);
}

_DebugText::DebugLine *GetDebugInputLine()
{
	_DebugText *debugText = _DebugText::GetSingleton();
	if (!debugText) return NULL;
	_DebugText::DebugLine *linesPtr = debugText->lines;
	_DebugText::DebugLine *result = linesPtr;
	float maxY = linesPtr->offsetY;
	UInt32 counter = 200;
	do
	{
		linesPtr++;
		if (!linesPtr->text.m_data) break;
		if (maxY < linesPtr->offsetY)
		{
			maxY = linesPtr->offsetY;
			result = linesPtr;
		}
	} while (--counter);
	return result;
}

String* GetDebugInput() {
	return &(GetDebugInputLine()->text);
}


void* (__cdecl *_memcpy)(void *destination, const void *source, size_t num) = memcpy;

__declspec(naked) char* __fastcall StrCopy(char *dest, const char *src)
{
	__asm
	{
		push	ebx
			mov		eax, ecx
			test	ecx, ecx
			jz		done
			test	edx, edx
			jz		done
			xor		ebx, ebx
		getSize :
		cmp[edx + ebx], 0
			jz		doCopy
			inc		ebx
			jmp		getSize
		doCopy :
		push	ebx
			push	edx
			push	eax
			call	_memcpy
			add		esp, 0xC
			add		eax, ebx
			mov[eax], 0
		done:
		pop		ebx
			retn
	}
}

bool _String::Set(const char *src)
{
	if (!src || !*src)
	{
		if (m_data)
		{
			GameHeapFree(m_data);
			m_data = NULL;
		}
		m_dataLen = m_bufLen = 0;
		return true;
	}
	m_dataLen = strlen(src);
	if (m_bufLen < m_dataLen)
	{
		m_bufLen = m_dataLen;
		if (m_data) GameHeapFree(m_data);
		m_data = (char*)GameHeapAlloc(m_dataLen + 1);
	}
	StrCopy(m_data, src);
	return true;
}

void nopFunctionCall(UInt32 callAddress) {
	SafeWriteBuf(callAddress, "\x90\x90\x90\x90\x90", 5);
}

bool isHelpSeparator(char* string) {
	if (strlen(string) != 3) return false;
	for (int i = 0; i < 3; i++) {
		if (string[i] != '-') return false;
	}
	return true;
}

void getFirstWord(char* dst, char* src) {
	while (*src != ' ' && *src != '\0') {
		*dst = *src;
		dst++;
		src++;
	}
	*dst = '\0';
}

__declspec(naked) bool GetIsLeftControlPressed() {
	_asm {
		mov     eax, g_DIHookCtrl
			cmp     byte ptr[eax + 0xCF], 0
			setne al
			ret
	}
}

void removeChar(char* str, const char garbage) {
	char* src, *dst;
	for (src = dst = str; *src != '\0'; src++) {
		*dst = *src;
		if (*dst != garbage) dst++;
	}
	*dst = '\0';
}

bool versionCheck(const NVSEInterface* nvse) {
	if (nvse->isEditor) return false;
	if (nvse->runtimeVersion < RUNTIME_VERSION_1_4_0_525) {
		_ERROR("incorrect runtime version (got %08X need at least %08X)", nvse->runtimeVersion, RUNTIME_VERSION_1_4_0_525);
		return false;
	}
	return true;
}

void _ConsoleManager::AppendToSentHistory(const char* str)
{
	String* string = (String*)GameHeapAlloc(sizeof(String));
	string->Init(0);
	string->Set(str);
	ThisStdCall(0x71DF20, &this->inputHistory, string);
	GameHeapFree(string);
}

void String::Init(UInt32 bufSize)
{
	if (m_data) GameHeapFree(m_data);
	m_bufLen = bufSize;
	m_data = (char*)GameHeapAlloc(m_bufLen + 1);
	*m_data = 0;
	m_dataLen = 0;
}

void InitStewie(NVSEInterface* nvse);