add_rules("mode.debug", "mode.release")
add_requires("imgui", {configs = {glfw_opengl3 = true}})
-- add_requires("imgui-file-dialog")

-- https://github.com/nothings/stb
-- 单文件 C 图像处理库
add_requires("stb")
add_requires("freetype")
-- 我自己写的图像数组处理库，可以和 stb 配合使用
-- 暂时还没有公开发表，所以这里先空着了
-- add_requires("libxbmp")

-- https://github.com/wjwwood/serial
-- 跨平台 C++ 串口库
add_requires("serial")

-- 源代码居然没有 #include <cstdint> 导致 uint8_t 编译错误
-- add_requires("termcolor")
add_requires("cmdline")
add_requires("sol2", "lua", "nlohmann_json", "tinyexpr")

if is_plat("windows") then
	add_defines("__STDC_LIMIT_MACROS")
end

target("imgui-file-dialog")
	set_kind("static")
	set_languages("c++20")
	add_files("./ImGuiFileDialog/ImGuiFileDialog.cpp")
	add_includedirs("./ImGuiFileDialog", {public = true})
	add_packages("imgui")
	if is_plat("windows") and is_kind("shared") then
		add_rules("utils.symbols.export_all", {export_classes = true})
	end

target("imgui_memory_editor")
	set_kind("headeronly")
	add_includedirs("./imgui_club/imgui_memory_editor", {public = true})

target("imgui-knobs")
	set_kind("static")
	set_languages("c++20")
	add_files("./imgui-knobs/imgui-knobs.cpp")
	add_includedirs("./imgui-knobs", {public = true})
	add_packages("imgui")

target("termcolor")
	set_kind("headeronly")
	add_includedirs("./termcolor/include", {public = true})

target("main")
	set_kind("binary")
	add_includedirs("src/")
	add_files("src/*.cc", "src/*.cpp")
	add_packages("cmdline", "sol2", "lua", "nlohmann_json", "tinyexpr")
	add_packages("imgui", "stb", "freetype", "serial")
	add_deps("termcolor")
	add_deps("imgui-file-dialog")
	add_deps("imgui_memory_editor")
	add_deps("imgui-knobs")
	set_languages("c++20")
	set_encodings("utf-8")
	-- set_warnings("everything")
	set_rundir("$(projectdir)")

includes("./test")
