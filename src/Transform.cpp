#include "Transform.h"
#include "GameObject.h"
#include "Camera.h"

uint32_t Transform::New() {
	return Components::TransformComponents.NewHandle();
}

Transform* Transform::Get(uint32_t index) {
	return Components::TransformComponents.Get(index);
}

void Transform::LookAt(const Vec3& localPosition, const Vec3& center, const Vec3& up) {
	mRotation = lookAt(localPosition, center, up);
	mPosition = localPosition;
}

void Transform::SetParent(Transform* parent) {
	using namespace Components;
	const Transform* comp = parent;
	while(comp != nullptr) {
		if(comp == this) {
			return;
		}

		comp = comp->mParent == -1 ? nullptr : &TransformComponents[comp->mParent];
	}

	for(uint32_t componentIndex = 0; componentIndex < TransformComponents.size(); ++componentIndex) {
		if(&TransformComponents[componentIndex] == parent) {
			mParent = static_cast<int>(componentIndex);
			return;
		}
	}
}

Transform* Transform::GetParent() const {
	using namespace Components;
	return mParent == -1 ? nullptr : &TransformComponents[mParent];
}

Vec3 Transform::GetForward() const {
	Vec3 direction = { mModel[0][2], mModel[1][2], mModel[2][2] };
	return direction;
}

Vec3 Transform::GetUp() const {
	Vec3 up = { mModel[0][1], mModel[1][1], mModel[2][1] };
	return up;
}

Vec3 Transform::GetRight() const {
	Vec3 right = { mModel[0][0], mModel[1][0], mModel[2][0] };
	return right;
}

void Transform::UpdateLocalMatrices() {
	using namespace Components;

	uint32_t nextFreeTransformComponent = TransformComponents.GetNextFreeIndex();

	for(uint32_t componentIndex = 0; componentIndex < nextFreeTransformComponent; ++componentIndex) {
		TransformComponents[componentIndex].SolveLocalMatrix();
	}

	for(uint32_t componentIndex = 0; componentIndex < nextFreeTransformComponent; ++componentIndex) {
		int parent = TransformComponents[componentIndex].mParent;
		Matrix transform = TransformComponents[componentIndex].mModel;
		Quaternion worldRotation = TransformComponents[componentIndex].mRotation;
		Vec3 worldPosition = TransformComponents[componentIndex].mPosition;

		while(parent != -1) {
			transform = TransformComponents[parent].GetModel() * transform;
			worldRotation = worldRotation * TransformComponents[parent].mRotation;
			worldPosition = worldPosition * TransformComponents[parent].mPosition;

			parent = TransformComponents[parent].mParent;
		}

		TransformComponents[componentIndex].mWorldModel = transform;
		TransformComponents[componentIndex].mWorldPosition = worldPosition;
		TransformComponents[componentIndex].mWorldRotation = worldRotation;

		if(TransformComponents[componentIndex].pObj) {
			Camera* camera = TransformComponents[componentIndex].pObj->GetComponent<Camera>();
			if(camera) {
				Transform& trans = TransformComponents[componentIndex];
				Vec3 eye = trans.mWorldPosition;
				Vec3 center = trans.GetForward();
				Vec3 up = trans.GetUp();
				camera->SetView(lookAt(eye, center, up));
			}
		}
	}
}

void Transform::SolveLocalMatrix() {
	Matrix rotation = mat4_cast(mRotation);
	Matrix scale;
	glm::scale(scale, Vec3(mLocalScale, mLocalScale, mLocalScale));
	Matrix translation;
	translate(translation, mPosition);
	mModel = scale * translation * rotation;
}