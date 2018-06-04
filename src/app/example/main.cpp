// ImGui - standalone example application for OpenGL 2, using fixed pipeline

#ifdef _MSC_VER
#pragma warning (disable: 4996)                        // 'This function or variable may be unsafe': strcpy, strdup, sprintf, vsnprintf, sscanf, fopen
#endif
#ifdef __clang__
#pragma clang diagnostic ignored "-Wunused-function"   // warning: unused function
#endif


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#ifdef _WIN32
#include <winsock2.h>
#endif

#ifdef __APPLE__
#include <unistd.h>
#endif
#include "../imgui/imgui.h"
#include <stdio.h>

// @RemoteImgui begin
#include "../imgui_remote.h"
#define VCANVAS_WIDTH  8192
#define VCANVAS_HEIGHT 8192
// @RemoteImgui end

// Glfw/Glew
#include <GLFW/glfw3.h>
#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))

// GLFW data
static GLFWwindow* window;
static double       g_Time = 0.0;
static bool         g_MouseJustPressed[3] = { false, false, false };
static GLFWcursor*  g_MouseCursors[ImGuiMouseCursor_COUNT] = { 0 };

// OpenGL data
static GLuint       g_FontTexture = 0;

// OpenGL2 Render function.
// (this used to be set in io.RenderDrawListsFn and called by ImGui::Render(), but you can now call this directly from your main loop)
// Note that this implementation is little overcomplicated because we are saving/setting up/restoring every OpenGL state explicitly, in order to be able to run within any OpenGL engine that doesn't do so. 
static void ImImpl_RenderDrawLists(ImDrawData* draw_data)
{
	// @RemoteImgui begin
	ImGui::RemoteDraw(draw_data->CmdLists, draw_data->CmdListsCount);
	// @RemoteImgui end

    if (draw_data->CmdListsCount == 0)
        return;

    // We are using the OpenGL fixed pipeline to make the example code simpler to read!
    // Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, vertex/texcoord/color pointers, polygon fill.
    GLint last_texture; glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    GLint last_polygon_mode[2]; glGetIntegerv(GL_POLYGON_MODE, last_polygon_mode);
    GLint last_viewport[4]; glGetIntegerv(GL_VIEWPORT, last_viewport);
    GLint last_scissor_box[4]; glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box); 
    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_TRANSFORM_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnable(GL_TEXTURE_2D);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    //glUseProgram(0); // You may want this if using this code in an OpenGL 3+ context where shaders may be bound

    // Setup orthographic projection matrix
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
	const float width = (float)display_w;
    const float height = (float)display_h;

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0f, width, height, 0.0f, -1.0f, +1.0f);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Render command lists
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
		const unsigned char* vtx_buffer = (const unsigned char*)&cmd_list->VtxBuffer.front();
		const ImDrawIdx* idx_buffer = &cmd_list->IdxBuffer.front();
        glVertexPointer(2, GL_FLOAT, sizeof(ImDrawVert), (const GLvoid*)((const char*)vtx_buffer + IM_OFFSETOF(ImDrawVert, pos)));
        glTexCoordPointer(2, GL_FLOAT, sizeof(ImDrawVert), (const GLvoid*)((const char*)vtx_buffer + IM_OFFSETOF(ImDrawVert, uv)));
        glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(ImDrawVert), (const GLvoid*)((const char*)vtx_buffer + IM_OFFSETOF(ImDrawVert, col)));

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback)
            {
                pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
				glScissor((int)pcmd->ClipRect.x, (int)(height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
                glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer);
            }
            idx_buffer += pcmd->ElemCount;
        }
    }

    // Restore modified state
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindTexture(GL_TEXTURE_2D, (GLuint)last_texture);
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();
    glPolygonMode(GL_FRONT, (GLenum)last_polygon_mode[0]); glPolygonMode(GL_BACK, (GLenum)last_polygon_mode[1]);
    glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
    glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
}

// NB: ImGui already provide OS clipboard support for Windows so this isn't needed if you are using Windows only.
static const char* ImImpl_GetClipboardTextFn(void* user_data)
{
    //return glfwGetClipboardString((GLFWwindow*)user_data);
    return glfwGetClipboardString(window);
}

static void ImImpl_SetClipboardTextFn(void* user_data, const char* text)
{
    //glfwSetClipboardString((GLFWwindow*)user_data, text);
    glfwSetClipboardString(window, text);
}

