#include "GameOverScene.h"

using namespace KamataEngine;

void GameOverScene::Initialize() {
	gameoverWorldTransform_ = new WorldTransform();
	playerWorldTransform_ = new WorldTransform();

	// カメラの初期化
	camera_.Initialize();

	// 3Dモデルの生成（※モデル名はお持ちのファイル名に合わせて調整してください）
	fontmodel_ = Model::CreateFromOBJ("gameoverFont", true);
	gameoverWorldTransform_->Initialize();
	gameoverWorldTransform_->translation_ = gameoverPosition_;
	UpdateWorldTransform(*gameoverWorldTransform_);

	// プレイヤーモデルの生成（横倒しなどの演出）
	model_ = Model::CreateFromOBJ("player", true);
	playerWorldTransform_->Initialize();
	playerWorldTransform_->translation_ = playerPosition_;
	playerWorldTransform_->rotation_.z = 1.57079632f; // 横に倒す演出
	UpdateWorldTransform(*playerWorldTransform_);

	modelSkydome_ = Model::CreateFromOBJ("gameoverSkydome", true);
	skydome_ = new skydome();
	skydome_->Initialize(modelSkydome_, &camera_);

	skydome_->Initialize(modelSkydome_, &camera_);

	// フェードの初期化とスタート
	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, fadeTime_);

	camera_.UpdateMatrix();
	finished_ = false;

	soundHandleBGM_ = KamataEngine::Audio::GetInstance()->LoadWave("Audio/GameOver.wav");
	voiceHandleBGM_ = KamataEngine::Audio::GetInstance()->PlayWave(soundHandleBGM_, false, 0.4f);
}

void GameOverScene::Update() {
	if (skydome_) {
		skydome_->Update();
	}

	if (Input::GetInstance()->TriggerKey(DIK_R)) {
		finished_ = true;
		nextScene_ = NextScene::kRetry;
	}
	// Tキー でタイトルへ戻る（TitleSceneへ）
	else if (Input::GetInstance()->TriggerKey(DIK_T)) {
		finished_ = true;
		nextScene_ = NextScene::kTitle;
	}

	fade_->Update();
}

void GameOverScene::Draw() {
	// 3D描画開始
	Model::PreDraw();

	if (skydome_) {
		skydome_->Draw();
	}

	if (fontmodel_ && gameoverWorldTransform_) {
		fontmodel_->Draw(*gameoverWorldTransform_, camera_);
	}

	// 3D描画終了
	Model::PostDraw();

	// フェードの描画
	if (fade_) {
		fade_->Draw();
	}
}

void GameOverScene::Finalize() {

	KamataEngine::Audio::GetInstance()->StopWave(voiceHandleBGM_);
	// メモリ解放
	delete gameoverWorldTransform_;
	gameoverWorldTransform_ = nullptr;

	delete playerWorldTransform_;
	playerWorldTransform_ = nullptr;

	delete fontmodel_;
	fontmodel_ = nullptr;

	delete model_;
	model_ = nullptr;

	delete fade_;
	fade_ = nullptr;
}