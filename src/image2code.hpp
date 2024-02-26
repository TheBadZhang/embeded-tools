#pragma once

#include "common.hpp"

class Image2Code {
private:
// public:
	::std::string file_path;
	::std::string dir_path;
	ImGuiIO* io;

	int image_width, image_height, comp;
	GLuint image_texture = 0;

	ImVec2 my_image = ImVec2(0, 0);
	GLuint my_image_texture = 0;
	bool show_imgui_file_dialog = false;

	void picture_preview (void) {

		ImGui::Begin("图片预览");
		::ImGui::BeginGroup();

		static bool use_text_color_for_tint = false;
		ImVec2 pos = ImGui::GetCursorScreenPos();
		ImVec2 uv_min = ImVec2(0.0f, 0.0f);                 // Top-left
		ImVec2 uv_max = ImVec2(1.0f, 1.0f);                 // Lower-right
		ImVec4 tint_col = use_text_color_for_tint ? ImGui::GetStyleColorVec4(ImGuiCol_Text) : ImVec4(1.0f, 1.0f, 1.0f, 1.0f); // No tint
		ImVec4 border_col = ImGui::GetStyleColorVec4(ImGuiCol_Border);
		auto region = ImGui::GetContentRegionMax();
		if (region.x < my_image.x) {
			region.y = region.x * my_image.y / my_image.x;
		} else {
			region.x = my_image.x;
			region.y = my_image.y;
		}
		region.x = region.x;
		region.y = region.y;
		ImGui::Image((void*)(intptr_t)my_image_texture, region, uv_min, uv_max, tint_col, border_col);
		if (ImGui::BeginItemTooltip())
		{
			float region_sz = 32.0f;
			float region_x = io -> MousePos.x - pos.x - region_sz * 0.5f;
			float region_y = io -> MousePos.y - pos.y - region_sz * 0.5f;
			float zoom = 4.0f;
			if (region_x < 0.0f) { region_x = 0.0f; }
			else if (region_x > region.x - region_sz) { region_x = region.x - region_sz; }
			if (region_y < 0.0f) { region_y = 0.0f; }
			else if (region_y > region.y - region_sz) { region_y = region.y - region_sz; }
			ImGui::Text("Min: (%.2f, %.2f)", region_x, region_y);
			ImGui::Text("Max: (%.2f, %.2f)", region_x + region_sz, region_y + region_sz);
			ImVec2 uv0 = ImVec2((region_x) / region.x, (region_y) / region.y);
			ImVec2 uv1 = ImVec2((region_x + region_sz) / region.x, (region_y + region_sz) / region.y);
			ImGui::Image((void*)(intptr_t)my_image_texture, ImVec2(region_sz * zoom, region_sz * zoom), uv0, uv1, tint_col, border_col);
			ImGui::EndTooltip();
		}

		::ImGui::EndGroup();


		ImGui::End();

	}

	void open_picture(void) {

		if (not show_imgui_file_dialog) {
			if (ImGui::Button("打开图片选择器")) {
				IGFD::FileDialogConfig config;config.path = ".";
				show_imgui_file_dialog = true;
				::std::string filters = "Image files (*.png *.gif *.jpg *.jpeg){.png,.gif,.jpg,.jpeg}";
				ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "选择图片", filters.c_str(), config);
			}
		} else {
			if (ImGui::Button("关闭图片选择器")) {

				// action
				show_imgui_file_dialog = false;
				// close
				ImGuiFileDialog::Instance()->Close();
			}
		}

		if (show_imgui_file_dialog) {
			// display
			if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) {
				if (ImGuiFileDialog::Instance()->IsOk()) { // action if OK
					std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
					std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();

					bool ret = LoadTextureFromFile(filePathName.c_str(), &my_image_texture, my_image);
					IM_ASSERT(ret);
				}
			}
		}
	}

