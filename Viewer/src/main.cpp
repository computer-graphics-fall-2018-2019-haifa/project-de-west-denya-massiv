#define _USE_MATH_DEFINES

#include <imgui/imgui.h>
#include <stdio.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "SceneActions.h"
#include "InputController.h"
#include "Renderer.h"
#include "Scene.h"
#include "Camera.h"
#include "ImguiMenus.h"
#include "Fogger.h"

// Function declarations
static void GlfwErrorCallback(int error, const char* description);
GLFWwindow* SetupGlfwWindow(int w, int h, const char* window_name);
ImGuiIO& SetupDearImgui(GLFWwindow* window);
void StartFrame();
void RenderFrame(GLFWwindow* window, Scene& scene, Renderer& renderer, ImGuiIO& io);
void Cleanup(GLFWwindow* window);
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
	
	// Handle mouse scrolling here...
}

static void HandleUserInput(ImGuiIO& io, IInputController& inputController)
{
	for (int i = 0; i < UCHAR_MAX; i++) if (io.KeysDownDuration[i] >= 0.0f) { inputController.KeyDown(i, io.KeysDownDuration[i]); }
	for (int i = 0; i < UCHAR_MAX; i++) if (ImGui::IsKeyPressed(i)) { inputController.KeyPress(i, false, false, false, false); }
	for (int i = 0; i < UCHAR_MAX; i++) if (ImGui::IsKeyReleased(i)) { inputController.KeyRelease(i); }
	
	if (!ImGui::IsMousePosValid()) { return; }

	for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++) if (io.MouseDownDuration[i] >= 0.0f) { inputController.MouseDown(i, io.MouseDownDuration[i]); }
	for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++) if (ImGui::IsMouseReleased(i)) { inputController.MouseRelease(i); }
	inputController.MouseMove(io.MouseDelta.x, io.MouseDelta.y);

	//ImGui::Text("Mouse clicked:");  for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++) if (ImGui::IsMouseClicked(i)) { ImGui::SameLine(); ImGui::Text("b%d", i); }
	//ImGui::Text("Mouse dbl-clicked:"); for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++) if (ImGui::IsMouseDoubleClicked(i)) { ImGui::SameLine(); ImGui::Text("b%d", i); }
	//ImGui::Text("Mouse released:"); for (int i = 0; i < IM_ARRAYSIZE(io.MouseDown); i++) if (ImGui::IsMouseReleased(i)) { ImGui::SameLine(); ImGui::Text("b%d", i); }
	//ImGui::Text("Mouse wheel: %.1f", io.MouseWheel);

}

int main(int argc, char **argv)
{
	// Create GLFW window
	int windowWidth = 1920, windowHeight = 1080;
	GLFWwindow* window = SetupGlfwWindow(windowWidth, windowHeight, "Mesh Viewer");
	if (!window)
	{
		return 1;
	}

	// Move OpenGL context to the newly created window
	glfwMakeContextCurrent(window);

	// Get the current width/height of the frame buffer
	int frameBufferWidth, frameBufferHeight;
	glfwGetFramebufferSize(window, &frameBufferWidth, &frameBufferHeight);

	// Create the scene
	Scene scene = Scene();
	
	// Clear the view
	glm::vec4 clearColor = scene.GetClearColor();
	glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
	glEnable(GL_DEPTH_TEST);

	// Input Controller
	std::vector<SceneAction> keysMapping(UCHAR_MAX,SceneAction::Nothing);
	keysMapping[toupper('w')] = SceneAction::MoveForward;
	keysMapping[toupper('a')] = SceneAction::MoveLeft;
	keysMapping[toupper('s')] = SceneAction::MoveBackwards;
	keysMapping[toupper('d')] = SceneAction::MoveRight;

	std::vector<SceneAction> mouseKeysMapping(MOUSE_BUTTONS_NUMBER, SceneAction::Nothing);
	mouseKeysMapping[MOUSE_WHEEL] = SceneAction::EnableDirectionChange;

	IInputController& inputController = InputController(scene.GetActiveMovingObject(),scene.GetActiveDirectionalObject(),keysMapping,mouseKeysMapping);

	// Create the renderer and the scene
	Renderer renderer = Renderer(scene);

	// Setup ImGui
	ImGuiIO& io = SetupDearImgui(window);

	// Register a mouse scroll-wheel callback
	glfwSetScrollCallback(window, ScrollCallback);

	// This is the main game loop..
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
		StartFrame();

		// Build the menus for the next frame
		DrawMenus(io, scene);

		// Handle user input
		HandleUserInput(io, inputController);

		// Render the next frame
		RenderFrame(window, scene, renderer, io);
    }

	// If we're here, then we're done. Cleanup memory.
	Cleanup(window);
    return 0;
}

static void GlfwErrorCallback(int error, const char* description)
{
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

GLFWwindow* SetupGlfwWindow(int w, int h, const char* window_name)
{
	glfwSetErrorCallback(GlfwErrorCallback);
	if (!glfwInit())
		return NULL;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	GLFWwindow* window = glfwCreateWindow(w, h, window_name, NULL, NULL);
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync
						 // very importent!! initialization of glad
						 // https://stackoverflow.com/questions/48582444/imgui-with-the-glad-opengl-loader-throws-segmentation-fault-core-dumped
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	return window;
}

ImGuiIO& SetupDearImgui(GLFWwindow* window)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	// Setup style
	ImGui::StyleColorsDark();

	return io;
}

void StartFrame()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void RenderFrame(GLFWwindow* window, Scene& scene, Renderer& renderer, ImGuiIO& io)
{
	// Render the menus
	auto start = std::chrono::high_resolution_clock::now();
	ImGui::Render();
	auto finish = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> elapsed = finish - start;
	scene.SetImGuiRenderExecutionTime(elapsed.count());

	// That's how you get the current width/height of the frame buffer (for example, after the window was resized)
	int frameBufferWidth, frameBufferHeight;
	glfwGetFramebufferSize(window, &frameBufferWidth, &frameBufferHeight);

	start = std::chrono::high_resolution_clock::now();
	renderer.ClearBuffers();
	finish = std::chrono::high_resolution_clock::now();
	elapsed = finish - start;
	scene.SetColorBufferExecutionTime(elapsed.count());

	// Render the scene
	renderer.Render();

	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	glfwSwapBuffers(window);
}

void Cleanup(GLFWwindow* window)
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glfwDestroyWindow(window);
	glfwTerminate();
}
