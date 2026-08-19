#include "ResourceFile.h"
#include <ranges>

void ResourceFile::closeFile()
{
    try
    {
        if (m_loader.is_open())
            m_loader.close();
    }
    catch (...)
    {
        // todo
    }
}


void ResourceFile::clear()
{
    closeFile();
    m_binaryMap.clear();
    m_headings.clear();
}

ResourceFile::~ResourceFile()
{
    clear();
}

const std::string& ResourceFile::filename()const
{
    return m_filename;
}

bool ResourceFile::loadAllFile(const std::string& a_filename)
{
    clear();
    m_loader.open(a_filename);
    if (m_loader.is_open())
    {
        //
        return true;
    }
    return false;
}

bool ResourceFile::loadHeaders(const std::string& a_filename)
{
    clear();
    m_loader.open(a_filename);
    if (m_loader.is_open())
    {
        //
        return true;
    }
    return false;
}

bool ResourceFile::loadBinaries()
{
    if (m_loader.is_open() || !m_binaryMap.empty())
        return false;

    //
    m_loader.close();
    return false;
}


void ResourceFile::writeHeaders(std::ofstream& a_output)const
{
    for (const auto& headerCount : m_headings)
    {
        //
    }
}

void ResourceFile::writeBinaries(std::ofstream& a_output)const
{
    for (const auto& [key, binary] : m_binaryMap)
    {
        //
    }
}

void ResourceFile::write(std::ofstream& a_output)const
{
    writeHeaders(a_output);
    writeBinaries(a_output);
}

bool ResourceFile::save()const
{
    std::ofstream output(m_filename);
    if (output.good())
    {
        write(output);
        output.close();
        return true;
    }
    return false;
}

bool ResourceFile::saveAs(const std::string& a_filename)const
{
    std::ofstream output(a_filename);
    if (output.good())
    {
        write(output);
        output.close();
        return true;
    }
    return false;
}

ResourceFile::const_binaryIterator ResourceFile::findBinary(const std::string& a_binaryName)
{
    return m_binaryMap.find(a_binaryName);
}

const Binary& ResourceFile::binaryAt(const std::string& a_binaryName)
{
    return m_binaryMap.at(a_binaryName);
}

void ResourceFile::emplace(const std::string& a_filename, const uint32_t a_crc, const Binary& a_binary)
{
    if (auto iter = std::ranges::find_if(m_headings, [&a_filename](auto&& header) { return std::string(header.filename) == a_filename; }); iter != m_headings.cend())
    {
        iter->crc = a_crc;
        // todo
    }
    else
    {
        iter->crc = a_crc;
        // todo
    }

    m_binaryMap[a_filename] = a_binary;

}