#pragma once

#include "console_arrays.h"
#include <memory>
#include <stdexcept>
#include <string>
#include "declarations.h"

using namespace ImprovedConsole;

namespace ConsoleArrays
{
	std::map<std::string, std::shared_ptr<ConsoleArray>> g_consoleArraysMap;

	CommandReturnType GetType(const ElementType type)
	{
		switch (type)
		{
		case kDataType_Invalid: return kRetnType_Ambiguous;
		case kDataType_Numeric: return kRetnType_Default;
		case kDataType_Form: return kRetnType_Form;
		case kDataType_String: return kRetnType_String;
		case kDataType_Array: return kRetnType_Array;
		default: return kRetnType_Ambiguous;
		}
	}

	ConsoleArray& GetByName(std::string name)
	{
		const auto varEntry = g_consoleArraysMap.find(name);
		if (varEntry == g_consoleArraysMap.end())
		{
			throw std::invalid_argument("couldn't find console array");
		}

		return *varEntry->second;
	}

	void StoreArray(std::string& variableName, const UInt32 arrayId)
	{
		auto* const arrayPtr = LookupArrayByID(arrayId);

		if (arrayPtr == nullptr)
		{
			Console_Print("Cannot set undefined array");
			return;
		}

		const int size = GetArraySize(arrayPtr);

		std::vector<NVSEArrayVarInterface::Element> keys(size);
		std::vector<NVSEArrayVarInterface::Element> values(size);
		GetElements(arrayPtr, &values[0], &keys[0]);

		const auto varPtr = std::make_shared<ConsoleArray>(ConsoleArray{
			keys,
			values
		});

		g_consoleArraysMap[variableName] = varPtr;
	}

	ConsoleArrayType GetArrayType(ConsoleArray& consoleArray)
	{
		try
		{
			const auto& key = consoleArray.keys.at(0);
			if (key.GetType() == kDataType_String)
			{
				return type_StringMap;
			}
			return type_Array;
		}
		catch (std::out_of_range&)
		{
			return type_Array;
		}
	}

	NVSEArrayElement GetElementAtIndex(std::string& varName, std::string& index)
	{
		ConsoleArray consoleArray;
		try
		{
			consoleArray = GetByName(varName);
		}
		catch (std::invalid_argument&)
		{
			throw std::invalid_argument("array name not found");
		}
		
		try
		{
			const auto numericalIndex = std::stoi(index);
			if (GetArrayType(consoleArray) != ConsoleArrays::type_Array)
			{
				throw std::invalid_argument("not an array");
			}
			return consoleArray.values.at(numericalIndex);
		}
		catch (std::invalid_argument&)
		{
			if (GetArrayType(consoleArray) != ConsoleArrays::type_StringMap)
			{
				throw std::invalid_argument("array not string map");
			}

			auto count = 0;
			for (auto& key : consoleArray.keys)
			{
				if (_stricmp(key.String(), index.c_str()) == 0)
				{
					return consoleArray.values.at(count);
				}
				count++;
			}
			throw std::invalid_argument("could not find value with index");
		}
		catch (std::out_of_range&)
		{
			throw std::invalid_argument("index out of range");
		}
	}

	UInt32 InjectArray(ConsoleArray& arrayEntry)
	{
		auto* arrayPtr = CreateArray(&arrayEntry.values[0], arrayEntry.values.size(), g_consoleScript);

		double result = 0;
		AssignCommandResult(arrayPtr, &result);
		PrintLog("Array packed: %d", g_arrayInterface->GetArrayPacked(arrayPtr));
		return result;
	}

	UInt32 InjectStringMap(ConsoleArray& arrayEntry)
	{
		auto keys = std::vector<const char*>();
		for (auto i = 0; i < arrayEntry.keys.size(); ++i)
		{
			keys.insert(keys.begin() + i, arrayEntry.keys.at(i).String());
		}

		auto* arrayPtr = CreateStringMap(&keys[0], &arrayEntry.values[0], arrayEntry.keys.size(), g_consoleScript);

		double result = 0;
		AssignCommandResult(arrayPtr, &result);
		return result;
	}

	UInt32 InjectMap(ConsoleArray& arrayEntry)
	{
		auto keys = std::vector<double>();
		for (unsigned int i = 0; i < keys.size(); ++i)
		{
			keys.push_back(arrayEntry.keys.at(i).Number());
		}

		auto* arrayPtr = CreateMap(&keys[0], &arrayEntry.values[0], arrayEntry.keys.size(), g_consoleScript);

		double result = 0;
		AssignCommandResult(arrayPtr, &result);
		return result;
	}

	UInt32 InjectConsoleArray(std::string& arrayVarName)
	{
		const auto arrayVarEntry = g_consoleArraysMap.find(arrayVarName);
		if (arrayVarEntry == g_consoleArraysMap.end())
		{
			throw std::invalid_argument("array does not exist");
		}
		const auto arrayVar = arrayVarEntry->second;
		switch (GetArrayType(*arrayVar))
		{
		case type_Array:
			return InjectArray(*arrayVar);
		case type_StringMap:
			return InjectStringMap(*arrayVar);
		case type_Map:
			return InjectMap(*arrayVar);
		}
		throw std::invalid_argument("error");
	}
}

