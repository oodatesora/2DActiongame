#include "ShieldEnemy.h"
#include "Ease.h"
#include "GameScene.h"
#include "MapchipField.h"
#include "Player.h"
#include "WorldTransformEx.h"
#include <algorithm>
#include <numbers>

void ShieldEnemy::Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position) {

	model_ = model;
	camera_ = camera;
	worldTransform_.Initialize();
	worldTransformDeath_.Initialize();
	worldTransform_.rotation_.y = std::numbers::pi_v<float> * 3.0f / 2.0f;
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};
	worldTransform_.translation_.x = position.x;
	worldTransform_.translation_.y = position.y;
	worldTransform_.translation_.z = position.z;
	// 速度を設定する
	velocity_ = {-kWalkSpeed, 0, 0};
	walkTimer_ = 0.0f;
	UpdateWorldTransform(worldTransform_);

	soundHandleGuard_ = KamataEngine::Audio::GetInstance()->LoadWave("Audio/Guard.wav");
	soundHandleEnemyDoun_ = KamataEngine::Audio::GetInstance()->LoadWave("Audio/EnemyDoun.wav");
}

// 指定した角の座標計算
KamataEngine::Vector3 ShieldEnemy::CornerPosition(const KamataEngine::Vector3& center, ShieldEnemy::Corner corner) {
	KamataEngine::Vector3 offsetTable[kNumCorner] = {
	    {+kWidth / 2.0f, -kHeight / 2.0f, 0},
        {-kWidth / 2.0f, -kHeight / 2.0f, 0},
        {+kWidth / 2.0f, +kHeight / 2.0f, 0},
        {-kWidth / 2.0f, +kHeight / 2.0f, 0}
    };
	return center + offsetTable[static_cast<uint32_t>(corner)];
}

// 右壁の判定と押し戻し
bool ShieldEnemy::CheckMapCollisionRight(CollisionMapInfo& info) {
	if (info.moveAmount.x <= 0.0f)
		return false;

	std::array<KamataEngine::Vector3, ShieldEnemy::kNumCorner> positionsNew;
	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.moveAmount, static_cast<ShieldEnemy::Corner>(i));
	}

	bool hit = false;
	MapChipField::IndexSet indexSet;

	// 右上・右下のブロックチェック（kBlock または kEnemyOnlyWall）
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);
	MapChipType typeTop = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (typeTop == MapChipType::kBlock || typeTop == MapChipType::kEnemyOnlyWall)
		hit = true;

	if (!hit) {
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);
		MapChipType typeBottom = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
		if (typeBottom == MapChipType::kBlock || typeBottom == MapChipType::kEnemyOnlyWall)
			hit = true;
	}

	if (hit) {
		MapChipField::IndexSet indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + KamataEngine::Vector3(+kWidth / 2.0f, 0.0f, 0.0f));

		if (indexSetNow.xIndex != indexSet.xIndex) {
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.moveAmount.x = (rect.left - worldTransform_.translation_.x) - (kWidth / 2.0f + 0.001f);
			info.hitWall = true;
		}
	}

	return hit;
}

// 左壁の判定と押し戻し
bool ShieldEnemy::CheckMapCollisionLeft(CollisionMapInfo& info) {
	if (info.moveAmount.x >= 0.0f)
		return false;

	std::array<KamataEngine::Vector3, ShieldEnemy::kNumCorner> positionsNew;
	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.moveAmount, static_cast<ShieldEnemy::Corner>(i));
	}

	bool hit = false;
	MapChipField::IndexSet indexSet;

	// 左上・左下のブロックチェック（kBlock または kEnemyOnlyWall）
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);
	MapChipType typeTop = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (typeTop == MapChipType::kBlock || typeTop == MapChipType::kEnemyOnlyWall)
		hit = true;

	if (!hit) {
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);
		MapChipType typeBottom = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
		if (typeBottom == MapChipType::kBlock || typeBottom == MapChipType::kEnemyOnlyWall)
			hit = true;
	}

	if (hit) {
		MapChipField::IndexSet indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + KamataEngine::Vector3(-kWidth / 2.0f, 0.0f, 0.0f));

		if (indexSetNow.xIndex != indexSet.xIndex) {
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.moveAmount.x = (rect.right - worldTransform_.translation_.x) + (kWidth / 2.0f + 0.001f);
			info.hitWall = true;
		}
	}

	return hit;
}

