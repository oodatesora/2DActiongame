#include "HitEffect.h"
#include "Random.h"
#include "WorldTransformEx.h"
#include <algorithm>
#include <numbers>

// 静的メンバ変数の実体
KamataEngine::Model* HitEffect::model_ = nullptr;
KamataEngine::Camera* HitEffect::camera_ = nullptr;

void HitEffect::SetModel(KamataEngine::Model* model) { model_ = model; }

void HitEffect::SetCamera(KamataEngine::Camera* camera) { camera_ = camera; }

void HitEffect::Initialize(const KamataEngine::Vector3& position) {
	circleWorldTransform_.Initialize();
	circleWorldTransform_.translation_ = position;

	// 楕円エフェクト
	for (KamataEngine::WorldTransform& worldtransform : ellipseWorldTransforms_) {
		worldtransform.Initialize();
		worldtransform.scale_ = {0.1f, 2.0f, 1.0f};
		worldtransform.rotation_.z = Random::GetFloat(0.0f, std::numbers::pi_v<float> * 2.0f);

		worldtransform.translation_ = position;
	}

	phase_ = Phase::ScaleUp;
	counter_ = 0.0f;
	scale_ = kStartScale;

	objectColor_.Initialize();
	color_ = {1, 1, 1, 1};
}

void HitEffect::Update() {
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
		}
		break;
	}

	case Phase::Dead:
		break;
	}

	// トランスフォームへスケール反映
	circleWorldTransform_.scale_ = {scale_, scale_, scale_};
	UpdateWorldTransform(circleWorldTransform_);

	for (KamataEngine::WorldTransform& worldtransform : ellipseWorldTransforms_) {
		worldtransform.scale_ = {0.1f * scale_, 2.0f * scale_, 1.0f * scale_};
		UpdateWorldTransform(worldtransform);
	}

	circleWorldTransform_.scale_ = {scale_, scale_, scale_};

	// 円形エフェクトのワールドトランスフォームを更新
	//   行列更新
	UpdateWorldTransform(circleWorldTransform_);

	for (KamataEngine::WorldTransform& worldtransform : ellipseWorldTransforms_) {
		UpdateWorldTransform(worldtransform);
	}
}

void HitEffect::Draw() {

	if (phase_ == Phase::Dead) {
		return;
	}

	if (model_ == nullptr || camera_ == nullptr) {
		return;
	}
	model_->Draw(circleWorldTransform_, *camera_, &objectColor_);

	for (const KamataEngine::WorldTransform& worldtransform : ellipseWorldTransforms_) {
		model_->Draw(worldtransform, *camera_, &objectColor_);
	}
}

HitEffect* HitEffect::Create(const KamataEngine::Vector3& position) {
	// インスタンス生成
	HitEffect* instance = new HitEffect();
	// newの失敗を検出

	// インスタンスの初期化
	instance->Initialize(position);
	// 初期化したインスタンスを返す
	return instance;
}