// GLFW callbacks to get events
static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Error %d: %s\n", error, description);
}

static void glfw_mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (action == GLFW_PRESS && button >= 0 && button < 3)
        g_MouseJustPressed[button] = true;
}

static void glfw_scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    ImGuiIO& io = ImGui::GetIO();
    io.MouseWheelH += (float)xoffset;
    io.MouseWheel += (float)yoffset;
}

static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    ImGuiIO& io = ImGui::GetIO();
    if (action == GLFW_PRESS)
        io.KeysDown[key] = true;
    if (action == GLFW_RELEASE)
        io.KeysDown[key] = false;

    (void)mods; // Modifiers are not reliable across systems
    io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
    io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
    io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
    io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
}

static void glfw_char_callback(GLFWwindow* window, unsigned int c)
{
    ImGuiIO& io = ImGui::GetIO();
    if (c > 0 && c < 0x10000)
        io.AddInputCharacter((unsigned short)c);
}

void InitGL()
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        exit(1);

    window = glfwCreateWindow(1280, 720, "ImGui OpenGL2 example", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui binding
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    glfwSetKeyCallback(window, glfw_key_callback);
    glfwSetMouseButtonCallback(window, glfw_mouse_button_callback);
    glfwSetScrollCallback(window, glfw_scroll_callback);
    glfwSetCharCallback(window, glfw_char_callback);
    // Setup style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();
}

void LoadFontsTexture()
{
    ImGuiIO& io = ImGui::GetIO();
    //ImFont* my_font1 = io.Fonts->AddFontDefault();
    //ImFont* my_font2 = io.Fonts->AddFontFromFileTTF("extra_fonts/Karla-Regular.ttf", 15.0f);
    //ImFont* my_font3 = io.Fonts->AddFontFromFileTTF("extra_fonts/ProggyClean.ttf", 13.0f); my_font3->DisplayOffset.y += 1;
    //ImFont* my_font4 = io.Fonts->AddFontFromFileTTF("extra_fonts/ProggyTiny.ttf", 10.0f); my_font4->DisplayOffset.y += 1;
    //ImFont* my_font5 = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 20.0f, io.Fonts->GetGlyphRangesJapanese());

    unsigned char* pixels;
    int width, height;
    //io.Fonts->GetTexDataAsAlpha8(&pixels, &width, &height);
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);   // Load as RGBA 32-bits (75% of the memory is wasted, but default font is so small) because it is more likely to be compatible with user's existing shaders. If your ImTextureId represent a higher-level concept than just a GL texture id, consider calling GetTexDataAsAlpha8() instead to save on GPU memory.

    // Upload texture to graphics system
    GLint last_texture;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    glGenTextures(1, &g_FontTexture);
    glBindTexture(GL_TEXTURE_2D, g_FontTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, pixels);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

    // Store our identifier
    io.Fonts->TexID = (void *)(intptr_t)g_FontTexture;

    // Restore state
    glBindTexture(GL_TEXTURE_2D, last_texture);

}

void InitImGui()
{
    g_Time = 0.0;

    // Setup back-end capabilities flags
    ImGuiIO& io = ImGui::GetIO();
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;   // We can honor GetMouseCursor() values (optional)
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;    // We can honor io.WantSetMousePos requests (optional, rarely used)

    // Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array.
    io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
    io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
    io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
    io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
    io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
    io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
    io.KeyMap[ImGuiKey_Insert] = GLFW_KEY_INSERT;
    io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
    io.KeyMap[ImGuiKey_Space] = GLFW_KEY_SPACE;
    io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
    io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
    io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
    io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
    io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
    io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
    io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
    io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

    io.RenderDrawListsFn = ImImpl_RenderDrawLists;
    io.SetClipboardTextFn = ImImpl_SetClipboardTextFn;
    io.GetClipboardTextFn = ImImpl_GetClipboardTextFn;
    io.ClipboardUserData = window;
#ifdef _WIN32
    io.ImeWindowHandle = glfwGetWin32Window(g_Window);
#endif
    // Load cursors
    // FIXME: GLFW doesn't expose suitable cursors for ResizeAll, ResizeNESW, ResizeNWSE. We revert to arrow cursor for those.
    g_MouseCursors[ImGuiMouseCursor_Arrow] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    g_MouseCursors[ImGuiMouseCursor_TextInput] = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
    g_MouseCursors[ImGuiMouseCursor_ResizeAll] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    g_MouseCursors[ImGuiMouseCursor_ResizeNS] = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
    g_MouseCursors[ImGuiMouseCursor_ResizeEW] = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
    g_MouseCursors[ImGuiMouseCursor_ResizeNESW] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    g_MouseCursors[ImGuiMouseCursor_ResizeNWSE] = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);

    LoadFontsTexture();
	// @RemoteImgui begin
	ImGui::RemoteInit("0.0.0.0", 7002); // local host, local port
	//ImGui::GetStyle().WindowRounding = 0.f; // no rounding uses less bandwidth
	io.DisplaySize = ImVec2((float)VCANVAS_WIDTH, (float)VCANVAS_HEIGHT);
	// @RemoteImgui end
}

