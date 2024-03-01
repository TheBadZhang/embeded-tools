
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
