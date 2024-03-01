add_rules("mode.debug", "mode.release")
add_requires("imgui", {configs = {glfw_opengl3 = true}})
-- git 中的私有库
add_repositories("local-repo required_repo")
-- add_requires("imgui-file-dialog v0.6.7", {verify = false})

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
-- 跨平台程序命令解析库
add_requires("cli11 v2.4.1", {verify = false})
add_requires("sol2", "lua", "nlohmann_json", "tinyexpr")

if is_plat("windows") then
	add_defines("__STDC_LIMIT_MACROS")
end

-- windows上还没有得到支持
-- 但是 msvc 本身的链接速度已经足够可用
-- add_cxflags("-fuse-ld=mold")


-- git 库中的其他依赖
includes("./required_repo")
includes("./3rd")
target("main")
	set_kind("binary")
	add_includedirs("src/")
	add_files("src/*.cc", "src/*.cpp")
	add_packages("cli11")
	add_packages("sol2", "lua", "nlohmann_json", "tinyexpr")
	add_packages("imgui", "stb", "freetype", "serial")
	-- add_packages("imgui-file-dialog")
	add_deps("dithering")
	add_deps("imgui-file-dialog")
	add_deps("termcolor")
	add_deps("imgui_memory_editor")
	add_deps("imgui-knobs")
	set_languages("c++20")
	set_encodings("utf-8")
	if is_mode("release") then
		set_warnings("everything")
	end
	set_rundir("$(projectdir)")

-- 测试程序
includes("./test")
