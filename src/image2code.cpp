#include "image2code.hpp"


void Image2Code::gen_code_from_image_sprites(std::string path) {

	int pic_width, pic_height, comp;
	uint8_t* pic = stbi_load(path.c_str(), &pic_width, &pic_height, &comp, 4);

	if (sprite.width == -1) sprite.width = pic_width;
	if (sprite.height == -1) sprite.height = pic_height;

	// 数组头
	printf("const uint8_t name[] = {\n0x%02x,0x%02x,", pic_width, pic_height);
	printf("    // %d x %d\n", pic_width, pic_height);

	int blen;
	switch (mode) {
		case MODE::BIT1: {
			blen = (sprite.width + 7) / 8;
		} break;
		case MODE::BIT4: {
			blen = (sprite.width + 1) / 2;
		} break;
		case MODE::BIT8: {
			blen = sprite.width;
		} break;
		case MODE::BIT16: {
			blen = sprite.width * 2;
		} break;
	}
	// 输出的行数

	// 当精灵图大小和图片大小完全一致的时候，相当于上面那个函数的功能了
	// 有多少行精灵图
	for (int i = 0; i < pic_height/sprite.height; i++) {
		// 一行对应多少精灵图
		for (int j = 0; j < pic_width/sprite.width; j++) {
			// 一个精灵图的行
			for (int k = 0; k < sprite.height; k++) {
				for (int l = 0; l < blen; l++) {
					unsigned char pixel = 0;
					switch (mode) {
						case MODE::BIT1: {
							// 每个字节可以表达8个像素点
							for (int m = 0; m < 8; m++) {
								// 对像素点求灰度值后再计算
								int sprite_relative_x = l * 8 + m;
								if (sprite_relative_x >= sprite.width) break;
								int x = j * sprite.width + sprite_relative_x;
								int y = i * sprite.height + k;
								uint8_t c = rgb2bit(getpixel (x, y, pic, pic_width, pic_height));
								// putpixel (x+300, y+300, EGERGB(c,c,c));
								pixel |= (c > 127 ? 0x01 : 0x00) << (m);      // 低位在前
								// pixel |= (getpixel (i * 8 + k, j, pic) & 0x01) << (7-k);    // 高位在前
							}
						} break;
						case MODE::BIT4: {
							for (int m = 0; m < 2; m++) {
								int sprite_relative_x = l * 2 + m;
								if (sprite_relative_x >= sprite.width) break;
								int x = j * sprite.width + sprite_relative_x;
								int y = i * sprite.height + k;
								uint8_t c = rgb2bit(getpixel (x, y, pic, pic_width, pic_height)) >> 4;
								if (c == 0x01) c = 0x00;
								pixel |= (c) << ((1-m) * 4);      // 低位在前
							}
						} break;
						case MODE::BIT8: {

						} break;
						case MODE::BIT16: {

						} break;
						default: return;
					}
					printf ("0x%02x,", (pixel) & 0xff);
				}
			}
			printf("\n");
		}
	}
	puts("};");
}


void Image2Code::picture_preview (void) {

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

void Image2Code::open_picture(void) {

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
				filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
				filePath = ImGuiFileDialog::Instance()->GetCurrentPath();

				bool ret = LoadTextureFromFile(filePathName.c_str(), &my_image_texture, my_image);
				IM_ASSERT(ret);
			}
		}
	}
}

void Image2Code::ui(void) {
	picture_preview();
	::ImGui::Begin("图片转换工具");

	::ImGui::BeginGroup();
	::ImGui::SeparatorText("文件路径");
	::ImGui::InputText("", filePathName.data(), 1024);
	::ImGui::SameLine();
	open_picture();
	::ImGui::EndGroup();

	::ImGui::BeginGroup();
	::ImGui::SeparatorText("文件夹路径");
	::ImGui::InputText("", filePath.data(), 1024);
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
		::ImGui::Combo("图像格式", &item, items, IM_ARRAYSIZE(items));
		::ImGui::EndGroup();
	}
	{
		::ImGui::BeginGroup();
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
		::ImGui::SeparatorText("位优先");
		::ImGui::RadioButton("MSB", &msb_lsb, 0);
		::ImGui::SameLine();
		::ImGui::RadioButton("LSB", &msb_lsb, 1);
		::ImGui::EndGroup();
	}
	{
		::ImGui::BeginGroup();
		::ImGui::SeparatorText("大小端");
		::ImGui::RadioButton("大端", &endian, 0);
		::ImGui::SameLine();
		::ImGui::RadioButton("小端", &endian, 1);
		::ImGui::EndGroup();
	}

	{
		::ImGui::BeginGroup();
		::ImGui::Checkbox("是否为精灵图", &is_sprite);
		if (is_sprite) {
			::ImGui::SeparatorText("预览");
			::ImGui::SliderInt("精灵图宽度", &sprite.width, 1, my_image.x);
			::ImGui::SliderInt("精灵图高度", &sprite.height, 1, my_image.y);
			::ImGui::SliderInt("offset-x", &sprite.offset_x, 0, my_image.x-sprite.width);
			::ImGui::SliderInt("offset-y", &sprite.offset_y, 0, my_image.y-sprite.height);
		}
		::ImGui::EndGroup();
	}
	{
		::ImGui::BeginGroup();
		::ImGui::SeparatorText("调整");
		::ImGui::SliderInt("宽度", &image_adjust.width, 1, 1024);
		::ImGui::SliderInt("高度", &image_adjust.height, 1, 1024);
		::ImGui::SliderInt("亮度", &image_adjust.brightness, 1, 1024);
		::ImGui::SliderInt("对比度", &image_adjust.contrast, 1, 1024);
		::ImGui::EndGroup();
	}
	{
		::ImGui::BeginGroup();
		::ImGui::SeparatorText("透明色");
		ImGui::ColorEdit3("颜色", transparent_colot);
		::ImGui::EndGroup();
	}
	{
		::ImGui::BeginGroup();
		// ::ImGui::Text("效果");
		::ImGui::SeparatorText("效果");
		{
			::ImGui::BeginGroup();
			::ImGui::RadioButton("反色", &selected_effect, 0);
			::ImGui::RadioButton("灰度", &selected_effect, 1);
			::ImGui::RadioButton("二值化", &selected_effect, 2);
			::ImGui::RadioButton("颜色抖动", &selected_effect, 3);
			// ::ImGui::RadioButton("模糊", &selected_effect, 5);
			::ImGui::EndGroup();
		}
		::ImGui::EndGroup();
	}

	::ImGui::SeparatorText("预览");


	::ImGui::End();
}