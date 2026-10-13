#include "skydome.h"
#include "KamataEngine.h"
#include "WorldTransformEx.h"

void skydome::Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera) {

	// NULLチェック


	// 受け取ったモデルとカメラを保存
	model_ = model;
	camera_ = camera;

	// ワールド変換初期化
	worldTransform_.Initialize();

	// 天球は大きくする
	worldTransform_.scale_ = {500.0f, 500.0f, 500.0f};
}
/// 更新
void skydome::Update() {

	worldTransform_.translation_ = {0, 0, 0};

	// 行列転送
	worldTransform_.TransferMatrix();
}

/// 描画
void skydome::Draw() { model_->Draw(worldTransform_, *camera_); }