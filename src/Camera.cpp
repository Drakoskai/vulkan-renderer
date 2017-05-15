#include "Camera.h"

void Camera::SetViewPort(int x, int y, int width, int height) {
	mViewport.x = x;
	mViewport.y = y;
	mViewport.width = width;
	mViewport.height = height;
}

void Camera::SetProjection(float left, float right, float bottom, float top, float n, float f) {
	mProjection = ProjectionType::Orthographic;
	mOrthoProperties.left = left;
	mOrthoProperties.right = right;
	mOrthoProperties.bottom = bottom;
	mOrthoProperties.top = top;
	mNear = n;
	mFar = f;

	mProj = glm::ortho(left, right, bottom, top, n, f);
}

void Camera::SetProjection(float fovDegrees, float aspect, float n, float f) {
	mProjection = ProjectionType::Perspective;
	mProj = glm::perspective(fovDegrees, aspect, n, f);
	mProj[1][1] *= -1;
}

void Camera::SetProjection(float fovDegrees, float w, float h, float n, float f) {
	mProjection = ProjectionType::Perspective;
	mProj = glm::perspective(fovDegrees, static_cast<float>(w) / static_cast<float>(h), n, f);
	mProj[1][1] *= -1;
}

void Camera::SetProjection(const Matrix& proj) { mProj = proj; }

Matrix Camera::GetView() const { return mView; }

Camera::ProjectionType Camera::GetProjectionType() const { return mProjection; }

GameObject* Camera::GetGameObject() const { return pObj; }

Vec3 Camera::GetForward() const {
	Vec3 direction = { mView[0][2], mView[1][2], mView[2][2] };
	return direction;
}

Vec3 Camera::GetUp() const {
	Vec3 up = { mView[1][0], mView[1][1], mView[1][2] };
	return up;
}

Vec3 Camera::GetRight() const {
	Vec3 right = { mView[0][0], mView[0][1], mView[0][2] };
	return right;
}

Matrix Camera::GetProj() const { return mProj; }

void Camera::SetView(const Matrix& view) { mView = view; }

Matrix Camera::GetViewProj() const { return  mProj * mView; }

uint32_t Camera::New() {
	using namespace Components;

	return CameraComponents.NewHandle();
}

Camera* Camera::Get(uint32_t index) {
	using namespace Components;

	return CameraComponents.Get(index);
}
