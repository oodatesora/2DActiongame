#include "TitleScene.h"
#include "KamataEngine.h"

using namespace KamataEngine;

void TitleScene::Initialize() {

	titleWoldTransform_ = new WorldTransform();
	playerWoldTransform_ = new WorldTransform();

	// カメラの初期化
	camera_.Initialize();

	// 3Dモデルの生成
	fontmodel_ = Model::CreateFromOBJ("titleFont", true);
	titleWoldTransform_->Initialize();
	titleWoldTransform_->translation_ = titleposition;
	UpdateWorldTransform(*titleWoldTransform_);

	// 3Dモデルの生成
	model_ = Model::CreateFromOBJ("player", true);
	playerWoldTransform_->Initialize();
	playerWoldTransform_->scale_ = {2.0f, 2.0f, 2.0f};
	playerWoldTransform_->translation_ = playerPosition;
	playerWoldTransform_->rotation_.y = 3.14159265f;
	UpdateWorldTransform(*playerWoldTransform_);

	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, fadeTime_);

	modelSkydome_ = KamataEngine::Model::CreateFromOBJ("titleskydome", true);
	skydome_ = new skydome();
	skydome_->Initialize(modelSkydome_, &camera_);

	camera_.UpdateMatrix();

	soundHandleBGM_ = KamataEngine::Audio::GetInstance()->LoadWave("Audio/TitleBGM.wav");

	voiceHandleBGM_ = KamataEngine::Audio::GetInstance()->PlayWave(soundHandleBGM_, true, 0.3f);
}

void TitleScene::Update() {
	if (KamataEngine::Input::GetInstance()->PushKey(DIK_SPACE)) {
		finished_ = true;
	}

	if (skydome_) {
		skydome_->Update();
	}

	fade_->Update();

	floatTimer_ += 0.05f;

	playerWoldTransform_->translation_.x = playerPosition.x + sinf(floatTimer_) * 0.5f;

	UpdateWorldTransform(*playerWoldTransform_);
}

void TitleScene::Draw() {
	// 3D描画開始
	Model::PreDraw();

	if (skydome_) {
		skydome_->Update();
	}

	model_->Draw(*playerWoldTransform_, camera_);
	fontmodel_->Draw(*titleWoldTransform_, camera_);

	// 3D描画終了
	Model::PostDraw();
	fade_->Draw();
}
void TitleScene::Finalize() {
	KamataEngine::Audio::GetInstance()->StopWave(voiceHandleBGM_);

	delete skydome_;
	skydome_ = nullptr;

	delete fade_;
}