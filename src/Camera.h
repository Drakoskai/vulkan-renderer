#ifndef CAMERA_H_
#define CAMERA_H_

#include "KaiMath.h"
#include "Component.h"

class Camera {
public:
	static ComponentFactory<Camera>& GetCameras();
	void SetProjection(float left, float right, float bottom, float top, float n, float f);
	void SetProjection(float fovDegrees, float aspect, float n, float f);
	void SetProjection(float fovDegrees, float w, float h, float n, float f);
	void SetProjection(const Matrix& proj);
	Matrix GetProj() const;
	void SetView(const Matrix& view);
	Matrix GetView() const;

	class GameObject* GetGameObject() const;
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

	static int Type() { return 0; }
	static uint32_t New();
	static Camera* Get(uint32_t index);

	float foVd_ = 45.0f;
	float near_ = 0;
	float far_ = 1.0f;
	float aspect_ = 1.0f;

	float left_ = 0.0f;
	float right_ = 100.0f;
	float top_ = 0.0f;
	float bottom_ = 100.0f;

	Matrix view_;
	Matrix proj_;

	GameObject* pObj_ = nullptr;
};

namespace Components {
	static ComponentFactory<Camera> CameraComponents;
}
#endif
