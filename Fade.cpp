#include "Fade.h"
#include <algorithm>

using namespace KamataEngine;

void Fade::Initialize() {

	textureHandle_ = TextureManager::Load("./Resources/white1x1.png");

	sprite_ = KamataEngine::Sprite::Create(textureHandle_, {0.0f, 0.0f});
	sprite_->SetSize(KamataEngine::Vector2(1280.0f, 720.0f));
	sprite_->SetColor(KamataEngine::Vector4(0.0f, 0.0f, 0.0f, 1.0f));
}

void Fade::Update() {
	// フェード状態に分岐する
	switch (status_) {
	case Status::Nome:
		// 何もしない
		break;
	case Status::FadeIn:
		// 1フレーム分の秒数カウントアップ
		counter_ -= 1.0f / 60.0f;
		// フェード継続時間に達したら打ち止め
		if (counter_ <= 0) {
			counter_ = 0;
		}

		sprite_->SetColor(KamataEngine::Vector4(0, 0, 0, std::clamp(counter_ / duration_, 0.0f, 1.0f)));
		break;
	case Status::FadeOut:
		// 1フレーム分の秒数カウントアップ
		counter_ += 1.0f / 60.0f;
		// フェード継続時間に達したら打ち止め
		if (counter_ >= duration_) {
			counter_ = duration_;
		}
		// 0.0fから１.０fの間で、経過時間がフェード継続時間に近づくほどアルファ値を大きくする
		sprite_->SetColor(KamataEngine::Vector4(0, 0, 0, std::clamp(counter_ / duration_, 0.0f, 1.0f)));
		break;
	}
}

void Fade::Draw() {
	Sprite::PreDraw();
	sprite_->Draw();
	Sprite::PostDraw();
}

void Fade::Start(Status status, float duration) {
	status_ = status;
	duration_ = duration;
	if (status == Status::FadeIn) {
		counter_ = duration_;
	} else if (status == Status::FadeOut) {
		counter_ = 0.0f;
	}
}

bool Fade::IsFinished() const {
	if (status_ == Status::FadeIn && counter_ <= 0.0f) {
		return true;
	}
	if (status_ == Status::FadeOut && counter_ >= duration_) {
		return true;
	}
	return false;
}