#pragma once
#include "KamataEngine.h"
#include "Player.h"

// 前方宣言
class Player;

struct Rect {
	float left;   // 左端
	float right;  // 右端
	float bottom; // 下端（
	float top;    // 上端
};
/// <summary>
/// カメラコントロール
/// </summary>
class CameraController {
public:
	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize(KamataEngine::Camera* camera);

	/// <summary>
	/// 更新
	/// </summary>
	void Update();

	void SetTarget(Player* target) { target_ = target; }

	void Reset();

	void SetMovableArea(const Rect& area) { movableArea_ = area; }

private:
	// カメラ
	KamataEngine::Camera* camera_ = nullptr;

	Player* target_ = nullptr;

	// 追従対象とカメラの座標の差（オフセット）
	KamataEngine::Vector3 targetOffset_ = {0.0f, 2.0f, -10.0f};

	// カメラ移動範囲
	Rect movableArea_ = {-100.0f, 1000.0f, -100.0f, 1000.0f};

	// カメラの目標座標
	KamataEngine::Vector3 targetPosition_;

	static inline const float kInterpolationRate = 0.1f;

	// 速度掛け率
	static inline const float kVelocityBias = 30.0f;

	// 追従対象の各方向へのカメラ移動範囲
	static inline const Rect kTargetDirectionMovableArea = {-100.0f, 1000.0f, -100.0f, 1000.0f};
};
