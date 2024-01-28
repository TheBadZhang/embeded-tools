#define SHOW_CONSOLE
#include "graphics.h"
#include <iostream>

uint8_t rgb2bit(color_t c) {
	return (0.299*EGEGET_R(c)+0.587*EGEGET_G(c)+0.114*EGEGET_B(c)) > 128 ? 1 : 0;
}

void gen_code_from_image(std::string path) {

	PIMAGE pic = newimage ();
	getimage (pic, path.c_str());
	putimage (0, 0, pic);
	printf("const uint8_t name[] = {\n0x%02x,0x%02x,", getwidth(pic), getheight(pic));
	printf("    // %d x %d\n", getwidth (pic), getheight (pic));
	// for (int j = 0; j < getheight (pic) / 8; j++) {
	// 	for (int i = 0; i < getwidth (pic); i++) {
	for (int j = 0; j < getheight (pic); j++) {
		// printf("\t");
		for (int i = 0; i <= (getwidth (pic)-1) / 8; i++) {
			// if (i % 6 == 0) {
			// 	printf ("\n");
			// 	// printf("0x00,");
			// 	continue;
			// }
			unsigned char pixel = 0;
			for (int k = 0; k < 8; k++) {
				// pixel |= (getpixel (i, j * 8 + k, pic) & 0x01) << (7 - k);
				pixel |= (rgb2bit(getpixel (i * 8 + k, j, pic)) & 0x01) << (k);      // 低位在前
				// pixel |= (getpixel (i * 8 + k, j, pic) & 0x01) << (7-k);    // 高位在前
			}
			printf ("0x%02x,", (pixel) & 0xff);
			// std::cout << std::hex << (int)pixel << ",";

			// if (i % 8 == 7) {
			// 	puts("");
			// }
		}
		printf("\n");
	}
	puts("};");
}

int main (int argc, char* argv[]) {
	// 手动刷新模式
	ege::setinitmode (INIT_RENDERMANUAL);
	// 界面分辨率
	ege::initgraph (800, 600);
	ege::setbkmode (TRANSPARENT);

	gen_code_from_image("./res/psychic-swamp-v2_1.png");
	getch();
	ege::closegraph();
	return 0;
}
