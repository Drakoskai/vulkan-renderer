#pragma once

#include <vector>

template<class T>
class ComponentFactory {
public:
	ComponentFactory() { }
	~ComponentFactory() { }
	ComponentFactory(const ComponentFactory& other) = delete;
	const T& operator[](const int index) const;
	T& operator[](const int index);
	uint32_t NewHandle();
	T* Get(uint32_t index);
	size_t size();
	uint32_t GetNextFreeIndex() const;

private:
	std::vector<T> Components;
	uint32_t nextFreeComponent = 0;

};

template <class T>
const T& ComponentFactory<T>::operator[](const int index) const {
	return Components[index];
}

template <class T>
T& ComponentFactory<T>::operator[](const int index) {
	return Components[index];
}

template <class T>
uint32_t ComponentFactory<T>::NewHandle() {
	if(nextFreeComponent == Components.size()) {
		size_t size = Components.size();
		Components.resize(size + 8);
	}

	return nextFreeComponent++;
}

template <class T>
T* ComponentFactory<T>::Get(uint32_t index) {
	return &Components[index];
}

template <class T>
size_t ComponentFactory<T>::size() {
	return Components.size();
}

template <class T>
uint32_t ComponentFactory<T>::GetNextFreeIndex() const {
	return nextFreeComponent;
}
