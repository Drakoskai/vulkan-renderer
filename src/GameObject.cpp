#include "GameObject.h"
#include "Camera.h"
#include "MeshRenderer.h"
#include "Transform.h"

GameObject::GameObject(const GameObject& other) {
	*this = other;
}

GameObject& GameObject::operator=(const GameObject& other) {
	name_ = other.name_;

	for(uint64_t i = 0; i < MaxComponents; ++i) {
		components_[i].type = -1;
		components_[i].handle = InvalidComponentIndex;
	}

	if(other.GetComponent<Transform>()) {
		AddComponent<Transform>();
		*GetComponent<Transform>() = *other.GetComponent<Transform>();
	}

	if(other.GetComponent<MeshRenderer>()) {
		AddComponent<MeshRenderer>();
		*GetComponent<MeshRenderer>() = *other.GetComponent<MeshRenderer>();
	}

	if(other.GetComponent<Camera>()) {
		AddComponent<Camera>();
		*GetComponent<Camera>() = *other.GetComponent<Camera>();
	}

	return *this;
}

bool GameObject::IsEnabled() const {
	const Transform* transform = GetComponent<Transform>();
	while(transform) {
		if(transform->GetGameObject() && !transform->GetGameObject()->isEnabled_) {
			return false;
		}

		transform = transform->GetParent();
	}

	return isEnabled_;
}

uint32_t GameObject::GetNextComponentIndex() {
	return nextFreeComponentIndex_ >= MaxComponents ? InvalidComponentIndex : nextFreeComponentIndex_++;
}
