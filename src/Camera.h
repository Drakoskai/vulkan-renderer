#pragma once

#include "KaiMath.h"
#include "Component.h"
#include "GraphicsStructs.h"

class Camera {
public:
	enum class ProjectionType { Orthographic, Perspective };

	void SetViewPort(int x, int y, int width, int height);
	void SetProjection(float left, float right, float bottom, float top, float n, float f);
	void SetProjection(float fovDegrees, float aspect, float n, float f);
	void SetProjection(float fovDegrees, float w, float h, float n, float f);
	void SetProjection(const Matrix& proj);
	Matrix GetProj() const;
	void SetView(const Matrix& view);
	Matrix GetView() const;
	ProjectionType GetProjectionType() const;

	class GameObject* GetGameObject() const;
	void SetRenderOrder(uint32_t renderOrder) { renderOrder_ = renderOrder; }
	uint32_t GetRenderOrder() const { return renderOrder_; }
	Vec3 GetForward() const;
	Vec3 GetUp() const;
	Vec3 GetRight() const;
	float GetNear() const { return near_; }
	float GetFar() const { return far_; }
	float GetAspect() const { return aspect_; }
	float GetFovDegrees() const { return foVd_; }
	Matrix GetViewProj() const;

	Camera() = default;
private:
	friend class GameObject;
	struct OrthoProperties {
		float left = 0.0f;
		float right = 100.0f;
		float top = 0.0f;
		float bottom = 100.0f;
	};

	static int Type() { return 0; }
	static uint32_t New();
	static Camera* Get(uint32_t index);

	float foVd_ = 45.0f;
	float near_ = 0;
	float far_ = 1.0f;
	float aspect_ = 1.0f;
	ProjectionType projection_ = ProjectionType::Perspective;
	OrthoProperties orthoProperties_;
	Viewport viewport_;
	Matrix view_ { };
	Matrix proj_ { };
	uint32_t renderOrder_ = 0;

	GameObject* pObj_ = nullptr;
};

namespace Components {
	static ComponentFactory<Camera> CameraComponents;
}
