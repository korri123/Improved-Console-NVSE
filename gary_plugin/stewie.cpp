#include "nvse/nvse/PluginAPI.h"
#include "common/IDebugLog.h"
#include "nvse/nvse/nvse_version.h"
#include "nvse/nvse/Hooks_DirectInput8Create.h"
#include "nvse/nvse/SafeWrite.h"
#include "nvse/nvse/CommandTable.h"
#include "nvse/nvse/Utilities.h"
#include "nvse/GameUI.h"
#include <windows.h>
#include <fstream>
#include <GameObjects.h>
#include "stewie.h"

#define REFID_FILENAME ".\\ConsoleLog.txt"

int g_bReplaceNewLineWithEnter = 0; // 0 -> replace with space
static const int MAX_BUFFER_SIZE = 255;

#define ClearLines(consoleManager) ThisStdCall(0x71E070, consoleManager+1);
void(__thiscall *RefreshConsoleOutput)(_ConsoleManager* consoleManager) = (void(__thiscall*)(_ConsoleManager*))0x71D410;

static char *clipboardText = (char*)malloc(MAX_BUFFER_SIZE);
const NVSEInterface* g_nvse;

void handleIniOptions() {
	char filename[MAX_PATH];
	GetModuleFileNameA(consolePasteHandle, filename, MAX_PATH);
	strcpy((char *)(strrchr(filename, '\\') + 1), "nvse_console_clipboard.ini");
	g_bReplaceNewLineWithEnter = GetPrivateProfileIntA("Main", "bReplaceNewLineWithEnter", 0, filename);
}

void patchOnConsoleInput() {
	WriteRelCall(0x70E09E, (UInt32)CheckHotkeys);

	WriteRelJump(0x71CDDF, (UInt32)hookInputLenCheck);
}

__declspec(naked) void hookInputLenCheck() {
	static const UInt32 retnAddr = 0x71CDE5;
	static const UInt32 skipAddr = 0x71CE53;
	__asm {
		cmp eax, 255 // eax contains string length
			jg ignoreInput

			mov eax, [ebp - 0x820]
			jmp retnAddr

		ignoreInput :
		jmp skipAddr
	}
}

bool _declspec(naked) IsControlHeld()
{
	_asm
	{
		mov     eax, g_DIHookCtrl
			cmp     byte ptr[eax + 0xCF], 0    // check left control
			jnz     done
			cmp     byte ptr[eax + 0x44F], 0    // check right control
		done:
		setne al
			ret
	}
}

bool _declspec(naked) IsAltHeld()
{
	_asm
	{
		mov     eax, g_DIHookCtrl
			cmp     byte ptr[eax + 0x18C], 0    // check left alt (DirectX scancode * 7 + 4)
			setne al
			ret
	}
}

void WriteSelectedRefIDToFile(_ConsoleManager* mgr)
{
	char refIDBuf[512];
	char consoleInputBuf[512];

	strcpy(consoleInputBuf, getConsoleInputString());
	removeChar(consoleInputBuf, '|');

	TESObjectREFR* selectedRef = InterfaceManager::GetSingleton()->debugSelection;

	if (strlen(consoleInputBuf))
	{
		if (selectedRef)
			sprintf(refIDBuf, "[%08x/%s]: %s", selectedRef->refID, selectedRef->baseForm ? selectedRef->baseForm->GetName() : selectedRef->GetName(), consoleInputBuf);
		else
			sprintf(refIDBuf, "[No Selection]: %s", consoleInputBuf);

		std::ofstream file(REFID_FILENAME, std::ofstream::app);
		file << refIDBuf << std::endl;

		ClearInputString(mgr);

		// refresh the console input
		mgr->HandleInput(kSpclChar_RightArrow);
	}
}

