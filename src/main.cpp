#include "stdafx.h"
#include <chrono>
#include <glm/gtc/matrix_transform.hpp>
#include "Util.h"
#include "GfxTypes.h"
#include "VulkanRenderer.h"
#include "Scene.h"
#include "Transform.h"
#include "MeshRenderer.h"
#include "Camera.h"

const int WIDTH = 800;
const int HEIGHT = 600;

const std::string SPONZA_MODEL_NAME = "sponza.obj";
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

		GameObject* mainCamera = new GameObject();
		mainCamera->AddComponent<Camera>();

		Camera* cam = mainCamera->GetComponent<Camera>();
		cam->SetView(lookAt(Vec3(2.0f, 2.0f, 2.0f), Vec3(0.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f)));

		GameObject* model = new GameObject();
		model->AddComponent<Transform>();
		model->AddComponent<MeshRenderer>();
		
		Mesh* mesh = new Mesh();
		renderer->InitMesh(mesh);
		//mesh->LoadFromFile(CUBE_MODEL_NAME);
		mesh->LoadFromFile(SPONZA_MODEL_NAME);
		MeshRenderer* cubeRenderer = model->GetComponent<MeshRenderer>();
		cubeRenderer->SetMesh(mesh);
		
		scene->Add(model);

		while(!glfwWindowShouldClose(window)) {
			glfwPollEvents();	
			static auto startTime = std::chrono::high_resolution_clock::now();
			auto currentTime = std::chrono::high_resolution_clock::now();
			float time = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 1000.0f;

			Transform* modelTransform = model->GetComponent<Transform>();

			//modelTransform->SetRotation(rotate(glm::quat(), time * glm::radians(90.0f), Vec3(0.0f, 0.0f, 1.0f)));			
			renderer->PrepareFrame();
			renderer->PresentFrame();
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
