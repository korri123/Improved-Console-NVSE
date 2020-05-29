#include "autocomplete.h"


#include "GameAPI.h"
#include "GameTiles.h"
#include "GameUI.h"

namespace Autocomplete
{
	class SearchTrie
	{
		struct TrieEntry
		{
			std::string text;
			int priority;
		};

	public:
		void Insert(std::string text, int priority)
		{
			
		}
	};
	
	Tile* g_autocompleteText;
	SearchTrie g_searchTrie;
	
	static const int X_OFFSET = 10;
	static const int Y_OFFSET = -14;
	
	void Initialize()
	{
		g_autocompleteText = InterfaceManager::GetMenuComponentTile("HUDMainMenu\\AutocompleteText");
		if (g_autocompleteText == nullptr)
		{
			PrintLog(R"(Could not initialize Autocomplete! Check if UIO is installed and if \menus\prefabs\AutoComplete\AutoComplete.txt exists.)");
			return;
		}


		const auto consoleTextPosX = *reinterpret_cast<UInt32*>(0x11D8D44 + 4);
		const auto consoleTextPosY = *reinterpret_cast<UInt32*>(0x11D8D08 + 4);
		const auto consoleFontId = *reinterpret_cast<UInt32*>(0x11D8D38 + 4);

		PrintLog("X: %d Y: %d Font: %d", consoleTextPosX, consoleTextPosY, consoleFontId);
		
		g_autocompleteText->SetFloat(kTileValue_x, consoleTextPosX + X_OFFSET);
		g_autocompleteText->SetFloat(kTileValue_y, consoleTextPosY + Y_OFFSET);
		g_autocompleteText->SetFloat(kTileValue_font, consoleFontId);

		PrintLog("Initialized Autocomplete");
	}

}
