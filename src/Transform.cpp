#include "Transform.h"
#include <glm/gtc/matrix_transform.hpp>
#include "GameObject.h"
#include "Camera.h"

uint32_t Transform::New() {
	return Components::TransformComponents.NewHandle();
}

Transform* Transform::Get(uint32_t index) {
	return Components::TransformComponents.Get(index);
}

void Transform::LookAt(const Vec3& localPosition, const Vec3& center, const Vec3& up) {
	rot_ = lookAt(localPosition, center, up);
	pos_ = localPosition;
}

void Transform::SetParent(Transform* parent) {
	using namespace Components;
	const Transform* comp = parent;
	while (comp != nullptr) {
		if (comp == this) {
			return;
		}

		comp = comp->parent_ == -1 ? nullptr : &TransformComponents[comp->parent_];
	}

	for (uint32_t componentIndex = 0; componentIndex < TransformComponents.size(); ++componentIndex) {
		if (&TransformComponents[componentIndex] == parent) {
			parent_ = static_cast<int>(componentIndex);
			return;
		}
	}
}

Transform* Transform::GetParent() const {
	using namespace Components;
	return parent_ == -1 ? nullptr : &TransformComponents[parent_];
}

Vec3 Transform::GetForward() const {
	Vec3 direction = { model_[0][2], model_[1][2], model_[2][2] };
	return direction;
}

Vec3 Transform::GetUp() const {
	Vec3 up = { model_[0][1], model_[1][1], model_[2][1] };
	return up;
}

Vec3 Transform::GetRight() const {
	Vec3 right = { model_[0][0], model_[1][0], model_[2][0] };
	return right;
}

const ComponentFactory<Transform>& Transform::GetTransforms() {
	return Components::TransformComponents;
}

void Transform::UpdateLocalMatrices() {
	using namespace Components;

	uint32_t nextFreeTransformComponent = TransformComponents.GetNextFreeIndex();

	for (uint32_t componentIndex = 0; componentIndex < nextFreeTransformComponent; ++componentIndex) {
		TransformComponents[componentIndex].SolveLocalMatrix();
	}

	for (uint32_t componentIndex = 0; componentIndex < nextFreeTransformComponent; ++componentIndex) {
		int parent = TransformComponents[componentIndex].parent_;
		Matrix transform = TransformComponents[componentIndex].model_;
		Quaternion worldRotation = TransformComponents[componentIndex].rot_;
		Vec3 worldPosition = TransformComponents[componentIndex].pos_;

		while (parent != -1) {
			transform = TransformComponents[parent].GetModel() * transform;
			worldRotation = worldRotation * TransformComponents[parent].rot_;
			worldPosition = worldPosition * TransformComponents[parent].pos_;

			parent = TransformComponents[parent].parent_;
		}

		TransformComponents[componentIndex].world_ = transform;
		TransformComponents[componentIndex].worldPos_ = worldPosition;
		TransformComponents[componentIndex].worldRot_ = worldRotation;

		if (TransformComponents[componentIndex].pObj_) {
			Camera* camera = TransformComponents[componentIndex].pObj_->GetComponent<Camera>();
			if (camera) {
				Transform& trans = TransformComponents[componentIndex];
				Vec3 eye = trans.worldPos_;
				Vec3 center = trans.GetForward();
				Vec3 up = trans.GetUp();
				camera->SetView(lookAt(eye, center, up));
			}
		}
	}
}

void Transform::SolveLocalMatrix() {
	Matrix rotation = mat4_cast(rot_);
	Matrix scale;
	glm::scale(scale, Vec3(scale_, scale_, scale_));
	Matrix translation;
	translate(translation, pos_);
	model_ = scale * translation * rotation;
}
