#include "DeathParticles.h"
#include "WorldTransformEx.h"
#include <algorithm>

KamataEngine::Matrix4x4 MakeIdentity4x4() {
	KamataEngine::Matrix4x4 result{};

	for (int i = 0; i < 4; ++i) {
		result.m[i][i] = 1.0f;
	}

	return result;
}

KamataEngine::Vector3 Transform(const KamataEngine::Vector3& v, const KamataEngine::Matrix4x4& m) {
	KamataEngine::Vector3 result;

	result.x = v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0] + m.m[3][0];
	result.y = v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1] + m.m[3][1];
	result.z = v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2] + m.m[3][2];

	return result;
}

KamataEngine::Matrix4x4 MakeRotateZMatrix(float radian) {
	KamataEngine::Matrix4x4 result = MakeIdentity4x4();

	result.m[0][0] = cosf(radian);
	result.m[0][1] = sinf(radian);
	result.m[1][0] = -sinf(radian);
	result.m[1][1] = cosf(radian);

	return result;
}

void DeathParticles::Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position) {
	// ワールド変換の初期化
	for (KamataEngine::WorldTransform& worldTransform : worldTransforms_) {
		worldTransform.Initialize();
		worldTransform.translation_ = position;
		model_ = model;
		camera_ = camera;
		objectColor_.Initialize();
		color_ = {1, 1, 1, 1};
	}
}

void DeathParticles::Update() {
	// 移動
	for (uint32_t i = 0; i < kNumParticles; ++i) {
		// 基本となる速度ベクトル
		KamataEngine::Vector3 Velocity = {kSpeed, 0, 0};
		// 回転角を計算
		float angle = kAngleUnit * i;
		// Z軸周りの回転行列
		KamataEngine::Matrix4x4 MatrixRotation = MakeRotateZMatrix(angle);
		// 基本ベクトルを回転させて速度ベクトルを得る
		Velocity = Transform(Velocity, MatrixRotation);
		// 移動処理
		worldTransforms_[i].translation_ += Velocity;
	}

	// カウンターを１フレーム分の秒数進める
	counter_ += 1.0f / 60.0f;

	// 存続時間の上限に達したら
	if (counter_ >= kDuration) {
		counter_ = kDuration;
		// 終了扱いにする
		isFinished_ = true;
	}

	// α値を更新
	color_.w = 1.0f - (counter_ / kDuration);

	color_.w = std::clamp(color_.w, 0.0f, 1.0f);
	// 色変更オブジェクトに色の数値を設定する
	objectColor_.SetColor(color_);

	// 終了なら何もしない
	if (isFinished_) {
		return;
	}

	// ワールド変換の更新
	for (KamataEngine::WorldTransform& worldTransform : worldTransforms_) {
		UpdateWorldTransform(worldTransform);
	}
}

void DeathParticles::Draw() {
	if (!isFinished_) {
		for (int i = 0; i < kNumParticles; i++) {
			model_->Draw(worldTransforms_[i], *camera_, &objectColor_);
		}
	}

	// 終了なら何もしない
	if (isFinished_) {
		return;
	}
}
