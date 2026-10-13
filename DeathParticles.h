#pragma once
#include "KamataEngine.h"
#include <array>

struct Transform {
	KamataEngine::Vector3 scale;
	KamataEngine::Vector3 rotate;
	KamataEngine::Vector3 translate;
};

KamataEngine::Matrix4x4 MakeIdentity4x4();

KamataEngine::Matrix4x4 MakeRotateZMatrix(float radian);

/// <summary>
/// デス選出用パーティクル
/// </summary>
class DeathParticles {
public:
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);

	void Update();

	void Draw();

	// デスフラグのgetter
	bool IsFinished() const { return isFinished_; }

private:
	// モデル
	KamataEngine::Model* model_ = nullptr;

	KamataEngine::Camera* camera_ = nullptr;

	// パーティクルの個数
	static inline const uint32_t kNumParticles = 8;

	std::array<KamataEngine::WorldTransform, kNumParticles> worldTransforms_;

	// 存続時間
	static inline const float kDuration = 1.0f;
	// 移動の速さ
	static inline const float kSpeed = 0.1f;
	// 分裂した一個分の角度
	static inline const float kAngleUnit = 3.141592654f * 2.0f / kNumParticles;

	// 終了フラグ
	bool isFinished_ = false;
	// 経過時間カウント
	float counter_ = 0.0f;

	// 色変更オブジェクト
	KamataEngine::ObjectColor objectColor_;
	// 色の数値
	KamataEngine::Vector4 color_;
};
