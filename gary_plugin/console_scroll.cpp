#include "console_scroll.h"

#include "declarations.h"
#include "GameAPI.h"

using namespace ImprovedConsole;

//static const int KEYCODE_L_SHIFT = 0x2A;
//static const int KEYCODE_R_SHIFT = 0x36;
static const int KEYCODE_WHEEL_UP = 0x108;
static const int KEYCODE_WHEEL_DOWN = 0x109;


void CheckForScrollWheelCallback()
{
	if (!IsConsoleOpen())
	{
		return;
	}

	auto* consoleManager = ConsoleManager::GetSingleton();
	//PrintLog("Scrolled");
	if (g_DIHookCtrl->IsKeyPressed(KEYCODE_WHEEL_UP)) {
		ThisStdCall(0x71D380, consoleManager, -1); // Scroll Console
		ThisStdCall(0x71D410, consoleManager); // RefreshConsoleOutput
	}
	else if (g_DIHookCtrl->IsKeyPressed(KEYCODE_WHEEL_DOWN)) {
		ThisStdCall(0x71D380, consoleManager, 1); // Scroll Console
		ThisStdCall(0x71D410, consoleManager); // RefreshConsoleOutput
	}
}
