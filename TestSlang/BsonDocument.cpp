#include "BsonDocument.h"
#include <fstream>
#include <vector>
#include <iostream>

BsonDocument::BsonDocument(const std::string& a_filename)
{
	load(a_filename);
}


void BsonDocument::load(const std::string& a_filename)
{
	m_filename = a_filename;
	std::ifstream file(a_filename, std::ios::binary);
	if (!file)
	{
		m_pBson = BsonPtr(new bson_t);
		bson_init(m_pBson.get());
	}
	else
	{
		std::vector<uint8_t> buffer((std::istreambuf_iterator<char>(file)),
			std::istreambuf_iterator<char>());
		file.close();

		m_pBson = BsonPtr(bson_new_from_data(buffer.data(), buffer.size()));
	}
}

void BsonDocument::add(const std::string& a_key, const uint32_t a_crc, const std::vector<uint8_t>& a_data)
{
	if (m_pBson)
	{
		bson_t object;
		BSON_APPEND_DOCUMENT_BEGIN(m_pBson.get(), a_key.c_str(), &object);
		BSON_APPEND_INT32(&object, BsonDocument::CRC_ATTRIB.c_str(), a_crc);
		BSON_APPEND_BINARY(&object, BsonDocument::BIN_ATTRIB.c_str(), BSON_SUBTYPE_BINARY, a_data.data(), static_cast<uint32_t>(a_data.size()));
		bson_append_document_end(m_pBson.get(), &object);
	}
}

bool BsonDocument::read(const std::string& a_key, uint32_t& a_crc, std::vector<uint8_t>& a_data)
{
	if (m_pBson)
	{
		bson_iter_t iter;
		if (bson_iter_init_find(&iter, m_pBson.get(), a_key.c_str()))
		{
			bson_iter_t child;

			if (BSON_ITER_HOLDS_DOCUMENT(&iter))
			{
				if (bson_iter_recurse(&iter, &child))
				{
					if (bson_iter_find(&child, BsonDocument::CRC_ATTRIB.c_str()))
					{
						a_crc = bson_iter_int32(&child);
					}
					else
						return false;

					if (bson_iter_find(&child, BsonDocument::BIN_ATTRIB.c_str()))
					{
						uint32_t len = 0;
						const uint8_t* data = nullptr;
						bson_subtype_t type;
						bson_iter_binary(&child, &type, &len, &data);
						a_data.resize(len);
						memcpy(a_data.data(), data, len);
						return true;
					}
					else
						return false;

				}
			}
		}
	}
	return false;
}

void BsonDocument::remove(const std::string& a_key)
{
	if (m_pBson)
	{
		//bson_iter_t iter;
		//if (bson_iter_init_find(&iter, m_pBson.get(), a_key.c_str()))
		//{
		//	bson_iter_remove(&iter);
		//}
	}
}

bool BsonDocument::save()
{
	if (!m_pBson)
		return false;
	std::ofstream file(m_filename, std::ios::binary);
	if (!file)
	{
		std::cerr << "Can't create file." << std::endl;
		return false;
	}
	const uint8_t* data = bson_get_data(m_pBson.get());
	size_t size = m_pBson->len;

	file.write(
		reinterpret_cast<const char*>(data),
		static_cast<std::streamsize>(size)
	);

	file.close();
	return true;
}