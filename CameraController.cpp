#include "CameraController.h"
#include "GameScene.h"
#include <algorithm>

KamataEngine::Vector3 Lerp(const KamataEngine::Vector3& start, const KamataEngine::Vector3& end, float rate) {
	return {
	    start.x + (end.x - start.x) * rate,
	    start.y + (end.y - start.y) * rate,
	    start.z + (end.z - start.z) * rate,
	};
};

// 初期化
void CameraController::Initialize(KamataEngine::Camera* camera) {
	// カメラコントローラーの初期化
	camera_ = camera;
	// カメラとの距離（オフセット）
	targetOffset_ = {0.0f, 3.0f, -15.0f};
}

// 更新
void CameraController::Update() {

	if (!target_) {
		return;
	}

	// 追従対象のワールドトランスフォームを参照
	const KamataEngine::WorldTransform& targetWorldTransform = target_->GetWorldTransform();
	// 追従対象とオフセットと追従対象の速度からカメラの目標座標を計算
	KamataEngine::Vector3 targetPosition = {
	    targetWorldTransform.translation_.x + targetOffset_.x + target_->GetVelocity().x * kVelocityBias, targetWorldTransform.translation_.y + targetOffset_.y,
	    targetWorldTransform.translation_.z + targetOffset_.z};

	// 座標補間によるゆったり追従
	camera_->translation_ = Lerp(camera_->translation_, targetPosition, kInterpolationRate);

	// 追従対象が画面外に出ないように補正
	camera_->translation_.x =
	    std::clamp(camera_->translation_.x, targetWorldTransform.translation_.x + kTargetDirectionMovableArea.left, targetWorldTransform.translation_.x + kTargetDirectionMovableArea.right);
	camera_->translation_.y =
	    std::clamp(camera_->translation_.y, targetWorldTransform.translation_.y + kTargetDirectionMovableArea.bottom, targetWorldTransform.translation_.y + kTargetDirectionMovableArea.top);

	// x座標を範囲内に収める
	camera_->translation_.x = std::clamp(camera_->translation_.x, movableArea_.left, movableArea_.right);
	// y座標を範囲内に収める
	camera_->translation_.y = std::clamp(camera_->translation_.y, movableArea_.bottom, movableArea_.top);

	// 行列を更新する
	camera_->UpdateViewMatrix();
}

void CameraController::Reset() {

	if (!target_) {
		return;
	}

	// 追従対象のワールドトランスフォームを参照
	const KamataEngine::WorldTransform& targetWorldTransform = target_->GetWorldTransform();

	// 追従対象とオフセットからカメラの座標を計算
	camera_->translation_ = {targetWorldTransform.translation_.x + targetOffset_.x, targetWorldTransform.translation_.y + targetOffset_.y, targetWorldTransform.translation_.z + targetOffset_.z};
	camera_->UpdateMatrix();
}
