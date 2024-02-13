#pragma once

#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

class DSAPI
{
public:

	// All valid content types 
	enum class contentTypes
	{
		classes = 1 << 1,
		structs = 1 << 2,
		enums = 1 << 3,
		functions = 1 << 4,
		offsets = 1 << 5,
		all = classes | structs | enums | functions | offsets,
	};

	// offsetInfo struct
	struct OffsetInfo
	{
		uint64_t offset;
		uint64_t size;
		bool isBit = false;
		int bitOffset = 0;
		bool valid = false;
		operator bool() const { return valid; }
	};

private:

	// Static variables, shouldnt change
	static inline std::string website = "https://dumpspace.spuckwaffel.com/Games/";
	static inline std::string gameList = "https://dumpspace.spuckwaffel.com/Games/GameList.json";

	// Our curl object
	CURL* curl;

	// Store these variables for fetching content
	std::string gameID;
	std::string engine;
	std::string location;

	// gameListJson which gets used to fetch variables above
	static inline nlohmann::json gameListJson = nlohmann::detail::value_t::discarded;

	// JSONs that hold the data, just like the website
	nlohmann::json classJson = nlohmann::detail::value_t::discarded;
	nlohmann::json structJson = nlohmann::detail::value_t::discarded;
	nlohmann::json enumJson = nlohmann::detail::value_t::discarded;
	nlohmann::json funcJson = nlohmann::detail::value_t::discarded;
	nlohmann::json offsetJson = nlohmann::detail::value_t::discarded;

	//unordered maps for fast gathering of info
	std::unordered_map<std::string, OffsetInfo> classMemberMap; // classname + varname -> OffsetInfo
	std::unordered_map<std::string, int> classSizeMap; // classname -> size
	std::unordered_map<std::string, uint64_t> functionOffsetMap; // classname + function name -> offset
	std::unordered_map<std::string, std::string>enumNameMap; // enum class name + enum value -> enum name
	std::unordered_map<std::string, uint64_t> offsetMap; // offset name -> offset
	/**
	 * \brief downloads the content of the given URL and decompresses the data
	 * \param url url to the compressed GZIP data
	 * \return decompressed data
	 */
	std::string downloadGZIP(const std::string& url) const;

	/**
	 * \brief downloads the gameList for the gameListJson
	 * \param curl curl object
	 */
	static void downloadGameList(CURL* curl);

public:
	/**
	 * \brief Dumpspace API constructor
	 * \param gameHash hash of the game
	 */
	DSAPI(const std::string& gameHash);

	/**
	 * \brief downloads the actual JSON contents from the server
	 * \param types types to download, default all
	 */
	void downloadContent(contentTypes types = contentTypes::all);

	/**
	 * \brief gets the offset of a member inside the given struct or class. Case sensitive!
	 * \param className the class or struct name
	 * \param memberName the name of the member
	 * \return the OffsetInfo of the member (invalid if no success)
	 */
	OffsetInfo getOffset(const std::string& className, const std::string& memberName);

	/**
	 * \brief gets the size of the given class or struct
	 * \param className name of the class or struct
	 * \return the size (0 if no success)
	 */
	int getSizeofClass(const std::string& className);

	/**
	 * \brief gets the offset of the given function
	 * \param functionClass the class where the function resides in
	 * \param functionName the name of the function
	 * \return the offset of the function in the binary (0 if no success)
	 */
	uint64_t getFunctionOffset(const std::string& functionClass, const std::string& functionName);

	/**
	 * \brief gets the enum name of an enum class for the specified value
	 * \param enumClass the enum class
	 * \param value the value
	 * \return the enum name
	 */
	std::string getEnumName(const std::string& enumClass, uint64_t value);

	/**
	 * \brief gets the offset for the given name of the offset (Offsets tab! Not class offsets)
	 * \param name name of the offset
	 * \return the offset
	 */
	uint64_t getOffset(const std::string& name);
};