// 反転処理
void ShieldEnemy::Turn() {
	velocity_.x *= -1.0f;
	worldTransform_.rotation_.y += std::numbers::pi_v<float>;

	// 向きフラグを切り替え（ガード判定に使用）
	if (direction_ == LRDirection::kLeft) {
		direction_ = LRDirection::kRight;
	} else {
		direction_ = LRDirection::kLeft;
	}
}

void ShieldEnemy::Update() {
	if (behaviorRequest_ != Beahavior::kUnnown) {
		// ふるまいを変更
		behavior_ = behaviorRequest_;
		// 各振る舞いごとの初期化を実行
		switch (behavior_) {
			// 通常行動
		case Beahavior::kRoot:
		default:
			// ルートビヘイビアの初期化
			BehaviorRootInitialize();
			break;
			// 攻撃行動
		case Beahavior::kDeath:
			// 攻撃ビヘイビアの初期化
			BehaviorDeathInitialize();
			break;
		case Beahavior::gurad:
			BehaviorGuardInitialize();
			break;
		}

		// 振る舞いリクエストをリセット
		behaviorRequest_ = Beahavior::kUnnown;
	}

	switch (behavior_) {
	case Beahavior::kRoot:
		BehaviorRootUpdate();
		break;
	case Beahavior::kDeath:
		BehaviorDeathUpdate();
		break;
	case Beahavior::gurad:
		BehaviorGuardUpdate();
	}
}

KamataEngine::Vector3 ShieldEnemy::GetWorldPosition() {
	// ワールド座標を入れる変数
	KamataEngine::Vector3 worldPos;
	// ワールド行列の平行移動成分を取得
	worldPos.x = worldTransform_.matWorld_.m[3][0];
	worldPos.y = worldTransform_.matWorld_.m[3][1];
	worldPos.z = worldTransform_.matWorld_.m[3][2];

	return worldPos;
}

AABB ShieldEnemy::GetAABB() {

	KamataEngine::Vector3 worldPos = GetWorldPosition();

	AABB aabb;

	aabb.min = {worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f};

	aabb.max = {worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f};

	return aabb;
}

void ShieldEnemy::OnCollision(Player* player) {
	(void)player;
	// デス演出へ移行
	if (behavior_ == Beahavior::kDeath) {
		// 敵がやられているなら何もしない
		return;
	}

	bool guard = (player->GetDirection() == LRDirection::kRight && GetDirection() == LRDirection::kLeft) ||

	             (player->GetDirection() == LRDirection::kLeft && GetDirection() == LRDirection::kRight);

	// プレイヤーが攻撃中なら敵が死ね
	if (player->Player::IsAttack()) {
		// 前方であればガード成功
		if (guard) {
			KamataEngine::Audio::GetInstance()->PlayWave(soundHandleGuard_, false, 0.5f);

			// 敵と自キャラの中間位置にエフェクトを生成
			KamataEngine::Vector3 guardEffectPos = (GetWorldPosition() + player->GetWorldPosition()) / 2.0f;
			gameScene_->CreateGuardEffect(guardEffectPos);

			// プレイヤーのノックバックを要求する
			player->GetKnockBack();

			behaviorRequest_ = Beahavior::gurad;

			// 早期リターンで敵のデスを回避する
			return;
		}

		KamataEngine::Audio::GetInstance()->PlayWave(soundHandleEnemyDoun_, false, 0.5f);

		// 敵の振る舞いをデス演出に変更
		behaviorRequest_ = Beahavior::kDeath;
		// プレイヤーと敵の位置を取得
		KamataEngine::Vector3 playerPosition = player->GetWorldPosition();
		KamataEngine::Vector3 enemyPosition = GetWorldPosition();

		// プレイヤーから離れる方向へ吹っ飛ぶ
		if (enemyPosition.x >= playerPosition.x) {
			deathVelocity_.x = 0.4f;
		} else {
			deathVelocity_.x = -0.4f;
		}

		// 上方向への吹っ飛び
		deathVelocity_.y = 0.25f;
		deathVelocity_.z = 0.0f;

		// 子リジョン無効フラグを立てる
		isCollisionDisabled_ = true;
	}
}

