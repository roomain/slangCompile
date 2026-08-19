#pragma once
#include <string>
#include <vector>

bool compileSlang(const std::string& a_Filename, std::vector<char>& outData);
std::string generateSpirvFilename(const std::string& a_Filename);