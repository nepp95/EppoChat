#include "AppLayer.h"

#include <imgui/imgui.h>

void AppLayer::OnAttach()
{
}

void AppLayer::OnDetach()
{

}

void AppLayer::OnEvent(Event& e)
{

}

void AppLayer::OnUpdate(float timestep)
{

}

void AppLayer::OnUIRender()
{
	ImGui::ShowDemoWindow();
}