void ShieldEnemy::Draw() {
	if (behavior_ == Beahavior::kDeath) {
		model_->Draw(worldTransformDeath_, *camera_);
	} else {
		model_->Draw(worldTransform_, *camera_);
	}
}

void ShieldEnemy::BehaviorRootInitialize() {}

void ShieldEnemy::BehaviorRootUpdate() {
	if (mapChipField_) {
		CollisionMapInfo info;
		info.moveAmount = velocity_;

		// 1. 壁判定
		CheckMapCollisionRight(info);
		CheckMapCollisionLeft(info);

		// 移動量を適用
		worldTransform_.translation_ += info.moveAmount;

		// 2. 壁に当たったら反転
		if (info.hitWall) {
			Turn();
		}
	} else {
		worldTransform_.translation_ += velocity_;
	}

	// タイマーを加算
	walkTimer_ += 5.0f / 60.0f;
	// 回転アニメーション（現在の向きに応じた角度加算）
	float param = std::sin(walkTimer_);
	float degree = kWalkMotionAngleStart + kWalkMotionAngleEnd * (param + 1.0f) / 2.0f;
	float baseRotationY = (direction_ == LRDirection::kLeft) ? (std::numbers::pi_v<float> * 3.0f / 2.0f) : (std::numbers::pi_v<float> * 1.0f / 2.0f);
	worldTransform_.rotation_.y = baseRotationY + degree * std::numbers::pi_v<float> / 360.0f;

	// 行列更新
	UpdateWorldTransform(worldTransform_);
}

void ShieldEnemy::BehaviorDeathInitialize() {
	deathParameter_ = 0;
	isDeathParticlesCreated_ = false;

	// 回転速度
	deathRotationSpeed_ = std::numbers::pi_v<float> * 0.3f;
}

void ShieldEnemy::BehaviorDeathUpdate() {
	// アニメーションタイマーを加算
	deathParameter_++;

	// 吹っ飛んでから一定時間経過したらデスパーティクルを生成
	if (deathParameter_ >= deathParticlesDelay_ && !isDeathParticlesCreated_) {
		gameScene_->CreateHitEffect(worldTransform_.translation_);
		isDeathParticlesCreated_ = true;
	}

	// 吹っ飛び移動
	worldTransform_.translation_.x += deathVelocity_.x;
	worldTransform_.translation_.y += deathVelocity_.y;

	// 重力で徐々に落下させる
	deathVelocity_.y -= deathGravity_;

	// 回転
	worldTransform_.rotation_.y += deathRotationSpeed_;
	worldTransform_.rotation_.x += deathRotationSpeed_;

	// トランスフォームの値をコピー
	worldTransformDeath_.translation_ = worldTransform_.translation_;
	worldTransformDeath_.rotation_ = worldTransform_.rotation_;
	worldTransformDeath_.scale_ = worldTransform_.scale_;

	UpdateWorldTransform(worldTransformDeath_);

	// 一定時間経過したら消す
	if (deathParameter_ >= 20) {
		isDead_ = true;
	}

}

void ShieldEnemy::BehaviorGuardInitialize() { guardTimer_ = 0.0f; }

void ShieldEnemy::BehaviorGuardUpdate() {

	// ガード時間
	guardTimer_ += 2.0f / 60.0f;

	// 縦揺れアニメーション
	float param = std::sin(guardTimer_);

	float degree = kGuardMotionAngleStart + kGuardMotionAngleEnd * (param + 1.0f) / 2.0f;

	worldTransform_.rotation_.x = degree * std::numbers::pi_v<float> / 180.0f;

	// 行列更新
	UpdateWorldTransform(worldTransform_);

	// ガード終了
	if (guardTimer_ >= 1.0f) {

		// 回転を戻す
		worldTransform_.rotation_.x = 0.0f;

		behaviorRequest_ = Beahavior::kRoot;
	}
}