void UpdateImGui()
{
    if (!g_FontTexture)
	LoadFontsTexture();

    ImGuiIO& io = ImGui::GetIO();

    // Setup display size (every frame to accommodate for window resizing)
    int w, h;
    int display_w, display_h;
    glfwGetWindowSize(window, &w, &h);
    glfwGetFramebufferSize(window, &display_w, &display_h);
    io.DisplaySize = ImVec2((float)w, (float)h);
    io.DisplayFramebufferScale = ImVec2(w > 0 ? ((float)display_w / w) : 0, h > 0 ? ((float)display_h / h) : 0);

    // Setup time step
    double current_time =  glfwGetTime();
    io.DeltaTime = g_Time > 0.0 ? (float)(current_time - g_Time) : (float)(1.0f/60.0f);
    g_Time = current_time;

	// @RemoteImgui begin
	ImGui::RemoteUpdate();
	ImGui::RemoteInput input;
	if (ImGui::RemoteGetInput(input))
	{
		ImGuiIO& io = ImGui::GetIO();
		for (int i = 0; i < 256; i++)
			io.KeysDown[i] = input.KeysDown[i];
		io.KeyCtrl = input.KeyCtrl;
		io.KeyShift = input.KeyShift;
		io.MousePos = input.MousePos;
		io.MouseDown[0] = (input.MouseButtons & 1);
		io.MouseDown[1] = (input.MouseButtons & 2) != 0;
		io.MouseWheel += input.MouseWheelDelta;
		// Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array.
		io.KeyMap[ImGuiKey_Tab] = ImGuiKey_Tab;
		io.KeyMap[ImGuiKey_LeftArrow] = ImGuiKey_LeftArrow;
		io.KeyMap[ImGuiKey_RightArrow] = ImGuiKey_RightArrow;
		io.KeyMap[ImGuiKey_UpArrow] = ImGuiKey_UpArrow;
		io.KeyMap[ImGuiKey_DownArrow] = ImGuiKey_DownArrow;
		io.KeyMap[ImGuiKey_Home] = ImGuiKey_Home;
		io.KeyMap[ImGuiKey_End] = ImGuiKey_End;
		io.KeyMap[ImGuiKey_Delete] = ImGuiKey_Delete;
		io.KeyMap[ImGuiKey_Backspace] = ImGuiKey_Backspace;
		io.KeyMap[ImGuiKey_Enter] = 13;
		io.KeyMap[ImGuiKey_Escape] = 27;
		io.KeyMap[ImGuiKey_A] = 'a';
		io.KeyMap[ImGuiKey_C] = 'c';
		io.KeyMap[ImGuiKey_V] = 'v';
		io.KeyMap[ImGuiKey_X] = 'x';
		io.KeyMap[ImGuiKey_Y] = 'y';
		io.KeyMap[ImGuiKey_Z] = 'z';
	}
	else
	// @RemoteImgui end
	{
		// Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array.
		io.KeyMap[ImGuiKey_Tab] = 9;
		io.KeyMap[ImGuiKey_LeftArrow] = ImGuiKey_LeftArrow;
		io.KeyMap[ImGuiKey_RightArrow] = ImGuiKey_RightArrow;
		io.KeyMap[ImGuiKey_UpArrow] = ImGuiKey_UpArrow;
		io.KeyMap[ImGuiKey_DownArrow] = ImGuiKey_DownArrow;
		io.KeyMap[ImGuiKey_Home] = ImGuiKey_Home;
		io.KeyMap[ImGuiKey_End] = ImGuiKey_End;
		io.KeyMap[ImGuiKey_Delete] = ImGuiKey_Delete;
		io.KeyMap[ImGuiKey_Backspace] = 127;
		io.KeyMap[ImGuiKey_Enter] = 13;
		io.KeyMap[ImGuiKey_Escape] = 27;
		io.KeyMap[ImGuiKey_A] = 'a';
		io.KeyMap[ImGuiKey_C] = 'c';
		io.KeyMap[ImGuiKey_V] = 'v';
		io.KeyMap[ImGuiKey_X] = 'x';
		io.KeyMap[ImGuiKey_Y] = 'y';
		io.KeyMap[ImGuiKey_Z] = 'z';

    // Setup inputs
    // (we already got mouse wheel, keyboard keys & characters from glfw callbacks polled in glfwPollEvents())
    if (glfwGetWindowAttrib(window, GLFW_FOCUSED))
    {
        // Set OS mouse position if requested (only used when ImGuiConfigFlags_NavEnableSetMousePos is enabled by user)
        if (io.WantSetMousePos)
        {
            glfwSetCursorPos(window, (double)io.MousePos.x, (double)io.MousePos.y);
        }
        else
        {
            double mouse_x, mouse_y;
            glfwGetCursorPos(window, &mouse_x, &mouse_y);
            io.MousePos = ImVec2((float)mouse_x, (float)mouse_y);
        }
    }
    else
    {
        io.MousePos = ImVec2(-FLT_MAX,-FLT_MAX);
    }

    for (int i = 0; i < 3; i++)
    {
        // If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
        io.MouseDown[i] = g_MouseJustPressed[i] || glfwGetMouseButton(window, i) != 0;
        g_MouseJustPressed[i] = false;
    }

    // Update OS/hardware mouse cursor if imgui isn't drawing a software cursor
    if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) == 0 && glfwGetInputMode(window, GLFW_CURSOR) != GLFW_CURSOR_DISABLED)
    {
        ImGuiMouseCursor cursor = ImGui::GetMouseCursor();
        if (io.MouseDrawCursor || cursor == ImGuiMouseCursor_None)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
        }
        else
        {
            glfwSetCursor(window, g_MouseCursors[cursor] ? g_MouseCursors[cursor] : g_MouseCursors[ImGuiMouseCursor_Arrow]);
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
  }
    // Start the frame. This call will update the io.WantCaptureMouse, io.WantCaptureKeyboard flag that you can use to dispatch inputs (or not) to your application.
    ImGui::NewFrame();
}

