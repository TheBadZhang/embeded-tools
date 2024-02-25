#define SHOW_CONSOLE
#include "graphics.h"
#include <iostream>

uint8_t rgb2bit(color_t c) {
	return (0.299*EGEGET_R(c)+0.587*EGEGET_G(c)+0.114*EGEGET_B(c));
}

enum class MODE {
	BIT1, BIT4, BIT8, BIT16
};

void gen_code_from_image(std::string path, MODE mode) {

	ege::PIMAGE pic = ege::newimage ();
	ege::getimage (pic, path.c_str());
	ege::putimage (0, 0, pic);
	int pic_width  = ege::getwidth  (pic);
	int pic_height = ege::getheight (pic);


	// 数组头
	printf("const uint8_t name[] = {\n0x%02x,0x%02x,", pic_width, pic_height);
	printf("    // %d x %d\n", pic_width, pic_height);

	int blen;
	switch (mode) {
		case MODE::BIT1: {
			blen = (pic_width + 7) / 8;
		} break;
		case MODE::BIT4: {
			blen = (pic_width + 1) / 2;
		} break;
		case MODE::BIT8: {
			blen = pic_width;
		} break;
		case MODE::BIT16: {
			blen = pic_width * 2;
		} break;
	}
	// 输出的行数
	// 一行一行处理
	for (int j = 0; j < pic_height; j++) {
		// 一行对应多少字节
		for (int i = 0; i < blen; i++) {
			unsigned char pixel = 0;
			switch (mode) {
				case MODE::BIT1: {
					// 每个字节可以表达8个像素点
					for (int k = 0; k < 8; k++) {
						// 对像素点求灰度值后再计算
						uint8_t c = rgb2bit(getpixel (i * 8 + k, j, pic));
						pixel |= (c > 127 ? 0x01 : 0x00) << (k);      // 低位在前
						// pixel |= (getpixel (i * 8 + k, j, pic) & 0x01) << (7-k);    // 高位在前
					}
				} break;
				case MODE::BIT4: {
					for (int k = 0; k < 2; k++) {
						uint8_t c = rgb2bit(getpixel (i * 2 + k, j, pic)) >> 4;
						if (c == 0x01) c = 0x00;
						pixel |= (c) << ((1-k) * 4);      // 低位在前
					}
				} break;
			}
			printf ("0x%02x,", (pixel) & 0xff);
			// std::cout << std::hex << (int)pixel << ",";

		}
		printf("\n");
	}
	puts("};");
}

void gen_code_from_image_sprites(std::string path, MODE mode, int sprite_width, int sprite_height) {

	ege::PIMAGE pic = ege::newimage ();
	ege::getimage (pic, path.c_str());
	ege::putimage (0, 0, pic);
	int pic_width  = ege::getwidth  (pic);
	int pic_height = ege::getheight (pic);


	// 数组头
	printf("const uint8_t name[] = {\n0x%02x,0x%02x,", pic_width, pic_height);
	printf("    // %d x %d\n", pic_width, pic_height);

	int blen;
	switch (mode) {
		case MODE::BIT1: {
			blen = (sprite_width + 7) / 8;
		} break;
		case MODE::BIT4: {
			blen = (sprite_width + 1) / 2;
		} break;
		case MODE::BIT8: {
			blen = sprite_width;
		} break;
		case MODE::BIT16: {
			blen = sprite_width * 2;
		} break;
	}
	// 输出的行数

	// 当精灵图大小和图片大小完全一致的时候，相当于上面那个函数的功能了
	// 有多少行精灵图
	for (int i = 0; i < pic_height/sprite_height; i++) {
		// 一行对应多少精灵图
		for (int j = 0; j < pic_width/sprite_width; j++) {
			// 一个精灵图的行
			for (int k = 0; k < sprite_height; k++) {
				for (int l = 0; l < blen; l++) {
					unsigned char pixel = 0;
					switch (mode) {
						case MODE::BIT1: {
							// 每个字节可以表达8个像素点
							for (int m = 0; m < 8; m++) {
								// 对像素点求灰度值后再计算
								int sprite_relative_x = l * 8 + m;
								if (sprite_relative_x >= sprite_width) break;
								int x = j * sprite_width + sprite_relative_x;
								int y = i * sprite_height + k;
								uint8_t c = rgb2bit(getpixel (x, y, pic));
								// putpixel (x+300, y+300, EGERGB(c,c,c));
								pixel |= (c > 127 ? 0x01 : 0x00) << (m);      // 低位在前
								// pixel |= (getpixel (i * 8 + k, j, pic) & 0x01) << (7-k);    // 高位在前
							}
						} break;
						case MODE::BIT4: {
							for (int m = 0; m < 2; m++) {
								int sprite_relative_x = l * 2 + m;
								if (sprite_relative_x >= sprite_width) break;
								int x = j * sprite_width + sprite_relative_x;
								int y = i * sprite_height + k;
								uint8_t c = rgb2bit(getpixel (x, y, pic)) >> 4;
								if (c == 0x01) c = 0x00;
								pixel |= (c) << ((1-m) * 4);      // 低位在前
							}
						} break;
					}
					printf ("0x%02x,", (pixel) & 0xff);
				}
			}
			printf("\n");
		}
	}
	puts("};");
}

int main (int argc, char* argv[]) {
	// 手动刷新模式
	ege::setinitmode (INIT_RENDERMANUAL);
	// 界面分辨率
	ege::initgraph (800, 600);
	ege::setbkmode (TRANSPARENT);
	// gen_code_from_image("./res/gray_image/tsetBones.png", MODE::BIT4);
	// gen_code_from_image("./res/gray_image/tsetLava.png", MODE::BIT4);
	// gen_code_from_image("./res/gray_image/tsetSand.png", MODE::BIT4);
	gen_code_from_image_sprites("./normal_keys.png", MODE::BIT1, 16, 16);
	std::cout << "Hello World";

	printf ("%d", 123123);
	// for (;ege::is_run();ege::delay_fps(60)) {
	// 	ege::cleardevice();

	// 	ege::setcolor(EGERGB(255, 255, 255));
	// 	ege::xyprintf(10, 10, "FPS: %.1f", ege::getfps());
	// }
	getch();
	ege::closegraph();
	return 0;
}