public:
	Image2Code() {
		file_path.resize(1024);
		dir_path.resize(1024);
	}
	~Image2Code() {

	}
	void setIO(ImGuiIO* io) {
		this->io = io;
	}
	void ui(void) {
		picture_preview();
		::ImGui::Begin("图片转换工具");

		::ImGui::BeginGroup();
		::ImGui::SeparatorText("文件路径");
		::ImGui::InputText("", file_path.data(), 1024);
		::ImGui::SameLine();
		open_picture();
		::ImGui::EndGroup();

		::ImGui::BeginGroup();
		::ImGui::SeparatorText("文件夹路径");
		::ImGui::InputText("", dir_path.data(), 1024);
		::ImGui::SameLine();
		::ImGui::Button("选择文件夹");
		::ImGui::EndGroup();
		{
			::ImGui::BeginGroup();
			::ImGui::SeparatorText("格式");
			const char* items[] = {
				"单色位图",
				"web 颜色"
			};
			static int item = 0;
			::ImGui::Combo("图像格式", &item, items, IM_ARRAYSIZE(items));
			::ImGui::EndGroup();
		}


		{
			::ImGui::BeginGroup();
			static int scan_mode = 0;
			::ImGui::SeparatorText("扫描方式");
			::ImGui::RadioButton("逐行", &scan_mode, 0);
			::ImGui::SameLine();
			::ImGui::RadioButton("逐列", &scan_mode, 1);
			::ImGui::SameLine();
			::ImGui::RadioButton("行列", &scan_mode, 2);
			::ImGui::SameLine();
			::ImGui::RadioButton("列行", &scan_mode, 3);
			::ImGui::EndGroup();
		}
		{
			::ImGui::BeginGroup();
			static int eeee = 0;
			::ImGui::SeparatorText("字节序");
			::ImGui::RadioButton("MSB", &eeee, 0);
			::ImGui::SameLine();
			::ImGui::RadioButton("LSB", &eeee, 1);
			::ImGui::EndGroup();
		}
		{
			::ImGui::BeginGroup();
			static int endian = 0;
			::ImGui::SeparatorText("大小端");
			::ImGui::RadioButton("大端", &endian, 0);
			::ImGui::SameLine();
			::ImGui::RadioButton("小端", &endian, 1);
			::ImGui::EndGroup();
		}

		{
			::ImGui::BeginGroup();
			static bool is_sprite = false;
			struct {
				int width;
				int height;
			} sprite_image;
			::ImGui::Checkbox("是否为精灵图", &is_sprite);
			if (is_sprite) {
				::ImGui::SeparatorText("预览");
				struct {
					int width;
					int height;
					int offset_x;
					int offset_y;
				} sprite;
				::ImGui::SliderInt("精灵图宽度", &sprite.width, 1, sprite_image.width);
				::ImGui::SliderInt("精灵图高度", &sprite.height, 1, sprite_image.height);
				::ImGui::SliderInt("offset-x", &sprite.offset_x, 0, sprite_image.width-sprite.width);
				::ImGui::SliderInt("offset-y", &sprite.offset_y, 0, sprite_image.height-sprite.height);
			}
			::ImGui::EndGroup();
		}
		::ImGui::BeginGroup();
		::ImGui::SeparatorText("调整");
		struct {
			int width;
			int height;
			int brightness;
			int contrast;
		} image_adjust;
		::ImGui::SliderInt("宽度", &image_adjust.width, 1, 1024);
		::ImGui::SliderInt("高度", &image_adjust.height, 1, 1024);
		::ImGui::SliderInt("亮度", &image_adjust.brightness, 1, 1024);
		::ImGui::SliderInt("对比度", &image_adjust.contrast, 1, 1024);
		::ImGui::EndGroup();

		::ImGui::BeginGroup();
		struct {
			int r, g, b;
		} color;
		::ImGui::SeparatorText("透明色");
		::ImGui::SliderInt("通道 R", &color.r, 0, 100, "%d", ImGuiSliderFlags_None);
		::ImGui::SliderInt("通道 G", &color.g, 0, 100, "%d", ImGuiSliderFlags_None);
		::ImGui::SliderInt("通道 B", &color.b, 0, 100, "%d", ImGuiSliderFlags_None);
		::ImGui::EndGroup();

		::ImGui::BeginGroup();
		// ::ImGui::Text("效果");
		::ImGui::SeparatorText("效果");
		::ImGui::BeginGroup();
		static int selected_effect = 0;
		::ImGui::RadioButton("反色", &selected_effect, 0);
		::ImGui::RadioButton("灰度", &selected_effect, 1);
		::ImGui::RadioButton("二值化", &selected_effect, 2);
		::ImGui::RadioButton("颜色抖动", &selected_effect, 3);
		// ::ImGui::RadioButton("模糊", &selected_effect, 5);
		::ImGui::EndGroup();
		::ImGui::EndGroup();


		::ImGui::SeparatorText("预览");



		::ImGui::End();
	}

} image2code;