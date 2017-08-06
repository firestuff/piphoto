#pragma once

#include <string>

std::string ReadFile(const std::string& filename);
void WriteFile(const std::string& filename, const std::string& contents);
