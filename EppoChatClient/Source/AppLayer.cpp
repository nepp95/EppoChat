#include "AppLayer.h"

#include <EppoCore/Core/Application.h>
#include <EppoCore/Core/Buffer.h>

#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

#include "GLFW/glfw3.h"

ClientAppLayer* ClientAppLayer::s_Instance = nullptr;

namespace
{
    Buffer s_ScratchBuffer(1024);
}

ClientAppLayer::ClientAppLayer()
{
	EPPO_ASSERT(!s_Instance)
	s_Instance = this;
}

void ClientAppLayer::OnAttach()
{
	if (SteamDatagramErrMsg errMsg; !GameNetworkingSockets_Init(nullptr, errMsg))
        EPPO_ERROR("Failed to initialize GameNetworkingSockets: {}", errMsg);

    m_Socket = SteamNetworkingSockets();
}

void ClientAppLayer::OnDetach()
{
    s_ScratchBuffer.Release();
}

void ClientAppLayer::OnUpdate(float timestep)
{
	
}

void ClientAppLayer::OnUIRender()
{
	// From ImGui example
	static bool dockspaceOpen = true;
	static bool opt_fullscreen_persistant = true;
	const bool opt_fullscreen = opt_fullscreen_persistant;
	static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

	// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
	// because it would be confusing to have two docking targets within each others.
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
	if (opt_fullscreen)
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	}

	// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
	// and handle the pass-thru hole, so we ask Begin() to not render a background.
	if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
		window_flags |= ImGuiWindowFlags_NoBackground;

	// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
	// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
	// all active windows docked into it will lose their parent and become undocked.
	// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
	// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
	ImGui::Begin("Dockspace", &dockspaceOpen, window_flags);
	ImGui::PopStyleVar();

	if (opt_fullscreen)
		ImGui::PopStyleVar(2);

	// Submit the DockSpace
	const ImGuiIO& io = ImGui::GetIO();
	ImGuiStyle& style = ImGui::GetStyle();
	const float minWinSizeX = style.WindowMinSize.x;
	style.WindowMinSize.x = 370.0f;
	if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
	{
		const ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
	}

	style.WindowMinSize.x = minWinSizeX;

    int width, height;
    glfwGetWindowSize(Application::Get().GetWindow().GetNativeWindow(), &width, &height);
    int xPos, yPos;
    glfwGetWindowPos(Application::Get().GetWindow().GetNativeWindow(), &xPos, &yPos);

    ImGui::SetNextWindowPos(ImVec2(static_cast<float>(width) / 2.0f + xPos, static_cast<float>(height) / 2.0f + yPos), 0, ImVec2(0.5f, 0.5f));
    constexpr ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove;
    ImGui::Begin("Connect", &dockspaceOpen, flags);

    ImGui::BeginGroup();
    ImGui::Text("IP Address (IP:PORT)");
    ImGui::InputText("##ipaddr", s_ScratchBuffer.As<char>(), 100);
    if (ImGui::Button("Connect"))
        ConnectToServer();
    ImGui::EndGroup();

    ImGui::End(); // Login


    ImGui::End(); // Dockspace
}

void ClientAppLayer::OnConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *info)
{
    EPPO_INFO("Status changed");
}

void ClientAppLayer::ConnectToServer()
{
    const auto str = std::string(s_ScratchBuffer.As<char>(), 100);
}
