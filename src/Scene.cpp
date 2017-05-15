#include "Scene.h"
#include "Transform.h"
#include "Camera.h"
#include "MeshRenderer.h"

Scene::Scene(IRenderer* renderer) : nextFreeGameObject(0), pRenderer(renderer) { pRenderer->Init(); }

Scene::~Scene() { }

void Scene::Add(GameObject* gameObj) {
	for(const auto& go : gameObjects) {
		if(go == gameObj) {
			return;
		}
	}

	if(nextFreeGameObject >= gameObjects.size()) {
		gameObjects.resize(gameObjects.size() + 10);
	}

	gameObjects[nextFreeGameObject++] = gameObj;
}

void Scene::Remove(GameObject* gameObj) {
	for(size_t i = 0; i < gameObjects.size(); ++i) {
		if(gameObj == gameObjects[i]) {
			gameObjects.erase(begin(gameObjects) + i);
			return;
		}
	}
}

void Scene::Frame() {
	Transform::UpdateLocalMatrices();

	std::vector<GameObject*> cameras;
	std::vector<GameObject*> renderables;
	for(auto gameObject : gameObjects) {
		if(!gameObject || !gameObject->IsEnabled()) {
			continue;
		}

		auto cameraComp = gameObject->GetComponent<Camera>();
		auto transformComp = gameObject->GetComponent<Transform>();
		auto renderable = gameObject->GetComponent<MeshRenderer>();
		if(cameraComp && transformComp) {
			cameras.push_back(gameObject);
		} else if(renderable && transformComp) {
			renderables.push_back(gameObject);
		}
	}

	for(auto camera : cameras) {
		auto cameraComp = camera->GetComponent<Camera>();
		Matrix view = cameraComp->GetView();
		Matrix proj = cameraComp->GetProj();
		for(auto renderable : renderables) {
			auto meshRenderer = renderable->GetComponent<MeshRenderer>();
			auto transform = renderable->GetComponent<Transform>();
			Matrix model = transform->GetModelToWorldMatrix();
			//meshRenderer->Render(model, view, proj);
		}
	}
}
