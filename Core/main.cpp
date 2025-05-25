#include <IMGUI\imgui.h>
#include <IMGUI\backends\imgui_impl_win32.h>
#include <IMGUI\backends\imgui_impl_dx11.h>
#include <d3d11.h>
#include <tchar.h>
#include <string>
#include <iostream>
#include "main.h"
#include "pickword.h"
#include <vector>

// Data
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static bool                     g_SwapChainOccluded = false;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Main code
int main(int, char**)
{
	// Create application window
	//ImGui_ImplWin32_EnableDpiAwareness();
	int scrnw = GetSystemMetrics(SM_CXSCREEN);
	int scrny = GetSystemMetrics(SM_CYSCREEN);
	WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
	::RegisterClassExW(&wc);
	HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Dear ImGui DirectX11 Example", WS_POPUP, 0, 0, scrnw, scrny, nullptr, nullptr, wc.hInstance, nullptr);
	// Initialize Direct3D
	if (!CreateDeviceD3D(hwnd))
	{
		CleanupDeviceD3D();
		::UnregisterClassW(wc.lpszClassName, wc.hInstance);
		return 1;
	}

	// Show the window
	::ShowWindow(hwnd, SW_SHOWDEFAULT);
	::UpdateWindow(hwnd);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

	// Load Fonts
	// - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
	// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
	// - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
	// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
	// - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
	// - Read 'docs/FONTS.md' for more instructions and details.
	// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
	//io.Fonts->AddFontDefault();
	//io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
	//ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
	//IM_ASSERT(font != nullptr);

	// Our state
	bool show_demo_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	// Load bigger font (in your setup, e.g. before ImGui::NewFrame())
	io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\consola.ttf", 28.0f); // Or any TTF
	// Adjust size/font path if needed

	// Main loop
	while (!done)
	{
		// Poll and handle messages (inputs, window resize, etc.)
		// See the WndProc() function below for our to dispatch events to the Win32 backend.
		MSG msg;
		while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
				done = true;
		}
		if (done)
			break;

		// Handle window being minimized or screen locked
		if (g_SwapChainOccluded && g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
		{
			::Sleep(10);
			continue;
		}
		g_SwapChainOccluded = false;

		// Handle window resize (we don't resize directly in the WM_SIZE handler)
		if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
		{
			CleanupRenderTarget();
			g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
			g_ResizeWidth = g_ResizeHeight = 0;
			CreateRenderTarget();
		}

		// Start the Dear ImGui frame
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		ImGuiStyle& style = ImGui::GetStyle();
		style.Colors[ImGuiCol_WindowBg] = ImColor(0, 0, 0, 255);
		style.Colors[ImGuiCol_ChildBg] = ImColor(255, 255, 255, 20);
		style.ChildRounding = 12.0f;
		style.WindowBorderSize = 0.0f;

		ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
		ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

		if (ImGui::Begin("##MainWindow", nullptr,
			ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {

			ImDrawList* drawList = ImGui::GetWindowDrawList();
			const float screenWidth = ImGui::GetIO().DisplaySize.x;
			const float screenHeight = ImGui::GetIO().DisplaySize.y;

			// ----- BACKGROUND -----
			drawList->AddRectFilledMultiColor(
				ImVec2(0, 0), ImVec2(screenWidth, screenHeight),
				IM_COL32(10, 10, 10, 255), IM_COL32(5, 5, 5, 255),
				IM_COL32(15, 15, 15, 255), IM_COL32(10, 10, 10, 255)
			);

			// ----- RADIO SEGMENTED CONTROL -----
			const char* timeOptions[] = { "30s", "60s", "120s" };
			static int selectedTime = 1;
			float segmentWidth = 60.0f;
			float segmentHeight = 28.0f;
			float groupWidth = segmentWidth * 3;
			float startX = (screenWidth - groupWidth) / 2.0f;
			float startY = screenHeight * 0.16f;

			ImVec2 groupStart = ImVec2(startX, startY);
			ImVec2 groupEnd = ImVec2(startX + groupWidth, startY + segmentHeight);
			drawList->AddRectFilled(groupStart, groupEnd, IM_COL32(25, 25, 25, 200), 8.0f);

			for (int i = 0; i < 3; ++i) {
				ImVec2 btnStart = ImVec2(startX + i * segmentWidth, startY);
				ImVec2 btnEnd = ImVec2(btnStart.x + segmentWidth, btnStart.y + segmentHeight);
				bool hovered = ImGui::IsMouseHoveringRect(btnStart, btnEnd);
				bool clicked = ImGui::IsMouseClicked(0) && hovered;

				ImU32 fillColor = selectedTime == i
					? IM_COL32(20, 40, 90, 255)
					: hovered ? IM_COL32(60, 60, 60, 200)
					: IM_COL32(40, 40, 40, 150);

				drawList->AddRectFilled(btnStart, btnEnd, fillColor,
					i == 0 ? 8.0f : (i == 2 ? 8.0f : 0.0f),
					i == 0 ? ImDrawFlags_RoundCornersLeft : (i == 2 ? ImDrawFlags_RoundCornersRight : 0)
				);

				if (i > 0) {
					drawList->AddLine(
						ImVec2(btnStart.x, btnStart.y),
						ImVec2(btnStart.x, btnEnd.y),
						IM_COL32(70, 70, 70, 150), 1.0f
					);
				}

				ImVec2 labelSize = ImGui::CalcTextSize(timeOptions[i]);
				ImVec2 labelPos = ImVec2(btnStart.x + (segmentWidth - labelSize.x) / 2.0f,
					btnStart.y + (segmentHeight - labelSize.y) / 2.0f);
				drawList->AddText(labelPos, IM_COL32(255, 255, 255, 255), timeOptions[i]);

				if (clicked) selectedTime = i;
			}

			// ----- FROSTED GLASS TYPING BOX -----
			const float boxWidth = screenWidth * 0.8f;
			const float boxHeight = screenHeight * 0.5f;
			const float boxX = (screenWidth - boxWidth) / 2.0f;
			const float boxY = screenHeight * 0.3f;
			ImVec2 boxPos = ImVec2(boxX, boxY);
			ImVec2 boxEnd = ImVec2(boxX + boxWidth, boxY + boxHeight);

			drawList->AddRectFilled(boxPos, boxEnd, IM_COL32(255, 255, 255, 35), 15.0f);
			drawList->AddRect(boxPos, boxEnd, IM_COL32(255, 255, 255, 40), 15.0f);
			drawList->AddRectFilledMultiColor(
				boxPos, ImVec2(boxEnd.x, boxPos.y + 20),
				IM_COL32(255, 255, 255, 60), IM_COL32(255, 255, 255, 30),
				IM_COL32(255, 255, 255, 15), IM_COL32(255, 255, 255, 5)
			);

			// ----- TYPING LOGIC -----
			static std::vector<std::string> testWords;
			static bool wordsInitialized = false;
			static pickword pw;

			if (!wordsInitialized) {
				testWords.clear();
				for (int i = 0; i < 2000; ++i)
					testWords.push_back(pw.getword(80));
				wordsInitialized = true;
			}

			static char inputBuffer[8192] = "";
			size_t bufferIndex = 0;

			float padding = 30.0f;
			float x = boxPos.x + padding;
			float y = boxPos.y + 20.0f;
			float endX = boxEnd.x - padding;
			float bottomY = boxEnd.y - padding;
			float spaceSize = ImGui::CalcTextSize(" ").x;
			float lineSpacing = 8.0f;
			float lineHeight = ImGui::GetFont()->FontSize + lineSpacing;

			std::vector<std::string> wordsToRender(testWords.begin(), testWords.begin() + std::min<int>(testWords.size(), 2000));
			for (const std::string& word : wordsToRender) {
				float wordWidth = ImGui::CalcTextSize(word.c_str()).x;

				if (x + wordWidth > endX) {
					x = boxPos.x + padding;
					y += lineHeight;
					if (y + lineHeight > bottomY) break;
				}

				for (char c : word) {
					ImU32 col;
					if (bufferIndex < strlen(inputBuffer))
						col = (inputBuffer[bufferIndex] == c) ? IM_COL32(255, 255, 255, 255) : IM_COL32(255, 70, 70, 255);
					else
						col = IM_COL32(180, 180, 180, 100);

					char buf[2] = { c, '\0' };
					drawList->AddText(ImVec2(x + 1, y + 1), IM_COL32(0, 0, 0, 100), buf);
					drawList->AddText(ImVec2(x, y), col, buf);

					x += ImGui::CalcTextSize(buf).x;
					bufferIndex++;
				}

				x += spaceSize;
				bufferIndex++;
			}

			// ----- INVISIBLE TEXT INPUT -----
			ImGui::SetCursorPos(ImVec2(-1000, -1000));
			ImGui::PushItemWidth(0);
			ImGui::SetKeyboardFocusHere();
			ImGui::InputText("##hiddeninput", inputBuffer, IM_ARRAYSIZE(inputBuffer), ImGuiInputTextFlags_None);
			ImGui::PopItemWidth();

			// ----- FULLSCREEN TOGGLE -----
			if (ImGui::IsKeyPressed(ImGuiKey_F11)) {
				if (f11PressCount == 0) {
					SetWindowLongPtr(hwnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
					SetWindowPos(hwnd, 0, 0, 0, scrnw, scrny, SWP_SHOWWINDOW);
					f11PressCount = 1;
				}
				else if (f11PressCount == 2) {
					SetWindowLongPtr(hwnd, GWL_STYLE, WS_POPUP);
					SetWindowPos(hwnd, 0, 0, 0, scrnw, scrny, SWP_SHOWWINDOW);
					f11PressCount = 3;
				}
			}
			if (ImGui::IsKeyReleased(ImGuiKey_F11)) {
				if (f11PressCount == 1) f11PressCount = 2;
				else if (f11PressCount == 3) f11PressCount = 0;
			}

			if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
				static bool once = []() { exit(0); return true; }();
			}
		}
		ImGui::End();


		// Rendering
		ImGui::Render();
		const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
		g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
		g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());


		// Present
		HRESULT hr = g_pSwapChain->Present(1, 0);   // Present with vsync
		//HRESULT hr = g_pSwapChain->Present(0, 0); // Present without vsync 
		g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
	}

	// Cleanup
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	CleanupDeviceD3D();
	::DestroyWindow(hwnd);
	::UnregisterClassW(wc.lpszClassName, wc.hInstance);

	return 0;
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
	// Setup swap chain
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = 0;
	//createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
	if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
		res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
	if (res != S_OK)
		return false;

	CreateRenderTarget();
	return true;
}

void CleanupDeviceD3D()
{
	CleanupRenderTarget();
	if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
	if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
	if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
	ID3D11Texture2D* pBackBuffer;
	g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
	pBackBuffer->Release();
}

void CleanupRenderTarget()
{
	if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_SIZE:
		if (wParam == SIZE_MINIMIZED)
			return 0;
		g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
		g_ResizeHeight = (UINT)HIWORD(lParam);
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}
	return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}