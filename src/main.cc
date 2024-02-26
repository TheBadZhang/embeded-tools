// Dear ImGui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs
#include <imgui.h>
#include "common.hpp"

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif



#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftpfr.h>
#include <freetype/ftadvanc.h>

#include <cstdint>

/* 字体数据（ttf） */
typedef struct _ft_fontinfo {
	FT_Face    face;     /* FreeType库句柄对象 */
	FT_Library library;  /* 外观对象（描述了特定字样和风格，比如斜体风格等） */
	int32_t     mono;    /* 是否为二值化模式 */
} ft_fontinfo;

/* 字模格式常量定义 */
typedef enum _glyph_format_t {
	GLYPH_FMT_ALPHA, /* 每个像素占用1个字节 */
	GLYPH_FMT_MONO,  /* 每个像素占用1个比特 */
} glyph_format_t;

/* 字模（位图） */
typedef struct _glyph_t {
	int16_t  x;
	int16_t  y;
	uint16_t w;
	uint16_t h;
	uint16_t advance;  /* 占位宽度 */
	uint8_t  format;   /* 字模格式 */
	uint8_t  pitch;    /* 跨距（每行像素个数 * 单个像素所占字节数） */
	uint8_t  *data;    /* 字模数据：每个像素点占用一个字节 */
	void     *handle;  /* 保存需要释放的句柄 */
} glyph_t;


/* 获取二值化位图上像素点的值 */
uint8_t bitmap_mono_get_pixel(const uint8_t* buff, uint32_t w, uint32_t h, uint32_t x, uint32_t y) {
	/* 计算字节偏移 */
	uint32_t line_length = ((w + 15) >> 4) << 1;
	uint32_t offset = y * line_length + (x >> 3);

	/* 计算位偏移 */
	uint32_t offset_bit = 7 - (x % 8);

	const uint8_t* data = buff + offset;
	if (buff == NULL || (x > w && y > h))
		return 0;
	return (*data >> offset_bit) & 0x1;
}

/* 获取字模 */
static int font_ft_get_glyph(ft_fontinfo *font_info, wchar_t c, float font_size, glyph_t* g) {
	FT_Glyph glyph;
	FT_GlyphSlot glyf;
	FT_Int32 flags = FT_LOAD_DEFAULT | FT_LOAD_RENDER | FT_LOAD_NO_BITMAP;

	if (font_info->mono) {
		flags |= FT_LOAD_TARGET_MONO;
	}
	/* 设置字体大小 */
	FT_Set_Char_Size(font_info->face, 0, font_size * 64, 0, 72);
	// FT_Set_Pixel_Sizes(font_info->face, 0, font_size);

	/* 通过编码加载字形并将其转化为位图（保存在face->glyph->bitmap中） */
	if (!FT_Load_Char(font_info->face, c, flags)) {
		glyf = font_info->face->glyph;
		FT_Get_Glyph(glyf, &glyph);
		// FT_Render_Glyph(glyf, FT_RENDER_MODE_NORMAL);

		g->format = GLYPH_FMT_ALPHA;
		g->h = glyf->bitmap.rows;
		g->w = glyf->bitmap.width;
		g->pitch = glyf->bitmap.pitch;
		g->x = glyf->bitmap_left;
		g->y = -glyf->bitmap_top;
		g->data = glyf->bitmap.buffer;
		g->advance = glyf->metrics.horiAdvance / 64;

		if (g->data != NULL) {
			if (glyf->bitmap.pixel_mode == FT_PIXEL_MODE_MONO) {
				g->format = GLYPH_FMT_MONO;
			}
			g->handle = glyph;
		}
		else {
			FT_Done_Glyph(glyph);
		}
	}
	return g->data != NULL || c == ' ' ? 1 : 0;
}



	ft_fontinfo   font_info;         /* 字库信息 */
	long int      size = 0;          /* 字库文件大小 */
	unsigned char *font_buf = NULL;  /* 字库文件数据 */

