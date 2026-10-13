#include "GuardEffect.h"
#include "Random.h"
#include "WorldTransformEx.h"
#include <algorithm>
#include <numbers>

// 静的メンバ変数の実体
KamataEngine::Model* GuardEffect::model_ = nullptr;
KamataEngine::Camera* GuardEffect::camera_ = nullptr;

void GuardEffect::SetModel(KamataEngine::Model* model) { model_ = model; }

void GuardEffect::SetCamera(KamataEngine::Camera* camera) { camera_ = camera; }

void GuardEffect::Initialize(const KamataEngine::Vector3& position) {
	circleWorldTransform_.Initialize();
	circleWorldTransform_.translation_ = position;

	phase_ = Phase::ScaleUp;
	counter_ = 0.0f;
	scale_ = kStartScale;

	objectColor_.Initialize();
	color_ = {1, 1, 1, 1};

	isActive_ = true;
}

void GuardEffect::Update() {
	if (!isActive_) {
		return;
	}

	if (phase_ == Phase::Dead)
		return;

	const float kDeltaTime = 1.0f / 60.0f;
	counter_ += kDeltaTime;

	switch (phase_) {
	case Phase::ScaleUp: {

		float rate = counter_ / kScaleUpDuration;
		rate = (std::min)(rate, 1.0f);

		scale_ = kStartScale + (kMaxScale - kStartScale) * rate;

		if (counter_ >= kScaleUpDuration) {
			phase_ = Phase::FadeOut;
			counter_ = 0.0f; // 次のフェーズ用にカウンターリセット
		}
		break;
	}

	case Phase::FadeOut: {

		color_.w = 1.0f - (counter_ / kFadeOutDuration);

		color_.w = std::clamp(color_.w, 0.0f, 1.0f);
		// 色変更オブジェクトに色の数値を設定する
		objectColor_.SetColor(color_);

		if (counter_ >= kFadeOutDuration) {
			phase_ = Phase::Dead;
			isActive_ = false;
		}
		break;
	}

	case Phase::Dead:
		break;
	}

	// トランスフォームへスケール反映
	circleWorldTransform_.scale_ = {scale_, scale_, scale_};
	UpdateWorldTransform(circleWorldTransform_);
}

void GuardEffect::Draw() {

	if (!isActive_) {
		return;
	}

	if (phase_ == Phase::Dead) {
		isActive_ = false;
		return;
	}

	if (model_ == nullptr || camera_ == nullptr) {
		return;
	}
	model_->Draw(circleWorldTransform_, *camera_, &objectColor_);
}

GuardEffect* GuardEffect::Create(const KamataEngine::Vector3& position) {
	// インスタンス生成
	GuardEffect* instance = new GuardEffect();
	// newの失敗を検出

	// インスタンスの初期化
	instance->Initialize(position);
	// 初期化したインスタンスを返す
	return instance;
}