#pragma once
#include <string>
#include <vector>

std::vector<const void*> scan_memory(void* address_low, std::size_t nbytes, const std::vector<BYTE>& bytes_to_find);

std::vector<const void*> scan_memory(std::string module_name, const std::vector<BYTE>& bytes_to_find);

std::vector<const void*> scan_memory(std::string module_name, const char* string);

std::vector<const void*> scan_memory_pointer(std::string module_name, const void* ptr);