int load_font(std::string path) {

	/* 加载字库文件存入font_buf */
	FILE *font_file = fopen(path.c_str(), "rb");
	if (font_file == NULL) {
		printf("Can not open font file!\n");
		getchar();
		return 0;
	}
	fseek(font_file, 0, SEEK_END); /* 设置文件指针到文件尾，基于文件尾偏移0字节 */
	size = ftell(font_file);       /* 获取文件大小（文件尾 - 文件头  单位：字节） */
	fseek(font_file, 0, SEEK_SET); /* 重新设置文件指针到文件头 */

	font_buf = (unsigned char*)calloc(size, sizeof(unsigned char));
	fread(font_buf, size, 1, font_file);
	fclose(font_file);

	font_info.mono = 1;  /* 设置为二值化模式 */

	/* 初始化FreeType */
	FT_Init_FreeType(&(font_info.library));

	/* 从font_buf中提取外观 */
	FT_New_Memory_Face(font_info.library, font_buf, size, 0, &(font_info.face));

	/* 设置字体编码方式为UNICODE */
	FT_Select_Charmap(font_info.face, FT_ENCODING_UNICODE);

}
int drawFreeType_char(int x, int y, wchar_t ch) {
	glyph_t g;
	// wchar_t c = L'a';
	float   font_size = 18;  /* 设置字体大小 */
	font_ft_get_glyph(&font_info, ch, font_size, &g);  /* 获取字模 */

/* 打印字模信息 */
	int i = 0, j = 0;
	if (g.format == GLYPH_FMT_MONO) {
		for (j = 0; j < g.h; ++j) {
			for (i = 0; i < g.w; ++i) {
				uint8_t pixel = bitmap_mono_get_pixel(g.data, g.w, g.h, i, j);
				// putpixel(x+i, y+g.y+j, pixel ? (EGERGB(255,255,255)) : (EGERGB(0,0,0)));
				// if (pixel) screen_pic.drawPixel(x+i, y+g.y+j);
			}
		}
	} else if (g.format == GLYPH_FMT_ALPHA) {
		for (j = 0; j < g.h; ++j) {
			for (i = 0; i < g.w; ++i) {
				uint8_t pixel = g.data[j*g.w + i];
				// putpixel (x+i, y+g.y+j, EGERGB(pixel, pixel, pixel));
			}
		}
	}

	return g.w;
}
/*
void drawFreeType_str(int x, int y, std::span<wchar_t> strw) {
	int w = 0;
	for (int i = 0; i < strw.size(); i++) {
		if (x > 128-w) {
			x = 0;
			y += 16 + 2;
		}
		w = drawFreeType_char(x, y, strw[i]) + 1;
		x += w;
	}
}
*/

// Simple helper function to load an image into a OpenGL texture with common settings
bool LoadTextureFromFile(const char* filename, GLuint* out_texture, ImVec2& out_image_size) {
	// Load from file
	int image_width = 0;
	int image_height = 0;
	unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
	if (image_data == NULL)
		return false;

	// Create a OpenGL texture identifier
	GLuint image_texture;
	glGenTextures(1, &image_texture);
	glBindTexture(GL_TEXTURE_2D, image_texture);

	// Setup filtering parameters for display
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

	// Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
	stbi_image_free(image_data);

	*out_texture = image_texture;
	out_image_size = ImVec2((float)image_width, (float)image_height);

	return true;
}



#define GET_R(c) ((c) & 0xff)
#define GET_G(c) (((c) >> 8) & 0xff)
#define GET_B(c) (((c) >> 16) & 0xff)
uint8_t rgb2bit(uint32_t c) {
	return (0.299*GET_R(c)+0.587*GET_G(c)+0.114*GET_B(c));
}

enum class MODE {
	BIT1, BIT4, BIT8, BIT16
};

uint32_t getpixel (int x, int y, uint8_t* pic, int width, int height) {
	return *(uint32_t*)(pic + (x + y * width) * 4);
}

