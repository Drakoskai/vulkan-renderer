#ifndef COMPONENT_H_
#define COMPONENT_H_

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
	std::vector<T> components_;
	uint32_t nextFreeComponent_ = 0;

};

template <class T>
const T& ComponentFactory<T>::operator[](const int index) const {
	return components_[index];
}

template <class T>
T& ComponentFactory<T>::operator[](const int index) {
	return components_[index];
}

template <class T>
uint32_t ComponentFactory<T>::NewHandle() {
	if(nextFreeComponent_ == components_.size()) {
		size_t size = components_.size();
		components_.resize(size + 8);
	}

	return nextFreeComponent_++;
}

template <class T>
T* ComponentFactory<T>::Get(uint32_t index) {
	return &components_[index];
}

template <class T>
size_t ComponentFactory<T>::size() {
	return components_.size();
}

template <class T>
uint32_t ComponentFactory<T>::GetNextFreeIndex() const {
	return nextFreeComponent_;
}
#endif
