#ifndef SCENE_H__
#define SCENE_H__

#include <vector>
#include "GameObject.h"
#include "GfxTypes.h"

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
#endif
