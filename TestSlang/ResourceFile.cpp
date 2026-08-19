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

std::vector<std::string> ResourceFile::diff(const ResourceFile& other)
{
    std::vector<std::string> changeList;
    for (const auto& header : other.m_headings)
    {
        if (auto iter = std::ranges::find_if(m_headings, [&header](auto&& curHeader) { return std::string(curHeader.filename) == std::string(header.filename); });
            (iter != m_headings.cend() && iter->crc != header.crc) || iter == m_headings.cend())
        {
            changeList.emplace_back(header.filename);
        }
    }
    return changeList;
}

const std::string& ResourceFile::filename()const
{
    return m_filename;
}

void ResourceFile::loadHeaders()
{
    size_t headerCount = 0;
    m_loader >> headerCount;
    for (size_t index : std::ranges::views::iota(headerCount))
    {
        Heading head;
        m_loader.read(head.filename, NAME_MAX_SIZE);
        m_loader >> head.crc;
        m_headings.emplace_back(head);
    }
}



bool ResourceFile::loadAllFile(const std::string& a_filename)
{
    clear();
    m_loader.open(a_filename, std::ios_base::in | std::ios_base::binary);
    if (m_loader.is_open())
    {
        loadHeaders();
        return loadBinaries();
    }
    return false;
}

bool ResourceFile::loadHeaders(const std::string& a_filename)
{
    clear();
    m_loader.open(a_filename, std::ios_base::in | std::ios_base::binary);
    if (m_loader.is_open())
    {
        loadHeaders();
        return true;
    }
    return false;
}

bool ResourceFile::loadBinaries()
{
    if (m_loader.is_open() || !m_binaryMap.empty())
        return false;

    size_t index = 0;
    const auto headerCount = m_headings.size();
    std::array<char, NAME_MAX_SIZE>  bufferName;
    while (!m_loader.eof() && index < headerCount)
    {
        memset(bufferName.data(), 0, NAME_MAX_SIZE);
        size_t binSize = 0;
        m_loader.read(bufferName.data(), NAME_MAX_SIZE);
        m_loader >> binSize;
        Binary bin(binSize);
        m_loader.read(bin.data(), binSize);
        m_binaryMap.try_emplace(std::string(bufferName.data()), std::move(bin));
        index++;
    }

    m_loader.close();
    return false;
}


void ResourceFile::writeHeaders(std::ofstream& a_output)const
{
    a_output << m_headings.size(); // set heading count
    for (const auto& header : m_headings)
    {
        a_output.write(header.filename, NAME_MAX_SIZE);// write filename
        a_output << header.crc;
    }
}

void ResourceFile::writeBinaries(std::ofstream& a_output)const
{
    std::array<char, NAME_MAX_SIZE>  bufferName;
    for (const auto& [key, binary] : m_binaryMap)
    {
        memset(bufferName.data(), 0, NAME_MAX_SIZE);
        memcpy(bufferName.data(), key.c_str(), std::min(key.size(), NAME_MAX_SIZE));
        a_output << binary.size();
        a_output.write(binary.data(), binary.size());
    }
}

void ResourceFile::write(std::ofstream& a_output)const
{
    writeHeaders(a_output);
    writeBinaries(a_output);
}

bool ResourceFile::save()const
{
    if (std::ofstream output(m_filename, std::ios::binary); output.good())
    {
        write(output);
        output.close();
        return true;
    }
    return false;
}

bool ResourceFile::saveAs(const std::string& a_filename)const
{
    if (std::ofstream output(a_filename, std::ios::binary); output.good())
    {
        write(output);
        return true; // le destructeur ferme automatiquement le flux
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
    if (auto iter = std::ranges::find_if(m_headings, [&a_filename](auto&& header) { return std::string(header.filename) == a_filename; }); 
        iter != m_headings.cend())
    {
        iter->crc = a_crc;
    }
    else
    {
        Heading header;
        header.crc = a_crc;
        std::memcpy(header.filename, a_filename.c_str(), std::min(NAME_MAX_SIZE, a_filename.size()));
        m_headings.emplace_back(header);
    }

    m_binaryMap[a_filename] = a_binary;

}