void gen_code_from_image(std::string path, MODE mode) {

	int pic_width, pic_height, comp;
	uint8_t* pic = stbi_load(path.c_str(), &pic_width, &pic_height, &comp, 4);


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
						uint8_t c = rgb2bit(getpixel (i * 8 + k, j, pic, pic_width, pic_height));
						pixel |= (c > 127 ? 0x01 : 0x00) << (k);      // 低位在前
						// pixel |= (getpixel (i * 8 + k, j, pic) & 0x01) << (7-k);    // 高位在前
					}
				} break;
				case MODE::BIT4: {
					for (int k = 0; k < 2; k++) {
						uint8_t c = rgb2bit(getpixel (i * 2 + k, j, pic, pic_width, pic_height)) >> 4;
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

	int pic_width, pic_height, comp;
	uint8_t* pic = stbi_load(path.c_str(), &pic_width, &pic_height, &comp, 4);

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
								uint8_t c = rgb2bit(getpixel (x, y, pic, pic_width, pic_height));
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
								uint8_t c = rgb2bit(getpixel (x, y, pic, pic_width, pic_height)) >> 4;
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



static void glfw_error_callback(int error, const char* description) {
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

#include "image2code.hpp"

int main(int, char**) {
	// Setup window
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit())
		return 1;

	// Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
	// GL ES 2.0 + GLSL 100
	const char* glsl_version = "#version 100";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
	// GL 3.2 + GLSL 150
	const char* glsl_version = "#version 150";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
	// GL 3.0 + GLSL 130
	const char* glsl_version = "#version 130";
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	//glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

	// Create window with graphics context
	::std::string title = "image2code";
	GLFWwindow* window = glfwCreateWindow(1920, 1080, title.c_str(), NULL, NULL);
	if (window == NULL)
		return 1;
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	// io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	// io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);



	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Load Fonts
	// - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
	// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
	// - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
	// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
	// - Read 'docs/FONTS.md' for more instructions and details.
	float size_in_pixels = 36.0f;
	ImFont* font = io.Fonts->AddFontFromFileTTF("./res/wqy16.ttf", size_in_pixels, NULL, io.Fonts->GetGlyphRangesChineseFull());  // Load Japanese characters
	IM_ASSERT(font != NULL);

	// Our state
	bool show_demo_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	// ::std::string image_name = "MyImage01.jpg";
	// unsigned char* pic = stbi_load(image_name.c_str(), &my_image_width, &my_image.y, NULL, 4);

	// gen_code_from_image_sprites("./normal_keys.png", MODE::BIT1, 16, 16);

	image2code.setIO(&io);

	// Main loop
	for (;!glfwWindowShouldClose(window);
		[&]() -> void {
			ImGui::EndFrame();
			// Rendering
			ImGui::Render();
			int display_w, display_h;
			glfwGetFramebufferSize(window, &display_w, &display_h);
			glViewport(0, 0, display_w, display_h);
			glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
			glClear(GL_COLOR_BUFFER_BIT);
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			glfwSwapBuffers(window);
		}()) {
		// Poll and handle events (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		glfwPollEvents();

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
		if (show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);




		image2code.ui();

		::ImGui::Begin("图表测试");
		::std::array<float, 100> wave;
		for (int n = 0; n < 100; n++)
			wave[n] = sinf(n * 0.2f + ::ImGui::GetTime() * 1.5f);
		::ImGui::PlotLines("正弦波", wave.data(), 100);
		::ImGui::End();

		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
		if (false)
		{
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

			ImGui::Text("这是一段有用的文本");               // Display some text (you can use a format strings too)
			ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
			ImGui::Checkbox("Another Window", &show_another_window);

			ImGui::SliderFloat("浮点数", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
			ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

			if (ImGui::Button("按我"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
				counter++;
			ImGui::SameLine();
			ImGui::Text("counter = %d", counter);
			ImGui::Text("应用平均 %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}

		// 3. Show another simple window.
		if (show_another_window)
		{
			ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
			ImGui::Text("Hello from another window!");
			if (ImGui::Button("Close Me"))
				show_another_window = false;
			ImGui::End();
		}

	}

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
