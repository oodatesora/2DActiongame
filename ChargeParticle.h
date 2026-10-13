#pragma once
#include <KamataEngine.h>

/// <summary>
/// チャージ中のパーティクル
/// </summary>
class ChargeParticle {
public:
	// 初期化
	void Initialize(const KamataEngine::Vector3& position);

	// 更新
	void Update();

	// 描画
	void Draw();

	// モデル設定
	static void SetModel(KamataEngine::Model* model);

	// カメラ設定
	static void SetCamera(KamataEngine::Camera* camera);

	// 生成と初期化
	static ChargeParticle* Create(const KamataEngine::Vector3& position);

	// 消滅フラグの取得
	bool IsDead() const { return isDead_; }

	void SetTargetPosition(const KamataEngine::Vector3& position);
	
private:
	// モデル
	static KamataEngine::Model* model_;

	// カメラ
	static KamataEngine::Camera* camera_;

	// ワールドトランスフォーム
	KamataEngine::WorldTransform worldTransform_;

	// 初期位置
	KamataEngine::Vector3 startPosition_;

	// 移動先
	KamataEngine::Vector3 targetPosition_;

	// 寿命
	float counter_ = 0.0f;

	// パーティクルが消えるまでの時間
	const float kLifeTime = 0.5f;

	// 大きさ
	float scale_ = 0.2f;

	// 消滅フラグ
	bool isDead_ = false;
};