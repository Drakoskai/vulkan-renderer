#include "Camera.h"

ComponentFactory<Camera>& Camera::GetCameras() {
	return Components::CameraComponents;
}

void Camera::SetViewPort(int x, int y, int width, int height) {
	viewport_.x = x;
	viewport_.y = y;
	viewport_.width = width;
	viewport_.height = height;
}

void Camera::SetProjection(float left, float right, float bottom, float top, float n, float f) {
	projection_ = ProjectionType::Orthographic;
	orthoProperties_.left = left;
	orthoProperties_.right = right;
	orthoProperties_.bottom = bottom;
	orthoProperties_.top = top;
	near_ = n;
	far_ = f;

	proj_ = glm::ortho(left, right, bottom, top, n, f);
}

void Camera::SetProjection(float fovDegrees, float aspect, float n, float f) {
	projection_ = ProjectionType::Perspective;
	proj_ = glm::perspective(fovDegrees, aspect, n, f);
	proj_[1][1] *= -1;
}

void Camera::SetProjection(float fovDegrees, float w, float h, float n, float f) {
	projection_ = ProjectionType::Perspective;
	proj_ = glm::perspective(fovDegrees, static_cast<float>(w) / static_cast<float>(h), n, f);
	proj_[1][1] *= -1;
}

void Camera::SetProjection(const Matrix& proj) { proj_ = proj; }

Matrix Camera::GetView() const { return view_; }

Camera::ProjectionType Camera::GetProjectionType() const { return projection_; }

GameObject* Camera::GetGameObject() const { return pObj_; }

Vec3 Camera::GetForward() const {
	Vec3 direction = { view_[0][2], view_[1][2], view_[2][2] };
	return direction;
}

Vec3 Camera::GetUp() const {
	Vec3 up = { view_[1][0], view_[1][1], view_[1][2] };
	return up;
}

Vec3 Camera::GetRight() const {
	Vec3 right = { view_[0][0], view_[0][1], view_[0][2] };
	return right;
}

Matrix Camera::GetProj() const { return proj_; }

void Camera::SetView(const Matrix& view) { view_ = view; }

Matrix Camera::GetViewProj() const { return  proj_ * view_; }

uint32_t Camera::New() {
	using namespace Components;

	return CameraComponents.NewHandle();
}

Camera* Camera::Get(uint32_t index) {
	using namespace Components;

	return CameraComponents.Get(index);
}
