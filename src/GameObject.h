#pragma once

#include <cstdint>
#include <string>
#include "MeshRenderer.h"
#include "Transform.h"
#include "Camera.h"

class GameObject {
public:
	static const uint32_t InvalidComponentIndex = 99999999;

	template<class T>
	void AddComponent() {
		const uint32_t index = GetNextComponentIndex();
		if(index != InvalidComponentIndex) {
			mComponents[index].handle = T::New();
			mComponents[index].type = T::Type();
			GetComponent<T>()->pObj = this;
		}
	}

	template<class T>
	void RemoveComponent() {
		for(uint32_t i = 0; i < mNextFreeComponentIndex; ++i) {
			if(mComponents[i].type == T::Type()) {
				GetComponent<T>()->pObj = nullptr;
				mComponents[i].handle = 0;
				mComponents[i].type = -1;
				return;
			}
		}
	}

	template<class T> T* GetComponent() const {
		for(const auto& component : mComponents) {
			if(T::Type() == component.type) {
				return T::Get(component.handle);
			}
		}

		return nullptr;
	}

	GameObject() = default;
	GameObject(const GameObject& other);
	GameObject& operator=(const GameObject& other);
	void SetName(const char* name) { mName = name; }
	void SetEnabled(bool enabled) { mIsEnabled = enabled; }
	bool IsEnabled() const;
	const std::string& GetName() const { return mName; }
	void SetLayer(uint32_t layer) { mLayer = layer; }
	uint32_t GetLayer() const { return mLayer; }

private:
	struct ComponentEntry {
		int type = -1;
		unsigned handle = 0;
	};

	static const int MaxComponents = 16;

	uint32_t GetNextComponentIndex();
	uint32_t mNextFreeComponentIndex = 0;
	ComponentEntry mComponents[MaxComponents];
	std::string mName;
	uint32_t mLayer = 1;
	bool mIsEnabled = true;
};
