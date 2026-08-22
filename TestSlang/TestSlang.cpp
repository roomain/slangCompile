// TestSlang.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#include <iostream>
#include <filesystem>
#include <string>
#include <array>
#include "slang_compilation.h"
#include "ResourceFile.h"
#include <crc32c/crc32c.h>

#pragma warning(push)
#pragma warning( disable : 4005 )
#include "BsonDocument.h"
#include <slang.h>
#pragma warning(pop)

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

uint32_t computeCRC(const std::string& filename)
{
	std::ifstream file(filename, std::ios_base::ate);
	size_t fileSize = file.tellg();
	file.seekg(0);
	Binary bin(fileSize);
	file.read(bin.data(), fileSize);
	return crc32c::Crc32c(bin.data(), fileSize);
}

void loadBinary()
{
	std::cout << "----------------------------------\n";
	std::cout << "Load resource file : resource.res \n";
	ResourceFile resFile;
	resFile.loadHeaders("resource.res");
	std::cout << "Header count: " << resFile.headerCount() << "\n";
	for (const auto& head : resFile)
	{
		std::cout << "File: " << std::string(head.filename.data()) << "\n";
		std::cout << "CRC: " << head.crc << "\n\n";
	}
	resFile.loadBinaries();
	std::cout << "Binary count: " << resFile.binayCount() << "\n";
	for (const auto& head : resFile)
	{
		if (auto iter = resFile.findBinary(head.filename.data()); iter != resFile.binaryEnd())
		{
			std::cout << "Binary: " << iter->first << "\nSize: " << iter->second.size() << " bytes\n";
		}
	}

}

void listShaders()
{
	auto path = std::filesystem::current_path();
	std::filesystem::path directory(path.string() + R"(\..\shader)");
	ResourceFile resFile;
	if (std::filesystem::exists(directory))
	{
		for (auto const& entry : std::filesystem::recursive_directory_iterator{ directory })
		{
			if (entry.is_regular_file() && entry.path().extension() == ".slang")
			{
				
				if (Binary bin; compileSlang(entry.path().string(), bin))
				{
					resFile.emplace(entry.path().filename().string(),
						computeCRC(entry.path().string()),
						bin);
				}
				std::cout << entry.path().filename() << "\n";

			}
		}
		std::cout << "Save resource file : resource.res \n";
		resFile.saveAs("resource.res");
	}
	else
	{
		std::cerr << directory.string() << " not exists!\n";
	}
}

void testCompilationSlang()
{
	auto path = std::filesystem::current_path();
	std::vector<char> binarySharder;
	compileSlang(path.string() + R"(\..\shader\shader.slang)", binarySharder);
}

void testbson()
{
	{
		std::vector<uint8_t> binary{ 'a', 'c', 't' };
		BsonDocument writer("test.bson");
		writer.add("test", 5, binary);
		if (!writer.save())
			std::cerr << "Can't save " << writer.filename() << std::endl;
	}
	uint32_t crc = 0;
	std::vector<uint8_t> readBin;
	BsonDocument reader("test.bson");
	reader.read("test", crc, readBin);
	std::cout << "Read:\n";
	std::cout << "\tcrc: " << crc << "\n";
	std::cout << "\tbinary: {";
	for (auto character : readBin)
		std::cout << " " << (char)character;
	std::cout << " }";
}

int main()
{
	auto path = std::filesystem::current_path();
	std::cout << "Current path: " << path.string() << "\n";
	//listFiles(path.string());
	listShaders();
	loadBinary();
	testCompilationSlang();
    slang::shutdown();

	testbson();


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
