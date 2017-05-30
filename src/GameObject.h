#ifndef GAME_OBJECT_H__
#define GAME_OBJECT_H__

#include <cstdint>
#include "Transform.h"

class GameObject {
public:
	static const uint32_t InvalidComponentIndex = 99999999;

	template<class T>
	void AddComponent() {
		const uint32_t index = GetNextComponentIndex();
		if(index != InvalidComponentIndex) {
			components_[index].handle = T::New();
			components_[index].type = T::Type();
			GetComponent<T>()->pObj_ = this;
		}
	}

	template<class T>
	void RemoveComponent() {
		for(uint32_t i = 0; i < nextFreeComponentIndex_; ++i) {
			if(components_[i].type == T::Type()) {
				GetComponent<T>()->pObj_ = nullptr;
				components_[i].handle = 0;
				components_[i].type = -1;
				return;
			}
		}
	}

	template<class T> T* GetComponent() const {
		for(const auto& component : components_) {
			if(T::Type() == component.type) {
				return T::Get(component.handle);
			}
		}

		return nullptr;
	}

	GameObject() = default;
	GameObject(const GameObject& other);
	GameObject& operator=(const GameObject& other);
	void SetName(const char* name) { name_ = name; }
	void SetEnabled(bool enabled) { isEnabled_ = enabled; }
	bool IsEnabled() const;
	const std::string& GetName() const { return name_; }
	void SetLayer(uint32_t layer) { layer_ = layer; }
	uint32_t GetLayer() const { return layer_; }

private:
	struct ComponentEntry {
		int type = -1;
		uint32_t handle = 0;
	};

	static const int MaxComponents = 16;
	uint32_t GetNextComponentIndex();

	uint32_t nextFreeComponentIndex_ = 0;
	ComponentEntry components_[MaxComponents];
	std::string name_;
	uint32_t layer_ = 1;
	bool isEnabled_ = true;
};
#endif
