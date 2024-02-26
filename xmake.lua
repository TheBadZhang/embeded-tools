add_rules("mode.debug", "mode.release")
add_requires("imgui", {configs = {glfw_opengl3 = true}})
-- add_requires("imgui-file-dialog")
add_requires("stb")
add_requires("freetype")

if is_plat("windows") then
	add_defines("__STDC_LIMIT_MACROS")
end

target("imgui-file-dialog")
	set_kind("static")
	set_languages("c++20")
	add_files("./3rd/imgui-file-dialog/ImGuiFileDialog.cpp")
	add_includedirs("./3rd/imgui-file-dialog/", {public = true})
	add_packages("imgui")
	if is_plat("windows") and is_kind("shared") then
		add_rules("utils.symbols.export_all", {export_classes = true})
	end

target("main")
	set_kind("binary")
	add_includedirs("src/")
	add_files("src/main.cc")
	add_packages("imgui", "stb", "freetype")
	add_deps("imgui-file-dialog")
	set_languages("c++23")
	set_encodings("utf-8")
	-- set_warnings("everything")
	set_rundir("$(projectdir)")
