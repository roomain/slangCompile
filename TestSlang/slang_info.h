#pragma once
#include <string>
#include <slang.h>

std::string to_string(const slang::DeclReflection::Kind a_ReflectKind);
void displayReflection(slang::IModule* a_slangModule);