// returns true if the key should not be passed to other menus (i.e. it was handled by the console)
bool __fastcall CheckHotkeys(_ConsoleManager* mgr, void* edx, int key)
{
	if (mgr->isConsoleOpen <= 0) return false;

	if (key == VK_TAB)
	{
		if (IsAltHeld())
		{
			return true;
		}
		else
		{
			CheckCommandValid(mgr);
		}
	}
	else if (!IsControlHeld())
	{
		return mgr->HandleInput(key);
	}
	else
	{
		if (key == kSpclChar_Backspace || key == kSpclChar_Delete || key == kSpclChar_LeftArrow || key == kSpclChar_RightArrow)
		{
			HandleWord(mgr, key);
		}
		else
		{
			switch (key | 0x20)
			{
			case 'v':
				PrintClipBoardToConsoleInput(mgr);
				break;

			case 'l':
				ClearConsoleOutput(mgr);
				break;

			case 'c':
				CopyInputToClipboard(mgr);
				break;

			case 'x':
				ClearInputString(mgr);
				break;

			case 'r':
				WriteSelectedRefIDToFile(mgr);
				break;
			}
		}
	}
	return true;
}

void PrintClipBoardToConsoleInput(_ConsoleManager *mgr) {
	GetClipboardText(&clipboardText);
	for (int i = 0, c = clipboardText[0]; c != '\0' && i < MAX_BUFFER_SIZE; i++) {
		c = clipboardText[i];
		switch (c) {
			/* replace newlines with spaces */
		case '\n':
			if (g_bReplaceNewLineWithEnter) mgr->HandleInput(kSpclChar_Enter);
			else mgr->HandleInput(' ');
			break;

			/* remove pipe characters */
		case '\r':
		case '|':
			break;

		default:
			mgr->HandleInput(clipboardText[i]);
		}
	}
}

void GetClipboardText(char **buffer) {
	/* Try opening the clipboard */
	if (!OpenClipboard(NULL)) {
		// clipboard empty so return empty string
		*buffer[0] = '\0';
		return;
	}

	/* Get handle of clipboard object for ANSI text */
	HANDLE hData = GetClipboardData(CF_TEXT);
	if (hData == NULL) {
		CloseClipboard();
		// clipboard empty so return empty string
		*buffer[0] = '\0';
		return;
	}

	/* Lock the handle to get the actual text pointer */
	char *pszText = static_cast<char *>(GlobalLock(hData));
	if (pszText == NULL) {
		GlobalUnlock(hData);
		CloseClipboard();
		// clipboard empty so return empty string
		*buffer[0] = '\0';
		return;
	}

	/* copy clipboard text into buffer */
	strncpy(*buffer, pszText, MAX_BUFFER_SIZE);

	/* Release the lock and clipboard */
	GlobalUnlock(hData);
	CloseClipboard();
}

char *getConsoleInputString() {
	String* str = GetDebugInput();
	return str ? str->m_data : NULL;
}

int getCharsSinceSpace(char *text) {
	char *barPos = strchr(text, '|');
	int numChars = 0;

	while (barPos != text && !isalnum(*--barPos)) numChars++;
	while (barPos >= text && isalnum(*barPos--)) numChars++;

	return numChars;
}

int getCharsTillSpace(char *text) {
	char *barPos = strchr(text, '|');
	if (!barPos) return 0;

	int numChars = 0;

	while (isalnum(*++barPos)) numChars++;
	while (*barPos && !isalnum(*barPos++)) numChars++;

	return numChars;
}

void HandleWord(_ConsoleManager *mgr, UInt32 key) {
	char *consoleInput = getConsoleInputString();
	if (!consoleInput || !(*consoleInput)) return;

	int timesToRepeat = 0;
	if (key == kSpclChar_Backspace || key == kSpclChar_LeftArrow)
		timesToRepeat = getCharsSinceSpace(consoleInput);
	else
		timesToRepeat = getCharsTillSpace(consoleInput);

	for (; timesToRepeat > 0; timesToRepeat--) {
		mgr->HandleInput(key);
	}
}

void ClearInputString(_ConsoleManager *mgr) {
	String* input = GetDebugInput();
	if (!input) return;
	input->Set("");
	mgr->HandleInput(kSpclChar_RightArrow); //any control character would work here
}

