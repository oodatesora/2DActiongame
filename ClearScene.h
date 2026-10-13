#pragma once
#include "Fade.h"
#include "Player.h"
#include "WorldTransformEx.h"
#include "skydome.h"
#include <KamataEngine.h>

class ClearScene {
public:
	ClearScene() = default;
	~ClearScene() = default;

	/// <summary>
	/// 初期化
	/// </summary>
	void Initialize();

	/// <summary>
	/// 毎フレーム更新
	/// </summary>
	void Update();

	/// <summary>
	/// 描画
	/// </summary>
	void Draw();

	/// <summary>
	/// 終了処理
	/// </summary>
	void Finalize();

	/// <summary>
	/// シーン終了フラグの取得
	/// </summary>
	bool IsFinished() const { return finished_; }

private:
	// カメラ
	KamataEngine::Camera camera_;

	// 3Dモデル関連
	KamataEngine::Model* clearFontModel_ = nullptr;
	KamataEngine::WorldTransform* clearFontWorldTransform_ = nullptr;

	KamataEngine::Model* playerModel_ = nullptr;
	KamataEngine::WorldTransform* playerWorldTransform_ = nullptr;

	KamataEngine::Model* modelSkydome_ = nullptr;
	skydome* skydome_ = nullptr;

	// 位置設定
	KamataEngine::Vector3 clearFontPosition_ = {0.0f, 2.0f, 0.0f};
	KamataEngine::Vector3 playerPosition_ = {0.0f, -1.0f, 0.0f};

	// フェード処理
	Fade* fade_ = nullptr;
	float fadeTime_ = 1.0f;

	// シーン終了フラグ
	bool finished_ = false;

	uint32_t soundHandleBGM_ = 0;
	uint32_t voiceHandleBGM_ = 0;
};