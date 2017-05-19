#pragma once

#include "GameObject.h"
#include <vector>
#include "GraphicsStructs.h"


class Scene {
public:
	Scene(IRenderer* renderer);
	~Scene();
	void Add(GameObject* gameObj);
	void Remove(GameObject* gameObj);
	void Frame();
	
private:
	std::vector<GameObject*> gameObjects_;
	uint64_t nextFreeGameObject_;
	IRenderer* pRenderer_;
};
