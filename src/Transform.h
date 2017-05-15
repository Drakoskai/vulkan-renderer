#pragma once

#include "Component.h"
#include "KaiMath.h"

class Transform {
public:
	static void UpdateLocalMatrices();

	const Vec3& GetLocalPosition() const { return mPosition; }
	const Quaternion& GetLocalRotation() const { return mRotation; }
	const Matrix& GetModel() const { return mModel; }
	const Matrix& GetModelToWorldMatrix() const { return mWorldModel; }
	float GetLocalScale() const { return mLocalScale; }
	const Vec3& GetWorldPosition() const { return mWorldPosition; }
	const Quaternion& GetWorldRotation() const { return mWorldRotation; }
	void LookAt(const Vec3& eye, const Vec3& center, const Vec3& up);
	void SetPosition(const Vec3 position) { mPosition = position; }
	void SetRotation(const Quaternion rotation) { mRotation = rotation; }
	void SetScale(float scale) { mLocalScale = scale; }
	void SetParent(Transform* parent);
	Transform* GetParent() const;
	Vec3 GetForward() const;
	Vec3 GetUp() const;
	Vec3 GetRight() const;
	class GameObject* GetGameObject() const { return pObj; }
	Transform() = default;
private:
	friend class GameObject;
	static int Type() { return 2; }
	static uint32_t New();
	static Transform* Get(uint32_t index);
	
	void SolveLocalMatrix();

	Vec3 mPosition{ };
	Vec3 mWorldPosition{ };
	Quaternion mRotation{ };
	Quaternion mWorldRotation{ };
	Matrix mModel{ };
	Matrix mWorldModel{ };
	int mParent = -1;
	float mLocalScale = 1.0f;

	GameObject* pObj = nullptr;
};

namespace Components {
	static ComponentFactory<Transform> TransformComponents;
}
