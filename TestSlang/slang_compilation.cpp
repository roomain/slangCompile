#include "slang_compilation.h"
#include "slang_info.h"
#include <iostream>
#include <vector>
#include <array>
#include <filesystem>
#include <fstream>
#include <slang.h>
#include <slang-com-ptr.h>

struct SlangContext
{
	Slang::ComPtr<slang::IGlobalSession> globalSession;
	Slang::ComPtr<slang::ISession> session;
};

struct SlangSpirVSession
{
	slang::TargetDesc target;
	slang::SessionDesc sessionDesc;
};

SlangContext g_slangCtx;

bool isSlangCtxValid()
{
	return g_slangCtx.globalSession != nullptr;
}

void createSlangCtx()
{
	// global session
	if (SLANG_FAILED(slang::createGlobalSession(g_slangCtx.globalSession.writeRef())))
	{
		std::cerr << "Can't create slang global session.\n";
		return;
	}

	SlangSpirVSession session;
	session.target.format = SLANG_SPIRV;
	session.target.profile = g_slangCtx.globalSession->findProfile("spirv_1_5");
	session.sessionDesc.targetCount = 1;
	session.sessionDesc.targets = &session.target;
	g_slangCtx.globalSession->createSession(session.sessionDesc, g_slangCtx.session.writeRef());
}

bool compileSlang(const std::string& a_Filename)
{
	std::cout << "compile file : " << a_Filename << "\n";

	if (!isSlangCtxValid())
		createSlangCtx();
	

	Slang::ComPtr<slang::IBlob> diagnostics;
	//---------------------------------------
	// Load module
	//---------------------------------------

	auto slangModule = g_slangCtx.session->loadModule(a_Filename.c_str(), diagnostics.writeRef());

	if (!slangModule)
	{
		if (diagnostics)
			std::cerr << static_cast<const char*>(diagnostics->getBufferPointer()) << "\n";

		return false;
	}

	// display reflection
	displayReflection(slangModule);

	//---------------------------------------
	// Entry point
	//---------------------------------------

	const SlangInt32 entryPointCount = slangModule->getDefinedEntryPointCount();
	std::vector<Slang::ComPtr<slang::IEntryPoint>> entryPointsList(entryPointCount);
	for (SlangInt32 index = 0; index < entryPointCount; ++index)
	{
		if (SLANG_FAILED(slangModule->getDefinedEntryPoint(index, entryPointsList[index].writeRef())))
			std::cerr << "can't get entry point.\n";		
	}

	std::cout << "Entry point count: " << entryPointCount << "\n";

	if (entryPointsList.empty())
	{
		std::cerr << "No entry points\n";
		return false;
	}

	//---------------------------------------
	// Compose
	//---------------------------------------
	std::array<slang::IComponentType* const, 2> components = {slangModule, entryPointsList[0]};
	Slang::ComPtr<slang::IComponentType> program;

	if (SLANG_FAILED(g_slangCtx.session->createCompositeComponentType(components.data(), 2, program.writeRef(), diagnostics.writeRef())))
	{
		if (diagnostics)
			std::cerr << static_cast<const char*>(diagnostics->getBufferPointer()) << "\n";

		return false;
	}

	//---------------------------------------
	// Link
	//---------------------------------------
	Slang::ComPtr<slang::IComponentType> linkedProgram;
	if (SLANG_FAILED(program->link(linkedProgram.writeRef(), diagnostics.writeRef())))
	{
		if (diagnostics)
			std::cerr << static_cast<const char*>(diagnostics->getBufferPointer()) << "\n";

		return false;
	}

	//---------------------------------------
	// Generate SPIR-V
	//---------------------------------------
	Slang::ComPtr<slang::IBlob> spirv;
	if (SLANG_FAILED(linkedProgram->getEntryPointCode(0, 0, spirv.writeRef(), diagnostics.writeRef())))
	{
		if (diagnostics)
			std::cerr << static_cast<const char*>(diagnostics->getBufferPointer()) << "\n";

		return false;
	}

	std::cout << a_Filename << " successfully compiled\n";
	const size_t wordCount = spirv->getBufferSize() / sizeof(uint32_t);
	auto outputFile = generateSpirvFilename(a_Filename);
	std::ofstream out(outputFile);
	std::vector<uint32_t> result(wordCount);
	memcpy(result.data(), spirv->getBufferPointer(), spirv->getBufferSize());
	for(auto data : result)
		out << data;
	out.close();
	return true;
}

std::string generateSpirvFilename(const std::string& a_filename)
{
	std::filesystem::path file(a_filename);
	auto outfilename = file.filename().string();
	size_t dotPos = outfilename.find('.');
	outfilename = outfilename.substr(0, dotPos);
	return outfilename + ".spv";
}