// Dear ImGui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
// #define GL_CLAMP_TO_EDGE 0x812F
// #define IMGUI_IMPL_OPENGL_ES2
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "ImGuiFileDialogConfig.h"
#include "ImGuiFileDialog.h"
#include <cstdio>
#include <array>
#include <iostream>
#include <string>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#undef GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif








// Simple helper function to load an image into a OpenGL texture with common settings
bool LoadTextureFromFile(const char* filename, GLuint* out_texture, int* out_width, int* out_height) {
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
	*out_width = image_width;
	*out_height = image_height;

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


::std::array<float, 100> wave;


static void glfw_error_callback(int error, const char* description) {
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

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
	GLFWwindow* window = glfwCreateWindow(1280, 720, title.c_str(), NULL, NULL);
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

	int my_image_width = 0;
	int my_image_height = 0;
	GLuint my_image_texture = 0;
	bool ret = LoadTextureFromFile("./res/MyImage01.jpg", &my_image_texture, &my_image_width, &my_image_height);
	IM_ASSERT(ret);

	// ::std::string image_name = "MyImage01.jpg";
	// unsigned char* pic = stbi_load(image_name.c_str(), &my_image_width, &my_image_height, NULL, 4);

	// gen_code_from_image_sprites("./normal_keys.png", MODE::BIT1, 16, 16);

	int image_width, image_height, comp;
	GLuint image_texture = 0;

	bool show_imgui_file_dialog = false;

	// Main loop
	for (;!glfwWindowShouldClose(window);
		[&]() -> void {
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
		// if (show_demo_window)
		//     ImGui::ShowDemoWindow(&show_demo_window);
		ImGui::Begin("图片使用范例");
		ImGui::Text("pointer = %p", my_image_texture);
		ImGui::Text("size = %d x %d", my_image_width, my_image_height);
		auto frame_size = ImGui::GetContentRegionMax();
		if (frame_size.x > frame_size.y) {
			frame_size.x = frame_size.y * my_image_width / my_image_height;
		} else {
			frame_size.y = frame_size.x * my_image_height / my_image_width;
		}
		ImGui::Image((void*)(intptr_t)my_image_texture, frame_size);
		for (int n = 0; n < 100; n++)
			wave[n] = sinf(n * 0.2f + ImGui::GetTime() * 1.5f);
		ImGui::PlotLines("正弦波", wave.data(), 100);
		ImGui::End();

		ImGui::Begin("图片转代码");

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

					bool ret = LoadTextureFromFile(filePathName.c_str(), &my_image_texture, &my_image_width, &my_image_height);
					IM_ASSERT(ret);
				}
			}
		}
		ImGui::EndFrame();
		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
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
