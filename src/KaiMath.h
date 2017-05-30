#ifndef KAI_MATH_H__
#define KAI_MATH_H__

#ifndef GLM_FORCE_RADIANS
#define GLM_FORCE_RADIANS
#endif
#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif
#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif

#include <glm/glm.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/detail/type_vec3.hpp>
#include <glm/gtx/hash.hpp>
#include <vector>

typedef glm::vec4 Vec4;
typedef glm::vec3 Vec3;
typedef glm::vec2 Vec2;
typedef glm::mat4 Matrix;
typedef glm::quat Quaternion;

static const float DefaultEpsilon = 0.0001f;

static uint16_t Min(uint16_t a, uint16_t b) {
	return a < b ? a : b;
}
static Vec3 Min(const Vec3& a, const Vec3& b) {
	return all(lessThan(a, b)) ? a : b;
}

static Vec3 Max(const Vec3& a, const Vec3& b) {
	return all(greaterThan(a, b)) ? a : b;
}

static bool IsEqual(float f1, float f2, float epsilon) {
	return abs(f1 - f2) < epsilon;
}

static bool IsEqual(float f1, float f2) {
	return IsEqual(f1, f2, DefaultEpsilon);
}

static bool IsEqual(const Vec2& v1, const Vec2& v2) {
	return IsEqual(v1.x, v2.x) && IsEqual(v1.y, v2.y);
}

static bool IsEqual(const Vec3& v1, const Vec3& v2) {
	return IsEqual(v1.x, v2.x) && IsEqual(v1.y, v2.y) && IsEqual(v1.z, v2.z);
}

static bool IsEqual(const Vec4& v1, const Vec4& v2) {
	return IsEqual(v1.x, v2.x) && IsEqual(v1.y, v2.y) && IsEqual(v1.z, v2.z) && IsEqual(v1.w, v2.w);
}

static uint32_t GetHash(const char* s, uint32_t length) {
	const uint32_t A = 54059;
	const uint32_t B = 76963;

	uint32_t h = 31;
	uint32_t i = 0;

	while (i < length) {
		h = (h * A) ^ (s[0] * B);
		++s;
		++i;
	}

	return h;
}

static uint32_t GetHash(std::string s) {

	return GetHash(s.c_str(), static_cast<uint32_t>(s.size()));
}

static void GetMinMax(const std::vector<Vec3>& points, Vec3& outMin, Vec3& outMax) {
	if (!points.empty()) {
		outMin = points[0];
		outMax = points[0];
	}

	for (size_t i = 1, s = points.size(); i < s; ++i) {
		const Vec3& point = points[i];

		if (point.x < outMin.x) {
			outMin.x = point.x;
		}

		if (point.y < outMin.y) {
			outMin.y = point.y;
		}

		if (point.z < outMin.z) {
			outMin.z = point.z;
		}

		if (point.x > outMax.x) {
			outMax.x = point.x;
		}

		if (point.y > outMax.y) {
			outMax.y = point.y;
		}

		if (point.z > outMax.z) {
			outMax.z = point.z;
		}
	}
}

static void GetCorners(const Vec3& min, const Vec3& max, std::vector<Vec3>& outCorners) {
	outCorners = {
		Vec3(min.x, min.y, min.z),
		Vec3(max.x, min.y, min.z),
		Vec3(min.x, max.y, min.z),
		Vec3(min.x, min.y, max.z),
		Vec3(max.x, max.y, min.z),
		Vec3(min.x, max.y, max.z),
		Vec3(max.x, max.y, max.z),
		Vec3(max.x, min.y, max.z)
	};
}
#endif
