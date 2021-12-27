#pragma once

#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <iostream>
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_dx11.h"
#include "Kiero/kiero.h"
#include "Globals.hpp"
#include "Beacons.hpp"
#include "SDK.hpp"

enum EBeaconState
{
	AllowRequests,
	DenyRequests
};

using namespace SDK;

typedef HRESULT(__stdcall* Present) (IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
typedef LRESULT(CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);
typedef uintptr_t PTR;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

Present oPresent;
HWND window = NULL;
WNDPROC oWndProc;
ID3D11Device* pDevice = NULL;
ID3D11DeviceContext* pContext = NULL;
ID3D11RenderTargetView* mainRenderTargetView;
static bool show = false;
static bool bSpeedHack = false;

void InitImGui()
{
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX11_Init(pDevice, pContext);
}

LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

	if (uMsg == WM_KEYUP && (wParam == 0x2D /*INSERT*/ || wParam == VK_F8 || (show && wParam == VK_ESCAPE)))
	{
		show = !show;
		ImGui::GetIO().MouseDrawCursor = show;
	}
	else if (uMsg == WM_QUIT && show)
	{
		ExitProcess(0);
	}

	if (show)
	{
		ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
		return TRUE;
	}

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

bool init = false;
HRESULT __stdcall hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags)
{
	if (!init)
	{
		if (SUCCEEDED(pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&pDevice)))
		{
			pDevice->GetImmediateContext(&pContext);
			DXGI_SWAP_CHAIN_DESC sd;
			pSwapChain->GetDesc(&sd);
			window = sd.OutputWindow;
			ID3D11Texture2D* pBackBuffer;
			pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
			pDevice->CreateRenderTargetView(pBackBuffer, NULL, &mainRenderTargetView);
			pBackBuffer->Release();
			oWndProc = (WNDPROC)SetWindowLongPtr(window, GWLP_WNDPROC, (LONG_PTR)WndProc);
			InitImGui();
			init = true;
		}

		else
			return oPresent(pSwapChain, SyncInterval, Flags);
	}

	if (show)
	{
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		ImGui::SetNextWindowBgAlpha(0.8);
		ImGui::SetNextWindowSize(ImVec2(560, 345));

		ImGui::Begin("FusionGS", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
		static int tab = 0;
		static int port = 7777;
		static char IP[30] = "";

		if (ImGui::BeginTabBar("", 0)) {

			if (ImGui::BeginTabItem("Client")) {
				tab = 0;
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("Server")) {
				tab = 1;
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Other")) {
				tab = 2;
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}

		switch (tab)
		{
		case 0:
			ImGui::InputText("IP", IP, IM_ARRAYSIZE(IP));
			if (ImGui::Button("Connect"))
			{
				auto map = IP + std::string("//Game/Maps/Athena_Terrain?game=athena");
			}
			break;
		case 1:
			ImGui::InputInt("Port", &port);
			if (ImGui::Button("Start Server"))
			{
				Beacons::Init();

				auto Beacon = SpawnActor<AOnlineBeaconHost>(AOnlineBeaconHost::StaticClass(), { 0, 0, 10000 }, {});
				Beacon->ListenPort = 7777;

				Beacons::InitHost(Beacon);
				Beacon->BeaconState = EBeaconState::AllowRequests;

				TArray<AActor*> OutActors;
				Globals::GameplayStatics->STATIC_GetAllActorsOfClass(Globals::World, APlayerPawn_Athena_C::StaticClass(), &OutActors);
				Globals::Pawn = static_cast<APlayerPawn_Athena_C*>(OutActors[0]);

				if (Globals::Pawn)
				{
					Globals::PlayerController->Possess(Globals::Pawn);

					auto PlayerState = reinterpret_cast<AFortPlayerStateAthena*>(Globals::PlayerController->PlayerState);
					PlayerState->TeamIndex = EFortTeam::HumanPvP_Team1;
					PlayerState->OnRep_TeamIndex();

					Sleep(2000);

					Globals::PlayerController->ServerReadyToStartMatch();
					static_cast<AGameMode*>(Globals::World->AuthorityGameMode)->StartMatch();
				}
				else
					MessageBoxA(NULL, "Invalid Player pawn", "EGS V2", MB_OK);
			}
			break;
		case 2:
			if (ImGui::Button("Dump Objects"))
			{
				// CreateThread(0, 0, UE4::DumpGObjects, 0, 0, 0);
			}
		}
		ImGui::End();

		ImGui::Render();

		pContext->OMSetRenderTargets(1, &mainRenderTargetView, NULL);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}
	return oPresent(pSwapChain, SyncInterval, Flags);
}

DWORD WINAPI GuiThread(LPVOID lpReserved)
{
	bool init_hook = false;
	do
	{
		if (kiero::init(kiero::RenderType::D3D11) == kiero::Status::Success)
		{
			kiero::bind(8, (void**)&oPresent, hkPresent);
			init_hook = true;
		}
	} while (!init_hook);
	std::cout << "Initialized ImGUI!\n";
	return TRUE;
}