void CopyInputToClipboard(_ConsoleManager* consoleMgr) {
	char* inputString = getConsoleInputString();
	int len = strlen(inputString) + 1;
	char* sanitisedInput = (char*)malloc(len);

	strcpy(sanitisedInput, inputString);
	removeChar(sanitisedInput, '|');

	int sanitisedLen = strlen(sanitisedInput) + 1;

	HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, sanitisedLen);
	if (!hg) {
		return;
	}

	memcpy(GlobalLock(hg), sanitisedInput, sanitisedLen);
	GlobalUnlock(hg);
	OpenClipboard(NULL);
	EmptyClipboard();
	SetClipboardData(CF_TEXT, hg);
	CloseClipboard();

	free(sanitisedInput);
}

void ClearConsoleOutput(_ConsoleManager* mgr) {
	mgr->printedLines.FreeAll();
	mgr->printedCount = 0;
	RefreshConsoleOutput(mgr); // refreshes the output so changes show
}

void trim(char * s) {
	char * p = s;
	int l = strlen(p);

	while (isspace(p[l - 1])) p[--l] = 0;
	while (*p && isspace(*p)) ++p, --l;

	memmove(s, p, l + 1);
}

void customHelp(char* searchString) {
	nopFunctionCall(0x5BCBBA); //----SCRIPT FUNCTIONS print
	nopFunctionCall(0x5BC9B9); // ----CONSOLE COMMANDS print
	WriteRelJump(0x5BCDBF, 0x5BCFC3); // skip over ini settings

	char* tempBuffer = (char*)malloc(strlen(searchString) + 6);
	strcpy(tempBuffer, "help ");
	strcat(tempBuffer, searchString);

	Console_Print("---");
	g_con->RunScriptLine(tempBuffer, NULL);

	/* restore original functionality */
	SafeWriteBuf(0x5BCBBA, "\xE8\x41\x70\x14\x00", 5);
	SafeWriteBuf(0x5BC9B9, "\xE8\x42\x72\x14\x00", 5);
	WriteRelJnz(0x5BCDBF, 0x5BCFC3);

	free(tempBuffer);
}

void CheckCommandValid(_ConsoleManager *consoleMgr) {
	char *buffer = getConsoleInputString();
	if (!buffer) return;

	char* sanitisedInput = (char*)malloc(strlen(buffer) + 1);
	strcpy(sanitisedInput, buffer);

	/* remove leading and trailing whitespace */
	trim(sanitisedInput);
	if (strchr(sanitisedInput, ' ')) {
		// if the input already contains a space, we will have sent error by now
		free(sanitisedInput);
		return;
	}

	removeChar(sanitisedInput, '|');
	if (!*sanitisedInput) return;

	const CommandInfo* command;

	char* tempptr = strchr(sanitisedInput, '.');
	if (tempptr) {
		customHelp(tempptr + 1);
	}
	else {
		customHelp(sanitisedInput);
	}

	if (GetIsLeftControlPressed()) {
		TextNode* last = consoleMgr->printedLines.last;
		if (last) {
			TextNode* prev = last->prev;
			if (prev) {
				if (isHelpSeparator(prev->text.m_data)) {
					char* firstWord = (char*)malloc(strlen(last->text.m_data) + 1);
					getFirstWord(firstWord, last->text.m_data);

					GetDebugInput()->Set(firstWord);
					consoleMgr->HandleInput(kSpclChar_RightArrow); //refresh the input buffer

					free(firstWord);
				}
			}
		}
	}

	free(sanitisedInput);
}

void InitStewie(NVSEInterface* nvse) {
	g_nvse = nvse;

	NVSEDataInterface *nvseData = (NVSEDataInterface *)nvse->QueryInterface(kInterface_Data);
	g_DIHookCtrl = (DIHookControl *)nvseData->GetSingleton(NVSEDataInterface::kNVSEData_DIHookControl);
	cmdTable = (NVSECommandTableInterface*)nvse->QueryInterface(kInterface_CommandTable);
	g_con = (NVSEConsoleInterface*)nvse->QueryInterface(kInterface_Console);

	patchOnConsoleInput();
}