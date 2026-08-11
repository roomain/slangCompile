#include "slang_info.h"
#include <iostream>

std::string to_string(const slang::DeclReflection::Kind a_ReflectKind)
{
    switch (a_ReflectKind)
    {
    case slang::DeclReflection::Kind::Unsupported:
        return "Unsupported";
    case slang::DeclReflection::Kind::Struct:
        return "Struct";
    case slang::DeclReflection::Kind::Func:
        return "Func";
    case slang::DeclReflection::Kind::Module:
        return "Module";
    case slang::DeclReflection::Kind::Generic:
        return "Generic";
    case slang::DeclReflection::Kind::Variable:
        return "Variable";
    case slang::DeclReflection::Kind::Namespace:
        return "Namespace";
    case slang::DeclReflection::Kind::Enum:
        return "Enum";
    }
    return "";
}

void indent(const int a_level)
{
    for (int i = 0; i < a_level; ++i)
        std::cout << "--";
    std::cout << ">";
}

void displayReflectNode(const int level, slang::DeclReflection* a_ReflectNode)
{
    indent(level);
    std::cout << a_ReflectNode->getName() << " " << to_string(a_ReflectNode->getKind()) << "\n";
    for (auto node : a_ReflectNode->getChildren())
        displayReflectNode(level + 1, node);
}


void displayReflection(slang::IModule* a_slangModule)
{
    if(slang::DeclReflection* moduleReflection = a_slangModule->getModuleReflection())
        displayReflectNode(0, moduleReflection);
}