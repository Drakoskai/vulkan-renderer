#ifndef TRANSFORM_H_
#define TRANSFORM_H_

#include "Component.h"
#include "KaiMath.h"

class Transform {
public:
	static void UpdateLocalMatrices();

	const Vec3& GetLocalPosition() const { return pos_; }
	const Quaternion& GetLocalRotation() const { return rot_; }
	const Matrix& GetModel() const { return model_; }
	const Matrix& GetModelToWorldMatrix() const { return world_; }
	float GetLocalScale() const { return scale_; }
	const Vec3& GetWorldPosition() const { return worldPos_; }
	const Quaternion& GetWorldRotation() const { return worldRot_; }
	void LookAt(const Vec3& eye, const Vec3& center, const Vec3& up);
	void SetPosition(const Vec3 position) { pos_ = position; }
	void SetRotation(const Quaternion rotation) { rot_ = rotation; }
	void SetScale(float scale) { scale_ = scale; }
	void SetParent(Transform* parent);
	Transform* GetParent() const;
	Vec3 GetForward() const;
	Vec3 GetUp() const;
	Vec3 GetRight() const;
	class GameObject* GetGameObject() const { return pObj_; }
	Transform() = default;
private:
	friend class GameObject;
	static int Type() { return 2; }
	static uint32_t New();
	static Transform* Get(uint32_t index);
	
	void SolveLocalMatrix();

	Vec3 pos_ { };
	Vec3 worldPos_ { };
	Quaternion rot_ { };
	Quaternion worldRot_ { };
	Matrix model_ { };
	Matrix world_ { };
	int parent_ = -1;
	float scale_ = 1.0f;

	GameObject* pObj_ = nullptr;
};

namespace Components {
	static ComponentFactory<Transform> TransformComponents;
}
#endif
