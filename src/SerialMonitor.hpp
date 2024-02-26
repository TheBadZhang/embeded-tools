#pragma once

#include "common.hpp"
#include <imgui.h>
#include <serial/serial.h>
#include "coding_convert.hpp"

class SerialMonitor {
private:
	ImGuiIO* io;
	::std::vector<::serial::PortInfo> ports_listed;
public:
	SerialMonitor() {

		ports_listed = ::serial::list_ports();
	}
	~SerialMonitor() {}
	void setIO(ImGuiIO* io) {
		this->io = io;
	}
	void ui(void) {
		::ImGui::Begin("串口监视器");
		::ImGui::BeginGroup();
		::ImGui::SeparatorText("串口列表");
		::ImGui::BeginChild("串口列表", ImVec2(0, 0), true);
		for (auto& port : ports_listed) {
			// port.description = GbkToUtf8(port.description.c_str());
			::std::string com = ::std::format("com: {}\ndesc: {}\nhwid: {}\n", port.port, port.description, port.hardware_id);
			// ::std::cout << com << ::std::endl;
			::ImGui::Text(com.c_str());
		}
		::ImGui::EndChild();
		::ImGui::EndGroup();
		::ImGui::End();
	}
};