#include <iostream>
#include <NativeWindow/Window.h>
#include <NativeWindow/Service/ImGuiService/ImGuiOpenGLService.h>

using namespace Ailurus;

static constexpr const char* SYSTEM_MSYH_REGULAR_FONT_PATH = "c:\\Windows\\Fonts\\msyhl.ttc";
static float f = 0.0f;
static int counter = 0;

ImFont* _pChineseFont = nullptr;

void UpdateMainWindow();
void UpdateNativeWindowFunctions(Window&);

int main()
{
    Window window;

    window.Create(800, 600, "Test", WindowStyle::DefaultStyle());
    window.AddService<ImGuiOpenGLService>();

    auto imgui = window.GetService<ImGuiOpenGLService>();
    auto defaultFontSize = imgui->GetDefaultFontSize();
    _pChineseFont = imgui->CreateImGuiFont(SYSTEM_MSYH_REGULAR_FONT_PATH, defaultFontSize);

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
        UpdateMainWindow();

        ImGui::End();
    });

    system("pause");

    return 0;
}

void UpdateMainWindow()
{
    ImGuiIO& io = ImGui::GetIO();

    ImGui::SeparatorText("ImGui Test");

    ImGui::SliderFloat("float", &f, 0.0f, 1.0f); // Edit 1 float using a slider from 0.0f to 1.0f

    if (ImGui::Button("Button"))
        counter++;

    ImGui::SameLine();

    if (ImGui::SmallButton("SmallButton"))
        counter++;

    ImGui::SameLine();

    if (ImGui::ArrowButton("ArrowButton", ImGuiDir_Up))
        counter++;

    ImGui::SameLine();

    ImGui::Text("counter = %d", counter);

    ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

    ImGui::Bullet();
    ImGui::Text("Bullet"); // auto same line

    if (ImGui::RadioButton("RadioButton", false))
        counter++;

    if (ImGui::CollapsingHeader("Help"))
    {
        ImGui::SeparatorText("ABOUT THIS DEMO:");
        ImGui::BulletText("Sections below are demonstrating many aspects of the library.");
        ImGui::BulletText("The \"Examples\" menu above leads to more demo contents.");
        ImGui::BulletText("The \"Tools\" menu above gives access to: About Box, Style Editor,\n"
            "and Metrics/Debugger (general purpose Dear ImGui debugging tool).");

        ImGui::SeparatorText("PROGRAMMER GUIDE:");
        ImGui::BulletText("See the ShowDemoWindow() code in imgui_demo.cpp. <- you are here!");
        ImGui::BulletText("See comments in imgui.cpp.");
        ImGui::BulletText("See example applications in the examples/ folder.");
        ImGui::BulletText("Read the FAQ at ");
        ImGui::SameLine(0, 0);
        ImGui::TextLinkOpenURL("https://www.dearimgui.com/faq/");
        ImGui::BulletText("Set 'io.ConfigFlags |= NavEnableKeyboard' for keyboard controls.");
        ImGui::BulletText("Set 'io.ConfigFlags |= NavEnableGamepad' for gamepad controls.");

        ImGui::SeparatorText("USER GUIDE:");
        ImGuiIO& io = ImGui::GetIO();
        ImGui::BulletText("Double-click on title bar to collapse window.");
        ImGui::BulletText(
            "Click and drag on lower corner to resize window\n"
            "(double-click to auto fit window to its contents).");
        ImGui::BulletText("CTRL+Click on a slider or drag box to input value as text.");
        ImGui::BulletText("TAB/SHIFT+TAB to cycle through keyboard editable fields.");
        ImGui::BulletText("CTRL+Tab to select a window.");
        if (io.FontAllowUserScaling)
            ImGui::BulletText("CTRL+Mouse Wheel to zoom window contents.");
        ImGui::BulletText("While inputing text:\n");
        ImGui::Indent();
        ImGui::BulletText("CTRL+Left/Right to word jump.");
        ImGui::BulletText("CTRL+A or double-click to select all.");
        ImGui::BulletText("CTRL+X/C/V to use clipboard cut/copy/paste.");
        ImGui::BulletText("CTRL+Z,CTRL+Y to undo/redo.");
        ImGui::BulletText("ESCAPE to revert.");
        ImGui::Unindent();
        ImGui::BulletText("With keyboard navigation enabled:");
        ImGui::Indent();
        ImGui::BulletText("Arrow keys to navigate.");
        ImGui::BulletText("Space to activate a widget.");
        ImGui::BulletText("Return to input text into a widget.");
        ImGui::BulletText("Escape to deactivate a widget, close popup, exit child window.");
        ImGui::BulletText("Alt to jump to the menu layer of a window.");
        ImGui::Unindent();
    }

    if (ImGui::TreeNode("Grid"))
    {
        static char selected[4][4] = {{1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {0, 0, 0, 1}};

        // Add in a bit of silly fun...
        const float time = (float) ImGui::GetTime();
        const bool winning_state = memchr(selected, 0, sizeof(selected)) == NULL; // If all cells are selected...
        if (winning_state)
            ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign,
                                ImVec2(0.5f + 0.5f * cosf(time * 2.0f), 0.5f + 0.5f * sinf(time * 3.0f)));

        for (int y = 0; y < 4; y++)
            for (int x = 0; x < 4; x++)
            {
                if (x > 0)
                    ImGui::SameLine();
                ImGui::PushID(y * 4 + x);
                if (ImGui::Selectable("Sailor", selected[y][x] != 0, 0, ImVec2(50, 50)))
                {
                    // Toggle clicked cell + toggle neighbors
                    selected[y][x] ^= 1;
                    if (x > 0) { selected[y][x - 1] ^= 1; }
                    if (x < 3) { selected[y][x + 1] ^= 1; }
                    if (y > 0) { selected[y - 1][x] ^= 1; }
                    if (y < 3) { selected[y + 1][x] ^= 1; }
                }
                ImGui::PopID();
            }

        if (winning_state)
            ImGui::PopStyleVar();
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Tab"))
    {
        ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
        if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags))
        {
            if (ImGui::BeginTabItem("Avocado"))
            {
                ImGui::Text("This is the Avocado tab!\nblah blah blah blah blah");
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Broccoli"))
            {
                ImGui::Text("This is the Broccoli tab!\nblah blah blah blah blah");
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Cucumber"))
            {
                ImGui::Text("This is the Cucumber tab!\nblah blah blah blah blah");
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        ImGui::Separator();
        ImGui::TreePop();
    }

    ImGui::PushFont(_pChineseFont);
    ImGui::DebugTextEncoding("测试");
    ImGui::SeparatorText("中文标题");
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
