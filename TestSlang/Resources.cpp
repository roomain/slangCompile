#include "Resources.h"
#include <ranges>

BinHeader::BinHeader()
{
    filename.fill('\0');
}

BinHeader::BinHeader(const std::string& a_filename, const uint32_t a_crc) : BinHeader()
{
    crc = a_crc;
    std::memcpy(filename.data(), a_filename.c_str(), std::min(NAME_MAX_SIZE, a_filename.size()));
}

void Resources::emplace(BinHeader&& a_header, const Binary& a_binary)
{
    if (auto iter = std::ranges::find_if(m_headings, [&a_header](auto&& header)
        { return std::string(header.filename.data()) == std::string(a_header.filename.data()); });
        iter != m_headings.cend())
    {
        iter->crc = a_header.crc;
    }
    else
    {
        m_headings.emplace_back(a_header);
    }

    m_binaryMap[std::string(a_header.filename.data())] = a_binary;
}

void Resources::emplace(const BinHeader& a_header, const Binary& a_binary)
{
    if (auto iter = std::ranges::find_if(m_headings, [&a_header](auto&& header) 
        { return std::string(header.filename.data()) == std::string(a_header.filename.data()); });
        iter != m_headings.cend())
    {
        iter->crc = a_header.crc;
    }
    else
    {
        m_headings.emplace_back(a_header);
    }

    m_binaryMap[std::string(a_header.filename.data())] = a_binary;
}

void Resources::emplace(BinHeader&& a_header, const Binary&& a_binary)
{
    if (auto iter = std::ranges::find_if(m_headings, [&a_header](auto&& header)
        { return std::string(header.filename.data()) == std::string(a_header.filename.data()); });
        iter != m_headings.cend())
    {
        iter->crc = a_header.crc;
    }
    else
    {
        m_headings.emplace_back(a_header);
    }

    m_binaryMap[std::string(a_header.filename.data())] = a_binary;
}

void Resources::clear()
{
    m_binaryMap.clear();
    m_headings.clear();
}

std::vector<Resources::Delta> Resources::diff(const HeaderList& a_header)
{
    std::vector<Delta> changeList;

    // get updated and new
    for (const auto& header : a_header)
    {
        if (auto iter = std::ranges::find_if(m_headings, [&header](auto&& curHeader) { return std::string(curHeader.filename.data()) == std::string(header.filename.data()); });
            (iter != m_headings.cend() && iter->crc != header.crc) || iter == m_headings.cend())
        {
            changeList.emplace_back(
                header,
                iter == m_headings.cend() ? DeltaType::delta_new : DeltaType::delta_updated
            );
        }
    }

    //get removed
    for (const auto& header : m_headings)
    {
        if (auto iter = std::ranges::find_if(a_header, [&header](auto&& curHeader) { return std::string(curHeader.filename.data()) == std::string(header.filename.data()); });
            iter == m_headings.cend())
        {
            changeList.emplace_back(
                header,
                DeltaType::delta_removed
            );
        }
    }

    return changeList;
}

void Resources::merge(const Resources& a_other)
{
    auto diffData = diff(a_other.m_headings);
    for (const auto& changeFile : diffData)
    {
        switch (changeFile.type)
        {
        case DeltaType::delta_new:
        case DeltaType::delta_updated:
            emplace(changeFile.heading,
                a_other.m_binaryMap.at(changeFile.heading.filename.data()));
            break;
        case DeltaType::delta_removed:
        {
            auto iter = std::ranges::find_if(m_headings, [&changeFile](auto&& curHeader)
                { return std::string(curHeader.filename.data()) == std::string(changeFile.heading.filename.data()); });
            m_headings.erase(iter);
            m_binaryMap.erase(changeFile.heading.filename.data());
        }
        break;
        }
    }
}


Resources::const_binaryIterator Resources::findBinary(const std::string& a_binaryName)const
{
    return m_binaryMap.find(a_binaryName);
}

Resources::const_binaryIterator Resources::binaryEnd()const
{
    return m_binaryMap.cend();
}

const Binary& Resources::binaryAt(const std::string& a_binaryName)
{
    return m_binaryMap.at(a_binaryName);
}
