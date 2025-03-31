#include <iostream>

int Main(int argc, char *argv[])
{
	return 0;
}

/*

using namespace Ailurus;

static constexpr const char* SYSTEM_MSYH_REGULAR_FONT_PATH = "c:\\Windows\\Fonts\\msyhl.ttc";
static constexpr const char* SYSTEM_YUGOTH_REGULAR_FONT_PATH = "c:\\Windows\\Fonts\\YuGothR.ttc";

ImFont* _pChineseFont = nullptr;
ImFont* _pJapaneseFont = nullptr;

void UpdateMainWindow();
void UpdateCharacters();
void UpdateNativeWindowFunctions(Window&);

int main()
{
    Window window;

    window.Create(1200, 800, "Test", WindowStyle::DefaultStyle());
    window.AddService<ImGuiOpenGLService>();

    auto imgui = window.GetService<ImGuiOpenGLService>();

    _pChineseFont = imgui->CreateImGuiFont(SYSTEM_MSYH_REGULAR_FONT_PATH, imgui->GetDefaultFontSize(),
        ImGui::GetIO().Fonts->GetGlyphRangesChineseSimplifiedCommon());

    _pJapaneseFont = imgui->CreateImGuiFont(SYSTEM_YUGOTH_REGULAR_FONT_PATH, imgui->GetDefaultFontSize(),
        ImGui::GetIO().Fonts->GetGlyphRangesJapanese());

    window.Loop([&]()-> void
    {
        const ImGuiViewport* mainViewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(ImVec2(mainViewport->WorkPos.x, mainViewport->WorkPos.y), ImGuiCond_Always);
        ImGui::SetNextWindowSize(mainViewport->WorkSize, ImGuiCond_Always);

        ImGuiWindowFlags window_flags = 0;
        window_flags |= ImGuiWindowFlags_NoTitleBar;
        window_flags |= ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoResize;
        window_flags |= ImGuiWindowFlags_NoCollapse;

        ImGui::Begin("Hello, world!", nullptr, window_flags);

        UpdateNativeWindowFunctions(window);
        UpdateCharacters();

        bool showDemo = true;
        ImGui::ShowDemoWindow(&showDemo);

        ImGui::End();
    });

    return 0;
}

void UpdateCharacters()
{
    ImGui::PushFont(_pChineseFont);
    ImGui::DebugTextEncoding("测试");
    ImGui::SeparatorText("中文标题");
    ImGui::PopFont();

    ImGui::PushFont(_pJapaneseFont);
    ImGui::DebugTextEncoding("試験");
    ImGui::SeparatorText("タイトル");
    ImGui::PopFont();
}

void UpdateNativeWindowFunctions(Window& window)
{
    ImGui::SeparatorText("Native Window States");

    auto [windowWidth, windowHeight] = window.GetSize();
    float size[2] = { static_cast<float>(windowWidth), static_cast<float>(windowHeight) };
    ImGui::InputFloat2("WindowSize", size);

    auto [windowPosX, windowPosY] = window.GetPosition();
    float pos[2] = { static_cast<float>(windowPosX), static_cast<float>(windowPosY) };
    ImGui::InputFloat2("WindowPos", pos);

    bool cursorVisible = window.IsCursorVisible();
    ImGui::Checkbox("CursorVisible", &cursorVisible);

    ImGui::SameLine();

    bool mouseInside = window.IsCursorInsideWindow();
    ImGui::Checkbox("MouseInsideWindow", &mouseInside);

    ImGui::SameLine();

    bool limitInWindow = window.IsCursorLimitedInWindow();
    ImGui::Checkbox("LimitInWindow", &limitInWindow);

    ImGui::Bullet();
    ImGui::Text("Press K to toggle mouse visible");

    ImGui::Bullet();
    ImGui::Text("Press G to toggle mouse limited in window");

    if (ImGui::IsKeyPressed(ImGuiKey_K))
    {
        bool current = window.IsCursorVisible();
        window.SetCursorVisible(!current);
    }

    if (ImGui::IsKeyPressed(ImGuiKey_G))
    {
        bool current = window.IsCursorLimitedInWindow();
        window.SetCursorLimitedInWindow(!current);
    }
}
*/