#pragma once
#include "Fade.h"
#include "Player.h"
#include "WorldTransformEx.h"
#include "skydome.h"
#include <KamataEngine.h>

/// <summary>
/// タイトルシーン
/// </summary>
class TitleScene {
public:
	TitleScene() = default;
	~TitleScene() = default;

	// 初期化
	void Initialize();

	// 更新
	void Update();

	// 描画
	void Draw();

	void Finalize();

	// 終了フラグ
	bool finished_ = false;

	// デスフラグのgetter
	bool IsFinished() const { return finished_; }

	Fade* fade_ = nullptr;

	float fadeTime_ = 1.0f;

	KamataEngine::Model* modelSkydome_ = nullptr;
	skydome* skydome_ = nullptr;

private:
	// カメラ
	KamataEngine::Camera camera_;

	// 3Dのモデル
	uint32_t modelHandle_ = 0;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::WorldTransform* titleWoldTransform_;
	static inline const KamataEngine::Vector3 titleposition = {0.0f, 5.0f, -20.0f};

	// フォントのモデル
	uint32_t fontmodelHandle_ = 0;
	KamataEngine::Model* fontmodel_ = nullptr;
	KamataEngine::WorldTransform* playerWoldTransform_;
	static inline const KamataEngine::Vector3 playerPosition = {0.0f, -2.5f, -40.0f};

	float floatTimer_ = 0.0f;

	uint32_t soundHandleBGM_ = 0; // 読み込んだ音源データ用
	uint32_t voiceHandleBGM_ = 0;
};
