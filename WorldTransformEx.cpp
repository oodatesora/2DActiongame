#include "WorldTransformEx.h"
#include <cmath>

using namespace KamataEngine;

// アフィン行列の計算
Matrix4x4 MakeAffineMatrix(Vector3 scale, Vector3 rotation, Vector3 translation) {
	Matrix4x4 matrix{};

	float cosX = cosf(rotation.x);
	float sinX = sinf(rotation.x);
	float cosY = cosf(rotation.y);
	float sinY = sinf(rotation.y);
	float cosZ = cosf(rotation.z);
	float sinZ = sinf(rotation.z);

	matrix.m[0][0] = scale.x * (cosY * cosZ);
	matrix.m[0][1] = scale.x * (cosY * sinZ);
	matrix.m[0][2] = scale.x * (-sinY);
	matrix.m[0][3] = 0.0f;

	matrix.m[1][0] = scale.y * (sinX * sinY * cosZ - cosX * sinZ);
	matrix.m[1][1] = scale.y * (sinX * sinY * sinZ + cosX * cosZ);
	matrix.m[1][2] = scale.y * (sinX * cosY);
	matrix.m[1][3] = 0.0f;

	matrix.m[2][0] = scale.z * (cosX * sinY * cosZ + sinX * sinZ);
	matrix.m[2][1] = scale.z * (cosX * sinY * sinZ - sinX * cosZ);
	matrix.m[2][2] = scale.z * (cosX * cosY);
	matrix.m[2][3] = 0.0f;

	matrix.m[3][0] = translation.x;
	matrix.m[3][1] = translation.y;
	matrix.m[3][2] = translation.z;
	matrix.m[3][3] = 1.0f;

	return matrix;
}

void UpdateWorldTransform(WorldTransform& worldTransform) {
	// スケール、回転、平行移動を合成して行列を計算する
	worldTransform.matWorld_ = MakeAffineMatrix(worldTransform.scale_, worldTransform.rotation_, worldTransform.translation_);

	// 定数バッファへの書き込み
	worldTransform.TransferMatrix();
}

// X軸回転行列を作成する関数
Matrix4x4 MakeRotateXMatrix(float angle) {
	Matrix4x4 matrix{};
	float c = cosf(angle);
	float s = sinf(angle);

	matrix.m[0][0] = 1.0f;
	matrix.m[1][1] = c;
	matrix.m[1][2] = s;
	matrix.m[2][1] = -s;
	matrix.m[2][2] = c;
	matrix.m[3][3] = 1.0f;

	return matrix;
}

// 行列どうしの掛け算関数
Matrix4x4 MultiplyMatrix(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result{};
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			result.m[i][j] = m1.m[i][0] * m2.m[0][j] + m1.m[i][1] * m2.m[1][j] + m1.m[i][2] * m2.m[2][j] + m1.m[i][3] * m2.m[3][j];
		}
	}
	return result;
}
