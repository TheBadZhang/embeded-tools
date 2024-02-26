#pragma once

#include "common.hpp"

class Image2Code {
private:
// public:
	::std::string filePathName;
	::std::string filePath;
	ImGuiIO* io;

	GLuint image_texture = 0;

	ImVec2 my_image = ImVec2(0, 0);
	GLuint my_image_texture = 0;
	bool show_imgui_file_dialog = false;

	void picture_preview (void);

	void open_picture(void);

	int item = 0;
	struct {
		int width = -1;
		int height = -1;
		int offset_x;
		int offset_y;
	} sprite;
	struct {
		int width;
		int height;
		int brightness;
		int contrast;
	} image_adjust;

	float transparent_colot[3] = { 1.0f, 0.0f, 0.2f };
	bool is_sprite = false;
	struct {
		int width;
		int height;
	} sprite_image;
	struct {
		int r, g, b;
	} color;
	int scan_mode = 0;
	int msb_lsb = 0;
	int endian = 0;
	int selected_effect = 0;

	enum class MODE {
		BIT1, BIT4, BIT8, BIT16
	} mode;

	uint8_t rgb2bit(uint32_t c) {
		uint8_t r = c & 0xff;
		uint8_t g = (c >> 8) & 0xff;
		uint8_t b = (c >> 16) & 0xff;

		uint8_t gray = 0.299*r + 0.587*g + 0.114*b;

		return gray;
	}
	uint32_t getpixel (int x, int y, uint8_t* pic, int width, int height) {
		return *(uint32_t*)(pic + (x + y * width) * 4);
	}

	void gen_code_from_image_sprites(std::string path);


public:
	Image2Code() {
		filePathName.resize(1024);
		filePath.resize(1024);
	}
	~Image2Code() {

	}
	void setIO(ImGuiIO* io) {
		this->io = io;
	}
	void ui(void);

};