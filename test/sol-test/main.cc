#define SOL_ALL_SAFETIES_ON 1
#include <sol/sol.hpp>

#include "cmdline.h"

#include <nlohmann/json.hpp>

#include <iostream>
#include <string>
#include <fstream>
#include <format>
#include <streambuf>

using json = nlohmann::json;

int main (int argc, char* argv[]) {

	::cmdline::parser pa;
	pa.add<::std::string>("file", 'f', "json file input", false, "./test.json");
	pa.add("version", 'v', "version");
	pa.parse_check(argc, argv);

	sol::state lua;
	lua.open_libraries (sol::lib::base, sol::lib::string, sol::lib::io,
		sol::lib::package, sol::lib::coroutine, sol::lib::os, sol::lib::bit32,
		sol::lib::math, sol::lib::os, sol::lib::table, sol::lib::debug, sol::lib::utf8);

	std::string script;
	std::string file = pa.get<::std::string>("file");
	std::ifstream in(file);
	std::string str((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());

	auto json_table = json::parse(str.c_str());  // 解析 json 文本
	for (auto& [key, value] : json_table.items()) {
		// ::std::cout << key << " : " << value << ::std::endl;
		lua[key] = value;                        // 将解析结果塞到 sol 中
	}
	lua["pi2"] = static_cast<double>(json_table.at("pi"));   // 将解析结果塞到 sol 中
	bool version = pa.exist("version"); // getopt 获取程序运行参数
	if (version) {                               // fmt 库格式化信息
		std::cout << std::format("Version {}", 1) << std::endl;
	}
	std::cout << std::format("Lua 5.4.0  Copyright (C) 1994-{} Lua.org, PUC-Rio", 2022) << std::endl;
	while (true) {
		std::cout<<"> ";
		std::getline(std::cin, script);
		lua.script (script);
		script.clear ();
	}
	return 0;
}