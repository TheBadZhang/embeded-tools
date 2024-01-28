#define SHOW_CONSOLE
#include "graphics.h"
#include <iostream>

#include "microui_impl_ege.h"

#define HORIZONTAL 0
#define VERTICAL 1
#define ORITENTATION HORIZONTAL

uint8_t rgb2bit(color_t c) {
	return (0.299*EGEGET_R(c)+0.587*EGEGET_G(c)+0.114*EGEGET_B(c)) > 128 ? 1 : 0;
}



void gen_code_from_image(std::string path) {

	PIMAGE pic = newimage ();
	getimage (pic, path.c_str());
	putimage (0, 0, pic);
	printf("const uint8_t name[] = {\n0x%02x,0x%02x,", getwidth(pic), getheight(pic));
	printf("    // %d x %d\n", getwidth (pic), getheight (pic));
	#if ORITENTATION == VERTICAL
	for (int j = 0; j < getheight (pic) / 8; j++) {
		for (int i = 0; i < getwidth (pic); i++) {
	#elif ORITENTATION == HORIZONTAL
	for (int j = 0; j < getheight (pic); j++) {
		// printf("\t");
		for (int i = 0; i <= (getwidth (pic)-1) / 8; i++) {
	#endif
			// if (i % 6 == 0) {
			// 	printf ("\n");
			// 	// printf("0x00,");
			// 	continue;
			// }
			unsigned char pixel = 0;
			for (int k = 0; k < 8; k++) {
	#if ORITENTATION == VERTICAL
				pixel |= (getpixel (i, j * 8 + k, pic) & 0x01) << (7 - k);
	#elif ORITENTATION == HORIZONTAL
				pixel |= (rgb2bit(getpixel (i * 8 + k, j, pic)) & 0x01) << (k);      // 低位在前
				// pixel |= (getpixel (i * 8 + k, j, pic) & 0x01) << (7-k);    // 高位在前
	#endif
			}
			printf ("0x%02x,", (pixel) & 0xff);
			// std::cout << std::hex << (int)pixel << ",";

	#if ORITENTATION == VERTICAL
			if (i % 8 == 7) {
				puts("");
			}
	#elif ORITENTATION == HORIZONTAL
	#endif
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

	// 初始化 microui
	mu_Context ctx_c;
	mu_Context *ctx = &ctx_c;
	mu_init(ctx);

	// 适配 EGE 平台
	microui_impl_ege_init(ctx);

	/* main loop */
	while (ege::is_run()) {
		/* handle events */
		microui_impl_ege_process_events(ctx);

		/* process frame */
		process_frame(ctx);

		/* render */
		ege::setbkcolor_f(EGERGBA((int)bg[0], (int)bg[1], (int)bg[2], (int)bg[3]));
		ege::cleardevice();
		microui_impl_ege_draw_data(ctx);

		ege::delay_ms(0);
	}

	microui_impl_ege_shutdown();
	ege::closegraph();
	return 0;
}
