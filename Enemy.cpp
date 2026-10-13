#define NOMINMAX
#include "Enemy.h"
#include "Ease.h"
#include "GameScene.h"
#include "MapchipField.h"
#include "Player.h"
#include "WorldTransformEx.h"
#include <algorithm>
#include <numbers>

void Enemy::Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position) {

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

	soundHandleEnemyDoun_ = KamataEngine::Audio::GetInstance()->LoadWave("Audio/EnemyDoun.wav");
}

// 指定した角の座標計算
KamataEngine::Vector3 Enemy::CornerPosition(const KamataEngine::Vector3& center, Enemy::Corner corner) {
	KamataEngine::Vector3 offsetTable[kNumCorner] = {
	    {+kWidth / 2.0f, -kHeight / 2.0f, 0},
        {-kWidth / 2.0f, -kHeight / 2.0f, 0},
        {+kWidth / 2.0f, +kHeight / 2.0f, 0},
        {-kWidth / 2.0f, +kHeight / 2.0f, 0}
    };
	return center + offsetTable[static_cast<uint32_t>(corner)];
}

// 右壁の判定と押し戻し
bool Enemy::CheckMapCollisionRight(CollisionMapInfo& info) {
	if (info.moveAmount.x <= 0.0f)
		return false;

	std::array<KamataEngine::Vector3, Enemy::kNumCorner> positionsNew;
	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.moveAmount, static_cast<Enemy::Corner>(i));
	}

	bool hit = false;
	MapChipField::IndexSet indexSet;

	// 右上・右下のブロックチェック（通常の壁 または 敵専用透明壁）
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);
	MapChipType typeTop = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (typeTop == MapChipType::kBlock || typeTop == MapChipType::kEnemyOnlyWall) {
		hit = true;
	}

	if (!hit) {
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);
		MapChipType typeBottom = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
		if (typeBottom == MapChipType::kBlock || typeBottom == MapChipType::kEnemyOnlyWall) {
			hit = true;
		}
	}

	if (hit) {
		// 移動前の右端位置からマップチップのインデックスを取得
		MapChipField::IndexSet indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + KamataEngine::Vector3(+kWidth / 2.0f, 0.0f, 0.0f));

		// 隣のブロック列に進入した時だけ押し戻し＆hitWallフラグを立てる
		if (indexSetNow.xIndex != indexSet.xIndex) {
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.moveAmount.x = (rect.left - worldTransform_.translation_.x) - (kWidth / 2.0f + 0.001f);
			info.hitWall = true;
		}
	}

	return hit;
}

// 左壁の判定と押し戻し
bool Enemy::CheckMapCollisionLeft(CollisionMapInfo& info) {
	if (info.moveAmount.x >= 0.0f)
		return false;

	std::array<KamataEngine::Vector3, Enemy::kNumCorner> positionsNew;
	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.moveAmount, static_cast<Enemy::Corner>(i));
	}

	bool hit = false;
	MapChipField::IndexSet indexSet;

	// 左上・左下のブロックチェック（通常の壁 または 敵専用透明壁）
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);
	MapChipType typeTop = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (typeTop == MapChipType::kBlock || typeTop == MapChipType::kEnemyOnlyWall) {
		hit = true;
	}

	if (!hit) {
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);
		MapChipType typeBottom = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
		if (typeBottom == MapChipType::kBlock || typeBottom == MapChipType::kEnemyOnlyWall) {
			hit = true;
		}
	}

	if (hit) {
		// 移動前の左端位置からマップチップのインデックスを取得
		MapChipField::IndexSet indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + KamataEngine::Vector3(-kWidth / 2.0f, 0.0f, 0.0f));

		// 隣のブロック列に進入した時だけ押し戻し＆hitWallフラグを立てる
		if (indexSetNow.xIndex != indexSet.xIndex) {
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.moveAmount.x = (rect.right - worldTransform_.translation_.x) + (kWidth / 2.0f + 0.001f);
			info.hitWall = true;
		}
	}

	return hit;
}

// 反転処理
void Enemy::Turn() {
	velocity_.x *= -1.0f;
	worldTransform_.rotation_.y += std::numbers::pi_v<float>;
}

void Enemy::Update() {
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
	}
}

KamataEngine::Vector3 Enemy::GetWorldPosition() {
	// ワールド座標を入れる変数
	KamataEngine::Vector3 worldPos;
	// ワールド行列の平行移動成分を取得
	worldPos.x = worldTransform_.matWorld_.m[3][0];
	worldPos.y = worldTransform_.matWorld_.m[3][1];
	worldPos.z = worldTransform_.matWorld_.m[3][2];

	return worldPos;
}

AABB Enemy::GetAABB() {

	KamataEngine::Vector3 worldPos = GetWorldPosition();

	AABB aabb;

	aabb.min = {worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f};

	aabb.max = {worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f};

	return aabb;
}

void Enemy::OnCollision(const Player* player) {
	(void)player;
	// デス演出へ移行
	if (behavior_ == Beahavior::kDeath) {
		// 敵がやられているなら何もしない
		return;
	}

	// プレイヤーが攻撃中なら敵が死ね
	if (player->Player::IsAttack()) {
		KamataEngine::Audio::GetInstance()->PlayWave(soundHandleEnemyDoun_, false, 0.5f);

		// 敵の振る舞いをデス演出に変更
		behaviorRequest_ = Beahavior::kDeath;
		KamataEngine::Vector3 playerPosition = player->GetWorldPosition();
		KamataEngine::Vector3 enemyPosition = GetWorldPosition();

		// プレイヤーが左側なら右へ、右側なら左へ吹っ飛ぶ
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

void Enemy::Draw() {
	if (behavior_ == Beahavior::kDeath) {
		model_->Draw(worldTransformDeath_, *camera_);
	} else {
		model_->Draw(worldTransform_, *camera_);
	}
}

void Enemy::BehaviorRootInitialize() {}

void Enemy::BehaviorRootUpdate() {
	if (mapChipField_) {
		CollisionMapInfo info;
		info.moveAmount = velocity_;

		// 1. 壁判定（Playerと同じ処理）
		CheckMapCollisionRight(info);
		CheckMapCollisionLeft(info);

		// 壁判定で計算された移動量を加算
		worldTransform_.translation_ += info.moveAmount;

		// 2. 壁に当たった場合だけ反転する（崖判定は一旦スキップ）
		if (info.hitWall) {
			Turn();
		}
	} else {
		worldTransform_.translation_ += velocity_;
	}

	// アニメーション処理
	walkTimer_ += 5.0f / 60.0f;
	float param = std::sin(walkTimer_);
	float degree = kWalkMotionAngleStart + kWalkMotionAngleEnd * (param + 1.0f) / 2.0f;
	worldTransform_.rotation_.x = degree * std::numbers::pi_v<float> / 180.0f;

	// 行列更新
	UpdateWorldTransform(worldTransform_);
}

void Enemy::BehaviorDeathInitialize() {
	deathParameter_ = 0;
	isHitEffectCreated_ = false;

	// 回転速度
	deathRotationSpeed_ = std::numbers::pi_v<float> * 0.3f;
}

void Enemy::BehaviorDeathUpdate() {
	// アニメーションタイマーを加算
	deathParameter_++;

	// 吹っ飛んでから一定時間経過したらヒットエフェクトを生成
	if (deathParameter_ >= hitEffectDelay_ && !isHitEffectCreated_) {
		gameScene_->CreateHitEffect(worldTransform_.translation_);
		isHitEffectCreated_ = true;
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