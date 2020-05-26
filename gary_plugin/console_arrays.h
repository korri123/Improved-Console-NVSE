#pragma once
#include <vector>

#include "PluginAPI.h"

namespace ConsoleArrays
{
	struct ConsoleArray
	{
		std::vector<NVSEArrayVarInterface::Element> keys;
		std::vector<NVSEArrayVarInterface::Element> values;
	};
	
	enum ElementType
	{
		kDataType_Invalid,
		kDataType_Numeric,
		kDataType_Form,
		kDataType_String,
		kDataType_Array
	};

	enum ConsoleArrayType
	{
		type_Array,
		type_StringMap,
		type_Map
	};

	//std::vector<ConsoleArray> g_consoleArrays;
	
	void StoreArray(std::string& variableName, const UInt32 arrayId);

	UInt32 InjectConsoleArray(std::string& arrayVarName);

	NVSEArrayElement GetElementAtIndex(std::string& varName, std::string& index);
}

