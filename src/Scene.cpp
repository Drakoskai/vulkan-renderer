#include "Scene.h"
#include "Transform.h"
#include "Camera.h"
#include "MeshRenderer.h"

Scene::Scene(IRenderer* renderer) : nextFreeGameObject_(0), pRenderer_(renderer) { pRenderer_->Init(); }

Scene::~Scene() { }

void Scene::Add(GameObject* gameObj) {
	for(const auto& go : gameObjects_) {
		if(go == gameObj) {
			return;
		}
	}

	if(nextFreeGameObject_ >= gameObjects_.size()) {
		gameObjects_.resize(gameObjects_.size() + 10);
	}

	gameObjects_[nextFreeGameObject_++] = gameObj;
}

void Scene::Remove(GameObject* gameObj) {
	for(size_t i = 0; i < gameObjects_.size(); ++i) {
		if(gameObj == gameObjects_[i]) {
			gameObjects_.erase(begin(gameObjects_) + i);
			return;
		}
	}
}

void Scene::Frame() {
	Transform::UpdateLocalMatrices();

	std::vector<GameObject*> cameras;
	std::vector<GameObject*> renderables;
	for(auto gameObject : gameObjects_) {
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
