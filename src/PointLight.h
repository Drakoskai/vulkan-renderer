#ifndef POINT_LIGHT_H_
#define POINT_LIGHT_H_

#include <cstdint>
#include "KaiMath.h"
#include "Component.h"

class PointLight {
public:
	class GameObject* GetGameObject() const;
	PointLight() = default;
	void SetColor(Vec4 color) { color_ = color; }
	void SetRadius(float radius) { radius_ = radius; }
	Vec4 GetColor() const { return color_; }
	float GetRadius() const { return radius_; }
private:
	friend class GameObject;
	static int Type() { return 6; }
	static uint32_t New();
	static PointLight* Get(uint32_t index);

	Vec4 color_ = Vec4(1.0f);
	float radius_ = 10;

	GameObject* pObj_ = nullptr;
};

namespace Components {
	static ComponentFactory<PointLight> PointLightComponents;
}
#endif
