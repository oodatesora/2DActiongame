#include "ChargeParticle.h"
#include "WorldTransformEx.h"
#include <algorithm>
#include <random>

// 静的メンバ変数
KamataEngine::Model* ChargeParticle::model_ = nullptr;
KamataEngine::Camera* ChargeParticle::camera_ = nullptr;

void ChargeParticle::SetModel(KamataEngine::Model* model) { model_ = model; }

void ChargeParticle::SetCamera(KamataEngine::Camera* camera) { camera_ = camera; }

void ChargeParticle::Initialize(const KamataEngine::Vector3& position) {
	worldTransform_.Initialize();

	// 生成位置
	startPosition_ = position;
	worldTransform_.translation_ = position;

	// 移動先は後から設定する
	targetPosition_ = position;

	// 大きさを設定
	scale_ = 0.2f;
	worldTransform_.scale_ = {scale_, scale_, scale_};

	// 初期化
	counter_ = 0.0f;
	isDead_ = false;

	UpdateWorldTransform(worldTransform_);
}

void ChargeParticle::Update() {
	if (isDead_) {
		return;
	}

	const float deltaTime = 1.0f / 60.0f;
	counter_ += deltaTime;

	// 寿命が来たら消滅
	if (counter_ >= kLifeTime) {
		isDead_ = true;
		return;
	}

	// 移動の進行度
	float t = counter_ / kLifeTime;
	t = std::clamp(t, 0.0f, 1.0f);

	// プレイヤーの位置に向かって移動
	worldTransform_.translation_.x += (targetPosition_.x - worldTransform_.translation_.x) * 0.1f;

	worldTransform_.translation_.y += (targetPosition_.y - worldTransform_.translation_.y) * 0.1f;

	worldTransform_.translation_.z += (targetPosition_.z - worldTransform_.translation_.z) * 0.1f;

	// 徐々に小さくする
	scale_ = 0.2f * (1.0f - t);
	worldTransform_.scale_ = {scale_, scale_, scale_};

	UpdateWorldTransform(worldTransform_);
}

void ChargeParticle::Draw() {
	if (isDead_ || model_ == nullptr || camera_ == nullptr) {
		return;
	}

	model_->Draw(worldTransform_, *camera_);
}

ChargeParticle* ChargeParticle::Create(const KamataEngine::Vector3& position) {
	ChargeParticle* instance = new ChargeParticle();

	// プレイヤーの周囲にランダムな位置を設定
	static std::mt19937 randomEngine(std::random_device{}());
	static std::uniform_real_distribution<float> randomOffset(-3.0f, 3.0f);

	KamataEngine::Vector3 particlePosition = position;
	particlePosition.x += randomOffset(randomEngine);
	particlePosition.y += randomOffset(randomEngine);
	particlePosition.z += randomOffset(randomEngine);

	instance->Initialize(particlePosition);
	instance->SetTargetPosition(position);

	return instance;
}

void ChargeParticle::SetTargetPosition(const KamataEngine::Vector3& position) { targetPosition_ = position; }