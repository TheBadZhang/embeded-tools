#pragma once
#ifndef __TBZ_COMMON_HPP__
#define __TBZ_COMMON_HPP__
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_STATIC
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
#include <cstdint>
#include <algorithm>
#include <format>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#undef GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h> // Will drag system OpenGL headers


bool LoadTextureFromFile(const char* filename, GLuint* out_texture, ImVec2& out_image_size);



#endif // __TBZ_COMMON_HPP__