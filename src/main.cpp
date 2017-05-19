#include "stdafx.h"
#include <chrono>
#include "Util.h"
#include "GraphicsStructs.h"
#include "VulkanRenderer.h"

#include "Scene.h"
#include "Transform.h"
#include "MeshRenderer.h"
const int WIDTH = 800;
const int HEIGHT = 600;


const std::string CHALET_MODEL_PATH = "models/chalet.obj";
const std::string CHALET_TEXTURE_PATH = "textures/chalet.jpg";

const std::string CUBE_MODEL_PATH = "models/cube/";
const std::string CUBE_MODEL_NAME= "cube.obj";


class App {
public:
	App() {
	}

	~App() { }

	void Run() {
		GLFWwindow* window = glfwCreateWindow(800, 600, "Kai Engine", nullptr, nullptr);
		IRenderer* renderer = new Vulkan::VulkanRenderer(window);
		Scene* scene = new Scene(renderer);

		GameObject* cube = new GameObject();
		cube->AddComponent<Transform>();
		cube->AddComponent<MeshRenderer>();
		
		Mesh* mesh = new Mesh();
		renderer->InitMesh(mesh);
		mesh->LoadFromFile(CUBE_MODEL_PATH, CUBE_MODEL_NAME);
		MeshRenderer* cubeRenderer = cube->GetComponent<MeshRenderer>();
		cubeRenderer->SetMesh(mesh);
		Transform* cubeTransform = cube->GetComponent<Transform>();

		scene->Add(cube);

		while(!glfwWindowShouldClose(window)) {
			glfwPollEvents();	
			renderer->PrepareFrame();
			renderer->DrawFrame();
		}

		SafeDelete(scene);
		SafeDelete(renderer);
		glfwDestroyWindow(window);
		glfwTerminate();
	}
};

int main() {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	
	App app;

	try {
		app.Run();
	} catch(const std::runtime_error& e) {
		DebugPrintF("ERROR - %s\n", e.what());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
