#include "imgui.h"
#include "imgui-sfml.h"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

int main()
{
	sf::RenderWindow window(sf::VideoMode(640, 480), "");
	window.setVerticalSyncEnabled(true);
	ImGui::SFML::Init(window);

	sf::Color bgColor;
	float color[3] = { 0.f, 0.f, 0.f };

	// ����� �� ����� ������������ ������ char. ����� ������������
	// std::string ����� ������� ��������, ��������� �� ������ �����
	char windowTitle[255] = "ImGui + SFML = <3";
	window.setTitle(windowTitle);

	sf::Clock deltaClock;
	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			ImGui::SFML::ProcessEvent(event);

			if (event.type == sf::Event::Closed) {
				window.close();
			}
		}

		ImGui::SFML::Update(window, deltaClock.restart());

		ImGui::Begin("Sample window"); // ������ ����

		// ���������� ������ �����
		if (ImGui::ColorEdit3("Background color", color)) {
			// ��� ���������� ��� ��������� ��������, ������� ��
			// ����������� �������������
			bgColor.r = static_cast<sf::Uint8>(color[0] * 255.f);
			bgColor.g = static_cast<sf::Uint8>(color[1] * 255.f);
			bgColor.b = static_cast<sf::Uint8>(color[2] * 255.f);
		}

		ImGui::InputText("Window title", windowTitle, 255);

		if (ImGui::Button("Update window title")) {
			// ���� ��� �����������, ����� ���� ��� �� ������
			// ����� ����� ���� �� �������� 
			// if(ImGui::InputText(...))
			window.setTitle(windowTitle);
		}
		ImGui::End(); // end window

		window.clear(bgColor); // ��������� ���� �������� ������
		ImGui::SFML::Render(window);
		window.display();
	}

	ImGui::SFML::Shutdown();
}