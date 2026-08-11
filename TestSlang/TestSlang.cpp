// TestSlang.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#include <iostream>
#include <filesystem>
#include <string>
#include <array>

#include <slang.h>
#include <slang-com-ptr.h>

Slang::ComPtr<slang::IGlobalSession> g_globalSession;
Slang::ComPtr<slang::ISession> g_session;

struct SlangSpirVSession
{
    slang::TargetDesc target;
    slang::SessionDesc sessionDesc;
} g_sprivSession;

void createSlangSession()
{
    // global session
    if (SLANG_FAILED(slang::createGlobalSession(g_globalSession.writeRef())))
    {
        std::cerr << "Can't create slang global session.\n";
        return;
    }
    g_sprivSession.target.format = SLANG_SPIRV;
    g_sprivSession.target.profile = g_globalSession->findProfile("spirv_1_5");

    g_sprivSession.sessionDesc.targetCount = 1;
    g_sprivSession.sessionDesc.targets = &g_sprivSession.target;

    //g_sprivSession.sessionDesc.defaultMatrixLayoutMode = desc.defaultMatrixLayoutMode;
    //g_sprivSession.sessionDesc.searchPathCount = desc.searchPathCount;
    //g_sprivSession.sessionDesc.searchPaths = desc.searchPaths;
    g_globalSession->createSession(g_sprivSession.sessionDesc, g_session.writeRef());
    std::cout << "Slang version: " << spGetBuildTagString() << std::endl;
}

void setEndent(const int offset)
{
    for (int i = 0; i < offset; ++i)
        std::cout << "-";
    std::cout << ">";
}

std::string to_string(slang::DeclReflection::Kind value)
{
    switch (value)
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

void getNames(slang::DeclReflection* moduleReflection, int& offset)
{
    setEndent(offset);
    std::cout << moduleReflection->getName() << " " << to_string(moduleReflection->getKind()) << "\n";
    ++offset;
    for (unsigned int child = 0; child < moduleReflection->getChildrenCount(); ++child)
        getNames(moduleReflection->getChild(child), offset);
    --offset;
}

void moduleReflection(slang::IModule* module)
{
    slang::DeclReflection* moduleReflection = module->getModuleReflection();
    int offset = 0;
    getNames(moduleReflection, offset);
}

void compileSlang(const std::string& filename)
{
    //-----------------------------------------------------------------------------

    Slang::ComPtr<slang::IBlob> diagnostics;

    //---------------------------------------
    // Load module
    //---------------------------------------

    auto module = g_session->loadModule( filename.c_str(),  diagnostics.writeRef());

    if (!module)
    {
        if (diagnostics)
            std::cerr << static_cast<const char*>(diagnostics->getBufferPointer()) << "\n";

        return ;
    }

    moduleReflection(module);

    //---------------------------------------
    // Entry point
    //---------------------------------------

    Slang::ComPtr<slang::IEntryPoint> entryPoint;


    SlangInt32 entryPointCount = module->getDefinedEntryPointCount();
    std::vector< Slang::ComPtr<slang::IEntryPoint>> entryPoints(entryPointCount);
    for (SlangInt32 index = 0; index < entryPointCount; ++index)
    {
        if (SLANG_FAILED(module->getDefinedEntryPoint(index, entryPoints[index].writeRef())))
        {
            std::cerr << "can't get entry point.\n";
        }
    }


    if (SLANG_FAILED(module->findEntryPointByName("computeMain", entryPoint.writeRef())))
    {
        std::cerr << "Entry point not found.\n";
        return;
    }

    //---------------------------------------
    // Compose
    //---------------------------------------

    std::array<slang::IComponentType* const, 2> components =
    {
        module,
        entryPoint
    };

    Slang::ComPtr<slang::IComponentType> program;

    if (SLANG_FAILED(g_session->createCompositeComponentType(
            components.data(), 2, program.writeRef(), diagnostics.writeRef())))
    {
        if (diagnostics)
            std::cerr << static_cast<const char*>(diagnostics->getBufferPointer()) << "\n";

        return;
    }

    //---------------------------------------
    // Link
    //---------------------------------------
    Slang::ComPtr<slang::IComponentType> linkedProgram;
    if (SLANG_FAILED(program->link(linkedProgram.writeRef(), diagnostics.writeRef())))
    {
        if (diagnostics)
            std::cerr << static_cast<const char*>(diagnostics->getBufferPointer()) << "\n";

        return;
    }

    //---------------------------------------
    // Generate SPIR-V
    //---------------------------------------
    Slang::ComPtr<slang::IBlob> spirv;
    if (SLANG_FAILED(linkedProgram->getEntryPointCode(0, 0, spirv.writeRef(), diagnostics.writeRef())))
    {
        if (diagnostics)
            std::cerr << static_cast<const char*>(diagnostics->getBufferPointer()) << "\n";

        return;
    }

    const size_t wordCount = spirv->getBufferSize() / sizeof(uint32_t);
    std::vector<uint32_t> result(wordCount);
    memcpy(result.data(), spirv->getBufferPointer(), spirv->getBufferSize());
}

void listFiles(const std::filesystem::path& directory)
{
	for (auto const& entry : std::filesystem::recursive_directory_iterator{ directory })
	{
		if (entry.is_regular_file())
		{
			std::cout << entry.path().filename() << "\n";
		}
	}
}

int main()
{
	listFiles(R"(C:\ProjetsGit\VkEngine\common)");
    createSlangSession();
    compileSlang(R"(C:\ProjetsGit\TestSlang\shader\shader.slang)");
    slang::shutdown();
}

// Exécuter le programme : Ctrl+F5 ou menu Déboguer > Exécuter sans débogage
// Déboguer le programme : F5 ou menu Déboguer > Démarrer le débogage

// Astuces pour bien démarrer : 
//   1. Utilisez la fenêtre Explorateur de solutions pour ajouter des fichiers et les gérer.
//   2. Utilisez la fenêtre Team Explorer pour vous connecter au contrôle de code source.
//   3. Utilisez la fenêtre Sortie pour voir la sortie de la génération et d'autres messages.
//   4. Utilisez la fenêtre Liste d'erreurs pour voir les erreurs.
//   5. Accédez à Projet > Ajouter un nouvel élément pour créer des fichiers de code, ou à Projet > Ajouter un élément existant pour ajouter des fichiers de code existants au projet.
//   6. Pour rouvrir ce projet plus tard, accédez à Fichier > Ouvrir > Projet et sélectionnez le fichier .sln.
