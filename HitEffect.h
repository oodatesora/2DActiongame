#pragma once
#include <KamataEngine.h>

/// <summary>
/// ヒット演出用エフェクト
/// </summary>
class HitEffect {
public:
	enum class Phase { ScaleUp, FadeOut, Dead };

	/// <summary>
	/// エフェクト初期化
	/// </summary>
	void Initialize(const KamataEngine::Vector3& position);

	/// <summary>
	/// エフェクト更新
	/// </summary>
	void Update();

	/// <summary>
	/// エフェクト描画
	/// </summary>
	void Draw();

	static void SetModel(KamataEngine::Model* model);
	static void SetCamera(KamataEngine::Camera* camera);

	std::array<KamataEngine::WorldTransform, 4> ellipseWorldTransforms_;

	// インスタンス生成と初期化
	static HitEffect* Create(const KamataEngine::Vector3& position);
	// デスフラグの取得
	bool IsDead() const { return phase_ == Phase::Dead; }

private:
	// モデル（借りてくるよう）
	static KamataEngine::Model* model_;
	// カメラ（借りてくるよう）
	static KamataEngine::Camera* camera_;

	// 円のワールドたランスフォーム
	KamataEngine::WorldTransform circleWorldTransform_;

	Phase phase_ = Phase::ScaleUp;
	float counter_ = 0.0f;
	float scale_ = 0.1f;

	const float kScaleUpDuration = 0.15f; // 拡大にかける時間
	const float kFadeOutDuration = 0.20f; // 縮小/フェードにかける時間

	const float kStartScale = 0.1f;
	const float kMaxScale = 2.0f;

	// 色変更オブジェクト
	KamataEngine::ObjectColor objectColor_;
	// 色の数値
	KamataEngine::Vector4 color_;
};
