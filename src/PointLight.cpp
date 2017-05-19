#include "PointLight.h"

uint32_t PointLight::New() {
	return Components::PointLightComponents.NewHandle();
}

PointLight* PointLight::Get(uint32_t index)
{
	return Components::PointLightComponents.Get(index);
}

GameObject* PointLight::GetGameObject() const { return pObj_; }
