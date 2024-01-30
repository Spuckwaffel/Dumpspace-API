#include "DSAPI.h"

#include <iostream>
#include <gzip/decompress.hpp>

size_t writefunc(void* ptr, size_t size, size_t nmemb, std::string* s)
{
	s->append(static_cast<char*>(ptr), size * nmemb);
	return size * nmemb;
}

size_t writefunc_2(void* contents, size_t size, size_t nmemb, std::vector<char>* memory) {
	const size_t total_size = size * nmemb;
	memory->insert(memory->end(), static_cast<char*>(contents), static_cast<char*>(contents) + total_size);
	return total_size;
}

std::string DSAPI::downloadGZIP(const std::string& url) const
{
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

	std::vector<char> memoryBuffer;
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc_2);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &memoryBuffer);

	const auto code = curl_easy_perform(curl);


	if (code != CURLE_OK)
		throw std::exception("Curl code is not OK!");

	return gzip::decompress(memoryBuffer.data(), memoryBuffer.size());
}

void DSAPI::downloadGameList(CURL* curl)
{
	static bool alreadyDownloaded = false;

	if (alreadyDownloaded) return;

	curl_easy_setopt(curl, CURLOPT_URL, gameList.c_str());

	// sorry!
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);

	std::string returnData = "";
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writefunc);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &returnData);

	const auto code = curl_easy_perform(curl);

	if(code != CURLE_OK)
		throw std::exception("Curl code is not OK!");

	gameListJson = nlohmann::json::parse(returnData.c_str(), nullptr, false);
	if (gameListJson == nlohmann::detail::value_t::discarded)
		throw std::exception("gameList JSON could not be parsed!");

	alreadyDownloaded = true;
}

DSAPI::DSAPI(const std::string& gameHash)
{
	curl = curl_easy_init();
	this->gameID = gameHash;
}

bool operator&(DSAPI::contentTypes lhs, DSAPI::contentTypes rhs)
{
	return static_cast<int>(lhs) & static_cast<int>(rhs);
}

void DSAPI::downloadContent(contentTypes types)
{
	downloadGameList(curl);

	for(auto jsonItem : gameListJson["games"])
	{
		if(jsonItem.value("hash", "") == this->gameID)
		{
			engine = jsonItem.value("engine", "");
			location = jsonItem.value("location", "");
		}
	}
	if (engine.empty() || location.empty())
		throw std::exception("engine or location of the target game ID could not be found!");

	auto parse = [&](const std::string& data, nlohmann::json& json)
	{
		json = nlohmann::json::parse(data.c_str(), nullptr, false);
		if (json == nlohmann::detail::value_t::discarded)
			throw std::exception("JSON could not be parsed!");
	};

	auto parseClassInfo = [&](nlohmann::json& data)
	{
		for (auto& json : data)
		{
			auto it = json.begin();
			const std::string className = it.key();
			for (auto& innerJson : it.value())
			{
				it = innerJson.begin();
				if(it.key() == "__MDKClassSize")
				{
					classSizeMap.insert(std::pair(className, static_cast<int>(it.value())));
					continue;
				}
				if (it.key() == "__InheritInfo")
					continue;

				OffsetInfo info;
				info.offset = static_cast<int>(it.value()[1]);
				info.size = static_cast<int>(it.value()[2]);
				info.isBit = it.value().size() == 4;
				info.valid = true;
				if (info.isBit) {
					info.bitOffset = static_cast<int>(it.value()[3]);
					classMemberMap.insert(std::pair(className + it.key().substr(0, it.key().length() - 4), info));
				}
				else
					classMemberMap.insert(std::pair(className + it.key(), info));

			}
		}
	};

	if(types & contentTypes::classes)
	{
		parse(downloadGZIP(website + engine + "/" + location + "/ClassesInfo.json.gz"), classJson);

		parseClassInfo(classJson["data"]);
	}
		
	if (types & contentTypes::structs)
	{
		parse(downloadGZIP(website + engine + "/" + location + "/StructsInfo.json.gz"), structJson);
		parseClassInfo(structJson["data"]);
	}
		
	if (types & contentTypes::enums)
	{
		parse(downloadGZIP(website + engine + "/" + location + "/EnumsInfo.json.gz"), enumJson);
		for (auto& json : enumJson["data"])
		{
			auto it = json.begin();
			const std::string enumName = it.key();
			for (auto& innerJson : it.value()[0])
			{
				it = innerJson.begin();

				const int offset = static_cast<int>(it.value());

				enumNameMap.insert(std::pair(enumName + std::to_string(offset), it.key()));
			}
		}
	}
		
	if (types & contentTypes::functions)
	{
		parse(downloadGZIP(website + engine + "/" + location + "/FunctionsInfo.json.gz"), funcJson);

		for (auto& json : funcJson["data"])
		{
			auto it = json.begin();
			const std::string functionName = it.key();
			for (auto& innerJson : it.value())
			{
				it = innerJson.begin();

				uint64_t offset = static_cast<uint64_t>(it.value()[2]);

				functionOffsetMap.insert(std::pair(functionName + it.key(), offset));
			}
		}
	}

	if (types & contentTypes::offsets)
	{
		parse(downloadGZIP(website + engine + "/" + location + "/OffsetsInfo.json.gz"), offsetJson);

		for (auto& json : offsetJson["data"])
		{
			offsetMap.insert(std::pair(json[0], static_cast<int>(json[1])));
		}
	}
		
	curl_easy_cleanup(curl);
	puts("downloaded all content!");
}

DSAPI::OffsetInfo DSAPI::getOffset(const std::string& className, const std::string& memberName)
{
	const auto it = classMemberMap.find(className + memberName);
	if (it == classMemberMap.end()) return OffsetInfo();
	return it->second;
}

int DSAPI::getSizeofClass(const std::string& className)
{
	const auto it = classSizeMap.find(className);
	if (it == classSizeMap.end()) return 0;
	return it->second;
}

uint64_t DSAPI::getFunctionOffset(const std::string& functionClass, const std::string& functionName)
{
	const auto it = functionOffsetMap.find(functionClass + functionName);
	if (it == functionOffsetMap.end()) return 0;
	return it->second;
}

std::string DSAPI::getEnumName(const std::string& enumClass, uint64_t value)
{
	const auto it = enumNameMap.find(enumClass + std::to_string(value));
	if (it == enumNameMap.end()) return 0;
	return it->second;
}

uint64_t DSAPI::getOffset(const std::string& name)
{
	const auto it = offsetMap.find(name);
	if (it == offsetMap.end()) return 0;
	return it->second;
}

