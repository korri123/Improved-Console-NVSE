#include "structs.h"

GameInterfaceManager* GameInterfaceManager::GetSingleton(void)
{
	return *reinterpret_cast<GameInterfaceManager**>(0x11D8A80);
}