// Application code
int main(int argc, char** argv)
{
    InitGL();
    InitImGui();

    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        ImGuiIO& io = ImGui::GetIO();
        g_MouseJustPressed[0] = g_MouseJustPressed[1] = false;
        glfwPollEvents();
        UpdateImGui();

        // 1. Show a simple window.
        // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets automatically appears in a window called "Debug".
        {
            static float f = 0.0f;
            static int counter = 0;
            ImGui::Text("Hello, world!");                           // Display some text (you can use a format string too)
            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f    
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our windows open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (NB: most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        }

        // 2. Show another simple window. In most cases you will use an explicit Begin/End pair to name your windows.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        // 3. Show the ImGui demo window. Most of the sample code is in ImGui::ShowDemoWindow(). Read its code to learn more about Dear ImGui!
        if (show_demo_window)
        {
            ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver); // Normally user code doesn't need/want to call this because positions are saved in .ini file anyway. Here we just want to make the demo initial state a bit more friendly!
            ImGui::ShowDemoWindow(&show_demo_window);
        }


	// Rendering
	int display_w, display_h;
	glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        //glUseProgram(0); // You may want this if using this code in an OpenGL 3+ context where shaders may be bound, but prefer using the GL3+ code.
        ImGui::Render();
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui::DestroyContext();
    // Destroy GLFW mouse cursors
    for (ImGuiMouseCursor cursor_n = 0; cursor_n < ImGuiMouseCursor_COUNT; cursor_n++)
        glfwDestroyCursor(g_MouseCursors[cursor_n]);
    memset(g_MouseCursors, 0, sizeof(g_MouseCursors));
    if (g_FontTexture)
    {
        glDeleteTextures(1, &g_FontTexture);
        ImGui::GetIO().Fonts->TexID = 0;
        g_FontTexture = 0;
    }
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
