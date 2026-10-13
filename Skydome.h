#pragma once
#include "KamataEngine.h"

/// <summary>
/// 天球
/// </summary>
class skydome {

public:
	/// 初期化
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera);

	/// 更新
	void Update();

	/// 描画
	void Draw();

private:
	// ワールド変換データ
	KamataEngine::WorldTransform worldTransform_;
	// モデル
	KamataEngine::Model* model_ = nullptr;

	// カメラ
	KamataEngine::Camera* camera_ = nullptr;
};
