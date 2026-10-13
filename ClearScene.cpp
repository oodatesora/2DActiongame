#include "ClearScene.h"
using namespace KamataEngine;

void ClearScene::Initialize() {
	clearFontWorldTransform_ = new WorldTransform();
	playerWorldTransform_ = new WorldTransform();

	// カメラの初期化
	camera_.Initialize();

	// 3Dモデルの生成（※モデル名はお持ちのファイル名に合わせて調整してください）
	clearFontModel_ = Model::CreateFromOBJ("clearFont", true);
	clearFontWorldTransform_->Initialize();
	clearFontWorldTransform_->translation_ = clearFontPosition_;
	UpdateWorldTransform(*clearFontWorldTransform_);

	playerModel_ = Model::CreateFromOBJ("player", true);
	playerWorldTransform_->Initialize();
	playerWorldTransform_->translation_ = playerPosition_;
	playerWorldTransform_->rotation_.y = 3.14159265f;
	UpdateWorldTransform(*playerWorldTransform_);

	modelSkydome_ = Model::CreateFromOBJ("clearSkydome", true);
	skydome_ = new skydome();
	skydome_->Initialize(modelSkydome_, &camera_);

	// フェードの初期化とスタート
	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, fadeTime_);

	camera_.UpdateMatrix();
	finished_ = false;

	soundHandleBGM_ = KamataEngine::Audio::GetInstance()->LoadWave("Audio/Clear.wav");
	// ループ再生不要なら第2引数は false、ループしたいなら true
	voiceHandleBGM_ = KamataEngine::Audio::GetInstance()->PlayWave(soundHandleBGM_, false, 0.4f);
}

void ClearScene::Update() {
	// SPACEキーでシーン終了
	if (Input::GetInstance()->PushKey(DIK_T)) {
		finished_ = true;
	}

	if (skydome_) {
		skydome_->Update();
	}

	fade_->Update();
}

void ClearScene::Draw() {
	// 3D描画開始
	Model::PreDraw();

	if (skydome_) {
		skydome_->Draw();
	}

	if (clearFontModel_ && clearFontWorldTransform_) {
		clearFontModel_->Draw(*clearFontWorldTransform_, camera_);
	}

	// 3D描画終了
	Model::PostDraw();

	// フェードの描画
	if (fade_) {
		fade_->Draw();
	}
}

void ClearScene::Finalize() {

	KamataEngine::Audio::GetInstance()->StopWave(voiceHandleBGM_);
	// メモリ解放
	delete clearFontWorldTransform_;
	clearFontWorldTransform_ = nullptr;

	delete modelSkydome_;
	modelSkydome_ = nullptr;

	delete skydome_;
	skydome_ = nullptr;

	delete playerWorldTransform_;
	playerWorldTransform_ = nullptr;

	delete clearFontModel_;
	clearFontModel_ = nullptr;

	delete playerModel_;
	playerModel_ = nullptr;

	delete fade_;
	fade_ = nullptr;
}