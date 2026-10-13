#define NOMINMAX
#include "Player.h"
#include "Ease.h"
#include "KamataEngine.h"
#include "MapchipField.h"
#include "WorldTransformEx.h"
#include <algorithm>
#include <numbers>

void Player::Initialize(KamataEngine::Model* model, KamataEngine::Model* modelAttack, KamataEngine::Camera* camera, const KamataEngine::Vector3& position) {
	// NULLポインタチェック

	model_ = model;
	modelAttack_ = modelAttack;
	camera_ = camera;
	worldTransform_.Initialize();

	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;
	worldTransform_.translation_.x = position.x;
	worldTransform_.translation_.y = position.y;
	worldTransform_.translation_.z = position.z;
	worldTransformAttack_.Initialize();

	onGround_ = true;
	hasAttackedInAir_ = false;

	soundHandleAttck_ = KamataEngine::Audio::GetInstance()->LoadWave("Audio/Attck.wav");
	soundHandleDoun_ = KamataEngine::Audio::GetInstance()->LoadWave("Audio/doun.wav");
	soundHandleWarp_ = KamataEngine::Audio::GetInstance()->LoadWave("Audio/warp.wav");
	isPlayedDounSound_ = false;

	playerHp_ = 3.0f;
}

void Player::CheckMapCollision(CollisionMapInfo& info) {
	CheckMapCollisionDown(info);
	CheckMapCollisionUp(info);
	CheckMapCollisionLeft(info);
	CheckMapCollisionRight(info);
}

void Player::CheckSpikeCollision() {
	if (isDead_) {
		return;
	}
	// プレイヤーのAABBを取得
	AABB playerAABB = GetAABB();

	MapChipField::IndexSet indexSet = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_);

	for (int y = -1; y <= 1; ++y) {
		for (int x = -1; x <= 1; ++x) {

			uint32_t xIndex = indexSet.xIndex + x;
			uint32_t yIndex = indexSet.yIndex + y;

			MapChipType type = mapChipField_->GetMapChipTypeByIndex(xIndex, yIndex);

			if (type != MapChipType::kSpike) {
				continue;
			}

			// トゲの矩形を取得
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(xIndex, yIndex);

			// プレイヤーとトゲが重なっているか
			if (playerAABB.max.x > rect.left && playerAABB.min.x < rect.right && playerAABB.max.y > rect.bottom && playerAABB.min.y < rect.top) {

				isDead_ = true;

				KamataEngine::Audio::GetInstance()->PlayWave(soundHandleDoun_, false, 0.5f);

				return;
			}
		}
	}
}

void Player::Move() {

	// 接地状態
	if (onGround_) {

		if (velocity_.y > 0.0f) {
			// 空中状態に移行
			onGround_ = false;

		} else {

			//  左右移動操作
			if (KamataEngine::Input::GetInstance()->PushKey(DIK_D) || KamataEngine::Input::GetInstance()->PushKey(DIK_A)) {

				KamataEngine::Vector3 acceleration = {0, 0, 0};

				if (KamataEngine::Input::GetInstance()->PushKey(DIK_D)) {

					if (velocity_.x < 0.0f) {
						velocity_.x *= (1.0f - kAttenuation);
					}

					acceleration.x += kAcceleration;

					if (lrDirection_ != LRDirection::kRight) {

						lrDirection_ = LRDirection::kRight;

						turnFirstRotationY_ = worldTransform_.rotation_.y;

						turnTimer_ = kTurnTime;
					}

				} else if (KamataEngine::Input::GetInstance()->PushKey(DIK_A)) {

					if (velocity_.x > 0.0f) {
						velocity_.x *= (1.0f - kAttenuation);
					}

					acceleration.x -= kAcceleration;

					if (lrDirection_ != LRDirection::kLeft) {

						lrDirection_ = LRDirection::kLeft;

						turnFirstRotationY_ = worldTransform_.rotation_.y;

						turnTimer_ = kTurnTime;
					}
				}

				velocity_.x += acceleration.x;

				// 最大速度制限
				velocity_.x = std::clamp(velocity_.x, -kLimitRunSpeed, kLimitRunSpeed);

			} else {

				velocity_.x *= (1.0f - kAttenuation);
			}

			// ジャンプ
			if (KamataEngine::Input::GetInstance()->TriggerKey(DIK_W)) {

				// ジャンプ初速
				velocity_.y = kJumpAcceleration;

				// 空中に移行
				onGround_ = false;
			}
		}
	}

	// 空中
	if (!onGround_) {

		// 落下速度
		velocity_.y -= kGravityAcceleration;

		// 落下速度制限
		velocity_.y = std::max(velocity_.y, -KLimitFallSpeed);

	} else {

		// 落下速度
		velocity_.y -= kGravityAcceleration;

		// 落下速度制限
		velocity_.y = std::max(velocity_.y, -KLimitFallSpeed);
	}

	// 旋回制御
	if (turnTimer_ > 0.0f) {

		turnTimer_ -= 1.0f / 60.0f;

		if (turnTimer_ < 0.0f)
			turnTimer_ = 0.0f;

		float t = 1.0f - (turnTimer_ / kTurnTime);

		// 左右の字キャラ角度テーブル
		float destinationRotationYTable[] = {

		    std::numbers::pi_v<float> / 2.0f, std::numbers::pi_v<float> * 3.0f / 2.0f};

		// 状態に応じた角度を取得する
		float destinationRotationY = destinationRotationYTable[static_cast<uint32_t>(lrDirection_)];

		// 自キャラの角度を設定する
		worldTransform_.rotation_.y = turnFirstRotationY_ + (destinationRotationY - turnFirstRotationY_) * t;
	}
}

// 指定した角の座標計算
KamataEngine::Vector3 Player::CornerPosition(const KamataEngine::Vector3& center, Player::Corner corner) {
	KamataEngine::Vector3 offsetTable[kNumCorner] = {
	    {+kWidth / 2.0f, -kHeight / 2.0f, 0},
        {-kWidth / 2.0f, -kHeight / 2.0f, 0},
        {+kWidth / 2.0f, +kHeight / 2.0f, 0},
        {-kWidth / 2.0f, +kHeight / 2.0f, 0}
    };
	return center + offsetTable[static_cast<uint32_t>(corner)];
}

bool Player::CheckMapCollisionUp(CollisionMapInfo& info) {

	if (info.moveAmount.y <= 0.0f) {
		return false;
	}

	// 移動後の四つの角の座標
	std::array<KamataEngine::Vector3, Player::kNumCorner> positionsNew;

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.moveAmount, static_cast<Player::Corner>(i));
	}

	MapChipType mapChipType;
	MapChipType mapChipTypeNext;
	// 真上の当たり判定を行う
	bool hit = false;
	MapChipField::IndexSet indexSet;
	// 左上点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex + 1);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}
	// 右上点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex + 1);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	// ブロックにヒット？
	if (hit) {
		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + info.moveAmount + KamataEngine::Vector3(0.0f, kHeight / 2.0f, 0.0f));

		// 現在座標壁の外か判定
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + KamataEngine::Vector3(0.0f, +kHeight / 2.0f, 0.0f));

		if (indexSetNow.yIndex != indexSet.yIndex) {

			// めり込み先ブロックの範囲矩形
			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.moveAmount.y = std::max(0.0f, (rect.bottom - worldTransform_.translation_.y) - (kHeight / 2.0f + 0.3f));
			// 天井に当たったことを記録する
			info.ceilingCollision = true;
		}
	}
	return hit;
}

bool Player::CheckMapCollisionDown(CollisionMapInfo& info) {
	if (info.moveAmount.y >= 0) {
		return false;
	}

	std::array<KamataEngine::Vector3, Player::kNumCorner> positionsNew;

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.moveAmount, static_cast<Player::Corner>(i));
	}

	MapChipType mapChipType;
	MapChipType mapChipTypeNext;
	bool hit = false;
	MapChipField::IndexSet indexSet;

	// 左下点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex - 1);

	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	// 右下点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	mapChipTypeNext = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex - 1);
	if (mapChipType == MapChipType::kBlock && mapChipTypeNext != MapChipType::kBlock) {
		hit = true;
	}

	if (hit) {
		// めり込みを排除する方向に移動量を設定する
		indexSet = mapChipField_->GetMapChipIndexSetByPosition((worldTransform_.translation_ + KamataEngine::Vector3(0.0f, -kHeight / 2.0f, 0.0f)) + info.moveAmount);
		// 現在座標壁の外か判定
		MapChipField::IndexSet indexSetNow;
		indexSetNow = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_ + KamataEngine::Vector3(0.0f, -kHeight / 2.0f, 0.0f));

		if (indexSetNow.yIndex != indexSet.yIndex) {

			MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
			info.moveAmount.y = std::min(0.0f, (rect.top - worldTransform_.translation_.y) + (kHeight / 2.0f + kBlank));

			info.landing = true;
		}
	}

	return hit;
}

bool Player::CheckMapCollisionRight(CollisionMapInfo& info) {
	if (info.moveAmount.x <= 0.0f) {
		return false;
	}

	std::array<KamataEngine::Vector3, Player::kNumCorner> positionsNew;

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.moveAmount, static_cast<Player::Corner>(i));
	}

	MapChipType mapChipType;
	bool hit = false;
	MapChipField::IndexSet indexSet;

	// 右上点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	// 右下点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	if (hit) {
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
		info.moveAmount.x = std::max(0.0f, (rect.left - worldTransform_.translation_.x) - (kWidth / 2.0f + kBlank));
		info.hitWall = true;
	}

	return hit;
}

bool Player::CheckMapCollisionLeft(CollisionMapInfo& info) {
	if (info.moveAmount.x >= 0.0f) {
		return false;
	}

	std::array<KamataEngine::Vector3, Player::kNumCorner> positionsNew;

	for (uint32_t i = 0; i < positionsNew.size(); ++i) {
		positionsNew[i] = CornerPosition(worldTransform_.translation_ + info.moveAmount, static_cast<Player::Corner>(i));
	}

	MapChipType mapChipType;
	bool hit = false;
	MapChipField::IndexSet indexSet;

	// 左上点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftTop]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	// 左下点の判定
	indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom]);
	mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
	if (mapChipType == MapChipType::kBlock) {
		hit = true;
	}

	if (hit) {
		MapChipField::Rect rect = mapChipField_->GetRectByIndex(indexSet.xIndex, indexSet.yIndex);
		info.moveAmount.x = std::min(0.0f, (rect.right - worldTransform_.translation_.x) + (kWidth / 2.0f + kBlank));
		info.hitWall = true;
	}

	return hit;
}

void Player::ChekWallCollision(const CollisionMapInfo& info) {
	if (info.hitWall) {

		velocity_.x *= (1.0f - kAttenuationWall);
	}
}

// 移動
void Player::ApplyMapCollision(const Player::CollisionMapInfo& info) { worldTransform_.translation_ += info.moveAmount; }

// 天井に当たった？
void Player::CheckMceilingCollision(const CollisionMapInfo& info) {
	if (info.ceilingCollision) {
		velocity_.y = 0.0f;
	}
}

void Player::ChangingGround(const CollisionMapInfo& info) {
	if (onGround_) {
		// ジャンプ開始
		if (velocity_.y > 0.0f) {
			onGround_ = false;
		} else {
			MapChipType mapChipType;
			// 左下の当たり判定を行う
			bool hit = false;
			std::array<KamataEngine::Vector3, Player::kNumCorner> positionsNew;

			for (uint32_t i = 0; i < positionsNew.size(); ++i) {
				positionsNew[i] = CornerPosition(worldTransform_.translation_, static_cast<Player::Corner>(i));
			}

			hit = false;
			// 左下点の判定
			MapChipField::IndexSet indexSet;
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kLeftBottom] + KamataEngine::Vector3(0.0f, -kGroundBlank, 0.0f));
			mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
			if (mapChipType == MapChipType::kBlock) {
				hit = true;
			}

			// 右上点の判定
			indexSet = mapChipField_->GetMapChipIndexSetByPosition(positionsNew[kRightBottom] + KamataEngine::Vector3(0.0f, -kGroundBlank, 0.0f));
			mapChipType = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);
			if (mapChipType == MapChipType::kBlock) {
				hit = true;
			}
			// 落下開始
			if (!hit) {
				// 空中状態に切り替える
				onGround_ = false;
			}
		}

	} else {
		// 接地フラグ
		if (info.landing) {
			// 接地状態に切り替え
			onGround_ = true;
			// 接地字にX速度を減衰
			velocity_.x *= (1.0f - kAttenuationLanding);

			hasAttackedInAir_ = false;
		}
	}
}

/// 更新
void Player::Update() {

	if (isDead_) {
		UpdateWorldTransform(worldTransform_);
		return;
	}

	if (IsKnockBack_) {
		behaviorRequest_ = Beahavior::knockBack;

		// フラグのリセット
		IsKnockBack_ = false;
	}

	if (isInvincible_) {
		invincibleTime_ -= 1.0f;

		if (invincibleTime_ <= 0.0f) {
			invincibleTime_ = 0.0f;
			isInvincible_ = false;
		}
	}

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
		case Beahavior::kAttack:
			// 攻撃ビヘイビアの初期化
			BehaviorAttackInitialize();
			break;
			// ノックバック行動
		case Beahavior::knockBack:
			BehaviorKnockBackInitialize();
			break;
			// 溜め攻撃行動
		case Beahavior::kchargeAttack:
			BehaviorChargeAttackInitialize();
		}
		// 振る舞いリクエストをリセット
		behaviorRequest_ = Beahavior::kUnnown;
	}

	switch (behavior_) {
	case Beahavior::kRoot:
		BehaviorRootUpdate();
		break;
	case Beahavior::kAttack:
		BehaviorAttackUpdate();
		break;
	case Beahavior::knockBack:
		BehaviorKnockBackUpdate();
		break;
	case Beahavior::kchargeAttack:
		BehaviorChargeAttackUpdate();
		break;
	}

	// 衝突情報を初期化
	CollisionMapInfo collisionMapInfo;
	// 移動量に速度の値をコピー
	collisionMapInfo.moveAmount = velocity_;

	// マップ衝突チェック
	CheckMapCollision(collisionMapInfo);
	CheckMceilingCollision(collisionMapInfo);
	ApplyMapCollision(collisionMapInfo);
	ChangingGround(collisionMapInfo);
	CheckSpikeCollision();

	if (collisionMapInfo.ceilingCollision) {
		velocity_.y = 0.0f;
	}

	MapChipField::IndexSet indexSet = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_);
	MapChipType type = mapChipField_->GetMapChipTypeByIndex(indexSet.xIndex, indexSet.yIndex);

	if (type == MapChipType::kWarp) {
		// GameScene に「次のステージへ切り替えて！」とフラグを送る
		if (!isWarpRequested_) {
			KamataEngine::Audio::GetInstance()->PlayWave(soundHandleWarp_, false, 0.5f);
			isWarpRequested_ = true;
		}
	}

	// 行列更新
	UpdateWorldTransform(worldTransform_);
}

KamataEngine::Vector3 Player::GetWorldPosition() const {
	// ワールド座標を入れる変数
	KamataEngine::Vector3 worldPos;
	// ワールド行列の平行移動成分を取得
	worldPos.x = worldTransform_.matWorld_.m[3][0];
	worldPos.y = worldTransform_.matWorld_.m[3][1];
	worldPos.z = worldTransform_.matWorld_.m[3][2];

	return worldPos;
}

AABB Player::GetAABB() {

	KamataEngine::Vector3 worldPos = GetWorldPosition();

	AABB aabb;

	aabb.min = {worldPos.x - kWidth / 2.0f, worldPos.y - kHeight / 2.0f, worldPos.z - kWidth / 2.0f};

	aabb.max = {worldPos.x + kWidth / 2.0f, worldPos.y + kHeight / 2.0f, worldPos.z + kWidth / 2.0f};

	return aabb;
}

void Player::OnCollision(const Enemy* enemy) {
	(void)enemy;
	if (Player::IsAttack()) {
		return; // 攻撃中はダメージ無効
	}

	if (isDead_) {
		return;
	}

	// 攻撃を受けた時にノックバック
	behavior_ = Beahavior::knockBack;

	if (isInvincible_) {
		isDamage_ = true;
		return;
	}

	// 体力を減らす
	playerHp_ -= damage_;

	// ダメージを受けてから次のダメージを受けるまでの無敵時間
	isInvincible_ = true;
	invincibleTime_ = 60.0f;

	if (playerHp_ <= 0.0f) {
		// 死亡フラグを立てる
		isDead_ = true;
	}

	KamataEngine::Audio::GetInstance()->PlayWave(soundHandleDoun_, false, 0.5f);
}

void Player::OnCollision(const ShieldEnemy* shieldEnemy) {
	(void)shieldEnemy;
	if (Player::IsAttack()) {
		return; // 攻撃中はダメージ無効
	}

	if (isDead_ || behavior_ == Beahavior::knockBack) {
		return;
	}

	// 攻撃を受けた時にノックバック
	behavior_ = Beahavior::knockBack;

	if (isInvincible_) {
		isDamage_ = true;
		return;
	}

	// 体力を減らす
	playerHp_ -= damage_;

	// ダメージを受けてから次のダメージを受けるまでの無敵時間
	isInvincible_ = true;
	invincibleTime_ = 60.0f;

	if (playerHp_ <= 0.0f) {
		// 死亡フラグを立てる
		isDead_ = true;
	}

	KamataEngine::Audio::GetInstance()->PlayWave(soundHandleDoun_, false, 0.5f);
}

/// 描画
void Player::Draw() {
	if (!isDead_) {
		model_->Draw(worldTransform_, *camera_);
		if (behavior_ == Beahavior::kAttack) {
			modelAttack_->Draw(worldTransformAttack_, *camera_);
		}
	}

	// チャージパーティクルの描画
	for (ChargeParticle* chargeParticle : chargeParticles_) {
		chargeParticle->Draw();
	}

#ifdef _DEBUG

	ImGui::Begin("Debug");

	ImGui::Text("playerHp %.1f", playerHp_);
	ImGui::Text("playerinvincibleTime %.1f", invincibleTime_);

	if (isInvincible_) {
		ImGui::Text("playerisInvincible_ :: True");
	} else {
		ImGui::Text("playerisInvincible_ :: false");
	}

	ImGui::End();

#endif // DEBUG
}

// ルートビヘイビアの初期化
void Player::BehaviorRootInitialize() {}

void Player::BehaviorRootUpdate() {
	// 移動処理
	Move();
	// 攻撃ボタンを押した瞬間
	if (KamataEngine::Input::GetInstance()->TriggerKey(DIK_SPACE)) {
		// 地上にいる OR 空中でまだ攻撃していないなら攻撃判定を開始
		if (onGround_ || !hasAttackedInAir_) {
			attackHoldParameter_ = 0;
			isAttackButtonHeld_ = true;
		}
	}

	// 攻撃ボタンを押している間
	if (isAttackButtonHeld_ && KamataEngine::Input::GetInstance()->PushKey(DIK_SPACE)) {
		attackHoldParameter_++;

		// 一定時間押したら溜め攻撃へ移行
		if (attackHoldParameter_ >= chargeAttackLimit_) {
			isAttackButtonHeld_ = false;
			behaviorRequest_ = Beahavior::kchargeAttack;
		}
	}

	// 攻撃ボタンが離された瞬間
	bool isSpacePressed = KamataEngine::Input::GetInstance()->PushKey(DIK_SPACE);

	if (isAttackButtonHeld_ && wasSpacePressed_ && !isSpacePressed) {
		isAttackButtonHeld_ = false;

		// 短押しなら通常攻撃
		KamataEngine::Audio::GetInstance()->PlayWave(soundHandleAttck_, false, 0.5f);
		behaviorRequest_ = Beahavior::kAttack;
	}

	// 次のフレーム用にキーの状態を保存
	wasSpacePressed_ = isSpacePressed;
}

// 攻撃ビヘイビアの初期化
void Player::BehaviorAttackInitialize() {
	// カウンター初期化
	attackParameter_ = 0;
	attackPhase_ = AttackPhase::charge;

	if (!onGround_ && velocity_.y > 0.0f) {
		velocity_.y = 0.0f;
	}
}

// 攻撃ビヘイビアの更新
void Player::BehaviorAttackUpdate() {
	// 攻撃動作用の速度
	KamataEngine::Vector3 velocity{};

	// トランスフォームの値をコピー
	worldTransformAttack_.translation_ = worldTransform_.translation_;
	worldTransformAttack_.rotation_ = worldTransform_.rotation_;

	UpdateWorldTransform(worldTransformAttack_);

	// 予備動作
	attackParameter_++;
	// 指定の時間経過で攻撃終了して通常状態に戻す
	if (attackParameter_ >= 20.0f) {
		behaviorRequest_ = Beahavior::kRoot;
	}

	switch (attackPhase_) {
		// 溜の動作
	case AttackPhase::charge:
	default: {
		float t = static_cast<float>(attackParameter_) / 8.0f;
		worldTransform_.scale_.z = EaseOut(1.0f, 0.3f, t);
		worldTransform_.scale_.y = EaseOut(1.0f, 1.0f, t);
		// 全身動作ヘ移行
		if (attackParameter_ >= 8.0f) {
			attackPhase_ = AttackPhase::dash;
			attackParameter_ = 0; // カウンターをリセット
		}
	} break;
		// 突進動作
	case AttackPhase::dash: {
		float t = static_cast<float>(attackParameter_) / 8.0f;
		worldTransform_.scale_.z = EaseOut(0.3f, 1.3f, t);
		worldTransform_.scale_.y = EaseOut(1.0f, 0.7f, t);
		// 右向きか？
		if (lrDirection_ != LRDirection::kRight) {
			velocity.x = -attackVelocity;
		} else {
			velocity.x = +attackVelocity;
		}

		// 衝突情報を初期化

		CollisionMapInfo collisionMapInfo;
		collisionMapInfo.moveAmount = velocity;

		CheckMapCollision(collisionMapInfo);
		CheckMceilingCollision(collisionMapInfo);
		ApplyMapCollision(collisionMapInfo);
		ChangingGround(collisionMapInfo);

		// 全身動作ヘ移行
		if (attackParameter_ >= 8.0f) {
			attackPhase_ = AttackPhase::recovery;
			attackParameter_ = 0; // カウンターをリセット
			velocity_.x = 0.0f;
		}
	}

	break;
		// 余韻動作
	case AttackPhase::recovery: {
		float t = static_cast<float>(attackParameter_) / 4.0f;
		worldTransform_.scale_.z = EaseOut(1.3f, 1.0f, t);
		worldTransform_.scale_.y = EaseOut(0.7f, 1.0f, t);

	} break;
	}
}

void Player::BehaviorChargeAttackInitialize() {
	attackParameter_ = 0;
	chargeAttackPhase_ = ChrageAttackPhase::charge;

	// 溜め攻撃開始時に横方向の速度を止める
	velocity_.x = 0.0f;

	// 拡大率を初期化
	chargeScale_ = 0.0f;
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};

	// 上昇中なら上昇速度を止める
	if (!onGround_ && velocity_.y > 0.0f) {
		velocity_.y = 0.0f;
	}

	// パーティクル生成タイマーをリセット
	chargeParticleSpawnTimer_ = 0.0f;

	// チャージパーティクルを生成
	for (int i = 0; i < 10; i++) {
		chargeParticles_.push_back(ChargeParticle::Create(worldTransform_.translation_));
	}
}

void Player::BehaviorChargeAttackUpdate() {
	// 攻撃動作用の速度
	KamataEngine::Vector3 velocity{};

	// チャージパーティクルの更新
	for (ChargeParticle* chargeParticle : chargeParticles_) {
		chargeParticle->SetTargetPosition(worldTransform_.translation_);
		chargeParticle->Update();
	}

	// 消滅したパーティクルを削除
	chargeParticles_.remove_if([](ChargeParticle* chargeParticle) {
		if (chargeParticle->IsDead()) {
			delete chargeParticle;
			return true;
		}
		return false;
	});

	// チャージ中だけパーティクルを生成
	if (chargeAttackPhase_ == ChrageAttackPhase::charge) {
		chargeParticleSpawnTimer_ += 1.0f / 60.0f;

		if (chargeParticleSpawnTimer_ >= kChargeParticleSpawnInterval_) {
			chargeParticleSpawnTimer_ -= kChargeParticleSpawnInterval_;

			ChargeParticle* chargeParticle = ChargeParticle::Create(worldTransform_.translation_);
			chargeParticles_.push_back(chargeParticle);
		}
	}

	// 攻撃用モデルの位置と向きを更新
	worldTransformAttack_.translation_ = worldTransform_.translation_;
	worldTransformAttack_.rotation_ = worldTransform_.rotation_;
	UpdateWorldTransform(worldTransformAttack_);

	switch (chargeAttackPhase_) {
	case ChrageAttackPhase::charge:

		// 溜め具合を更新
		chargeScale_ += 0.15f;

		// 縦に伸び縮みさせる
		worldTransform_.scale_.y = 1.0f + std::sin(chargeScale_) * 0.2f;

		// 溜め中はボタンを離すまで待つ
		if (!KamataEngine::Input::GetInstance()->PushKey(DIK_SPACE)) {
			chargeAttackPhase_ = ChrageAttackPhase::relese;
			attackParameter_ = 0;

			// チャージパーティクルをすべて削除
			for (ChargeParticle* chargeParticle : chargeParticles_) {
				delete chargeParticle;
			}
			chargeParticles_.clear();
		}

		// チャージパーティクルの更新
		for (ChargeParticle* chargeParticle : chargeParticles_) {
			chargeParticle->SetTargetPosition(worldTransform_.translation_);
			chargeParticle->Update();
		}

		// 消滅したパーティクルを削除
		chargeParticles_.remove_if([](ChargeParticle* chargeParticle) {
			if (chargeParticle->IsDead()) {
				delete chargeParticle;
				return true;
			}
			return false;
		});

		break;

	case ChrageAttackPhase::relese:
		// 向いている方向へ突進
		if (lrDirection_ == LRDirection::kRight) {
			velocity.x = chargeAttackVelocity;
		} else {
			velocity.x = -chargeAttackVelocity;
		}

		// マップとの衝突判定
		{
			CollisionMapInfo collisionMapInfo;
			collisionMapInfo.moveAmount = velocity;

			CheckMapCollision(collisionMapInfo);
			CheckMceilingCollision(collisionMapInfo);
			ApplyMapCollision(collisionMapInfo);
			ChangingGround(collisionMapInfo);
		}

		attackParameter_++;

		// 10フレーム突進したら余韻へ
		if (attackParameter_ >= 10) {
			chargeAttackPhase_ = ChrageAttackPhase::recovery;
			attackParameter_ = 0;
		}
		break;

	case ChrageAttackPhase::recovery:
		attackParameter_++;

		// プレイヤーのスケールを元に戻す
		worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};


		// 15フレーム経過したら通常状態へ
		if (attackParameter_ >= 15) {
			behaviorRequest_ = Beahavior::kRoot;
		}
		break;
	}
}

void Player::BehaviorKnockBackInitialize() {}

void Player::BehaviorKnockBackUpdate() {

	KamataEngine::Vector3 knockBackVelocity{};

	if (lrDirection_ == LRDirection::kRight) {
		knockBackVelocity.x = -KnockBackVelocity_;
	} else {
		knockBackVelocity.x = KnockBackVelocity_;
	}

	knockBackTParameter_--;

	if (knockBackTParameter_ <= 0) {
		knockBackVelocity.x = 0.0f;
		behaviorRequest_ = Beahavior::kRoot;
		knockBackTParameter_ = knockBackTime_;
	}

	CollisionMapInfo collisionMapInfo;
	collisionMapInfo.moveAmount = knockBackVelocity;

	CheckMapCollision(collisionMapInfo);
	CheckMceilingCollision(collisionMapInfo);
	ApplyMapCollision(collisionMapInfo);
	ChangingGround(collisionMapInfo);
}
bool Player::IsAttack() const { return behavior_ == Beahavior::kAttack || behavior_ == Beahavior::kchargeAttack; }

void Player::GetKnockBack() {
	IsKnockBack_ = true;
	knockBackTParameter_ = knockBackTime_;
}

void Player::AddPosition(const KamataEngine::Vector3& amount) {
	// --- 1. X軸の移動と衝突補正 ---
	if (amount.x != 0.0f) {
		worldTransform_.translation_.x += amount.x;

		// 固定マップとの判定
		CollisionMapInfo infoX;
		infoX.moveAmount = {amount.x, 0.0f, 0.0f};
		if (amount.x > 0.0f) {
			CheckMapCollisionRight(infoX);
		} else {
			CheckMapCollisionLeft(infoX);
		}
		if (infoX.hitWall) {
			worldTransform_.translation_.x += (infoX.moveAmount.x - amount.x);
		}

		// 動くブロックとのX軸衝突
		if (moveBlocks_) {
			worldTransform_.TransferMatrix(); // 位置変更を行列に即時反映
			AABB playerAABB = GetAABB();

			for (auto* moveBlock : *moveBlocks_) {
				if (!moveBlock)
					continue;
				KamataEngine::Vector3 pos = moveBlock->translation_;
				AABB blockAABB;
				blockAABB.min = {pos.x - MapChipField::kBlockWidth / 2.0f, pos.y - MapChipField::kBlockHeight / 2.0f, pos.z - MapChipField::kBlockWidth / 2.0f};
				blockAABB.max = {pos.x + MapChipField::kBlockWidth / 2.0f, pos.y + MapChipField::kBlockHeight / 2.0f, pos.z + MapChipField::kBlockWidth / 2.0f};

				if (IsCollision(playerAABB, blockAABB)) {
					float playerHalfWidth = (playerAABB.max.x - playerAABB.min.x) / 2.0f;
					if (amount.x > 0.0f) {
						// 右移動中：ブロックの左端にピッタリ合わせる
						worldTransform_.translation_.x = blockAABB.min.x - playerHalfWidth;
					} else {
						// 左移動中：ブロックの右端にピッタリ合わせる
						worldTransform_.translation_.x = blockAABB.max.x + playerHalfWidth;
					}
					worldTransform_.TransferMatrix(); // 位置確定
					playerAABB = GetAABB();
				}
			}
		}
	}

	// --- 2. Y軸の移動と衝突補正 ---
	if (amount.y != 0.0f) {
		worldTransform_.translation_.y += amount.y;

		// 固定マップとの判定
		CollisionMapInfo infoY;
		infoY.moveAmount = {0.0f, amount.y, 0.0f};
		if (amount.y > 0.0f) {
			CheckMapCollisionUp(infoY);
		} else {
			CheckMapCollisionDown(infoY);
		}
		if (infoY.ceilingCollision || infoY.landing) {
			worldTransform_.translation_.y += (infoY.moveAmount.y - amount.y);
			if (infoY.landing)
				OnGround();
		}

		// 動くブロックとのY軸衝突
		if (moveBlocks_) {
			worldTransform_.TransferMatrix();
			AABB playerAABB = GetAABB();

			for (auto* moveBlock : *moveBlocks_) {
				if (!moveBlock)
					continue;
				KamataEngine::Vector3 pos = moveBlock->translation_;
				AABB blockAABB;
				blockAABB.min = {pos.x - MapChipField::kBlockWidth / 2.0f, pos.y - MapChipField::kBlockHeight / 2.0f, pos.z - MapChipField::kBlockWidth / 2.0f};
				blockAABB.max = {pos.x + MapChipField::kBlockWidth / 2.0f, pos.y + MapChipField::kBlockHeight / 2.0f, pos.z + MapChipField::kBlockWidth / 2.0f};

				if (IsCollision(playerAABB, blockAABB)) {
					float playerHalfHeight = (playerAABB.max.y - playerAABB.min.y) / 2.0f;
					if (amount.y < 0.0f) {
						// 落下中：着地
						worldTransform_.translation_.y = blockAABB.max.y + playerHalfHeight;
						OnGround();
					} else {
						// 上昇中：天井ヒット
						worldTransform_.translation_.y = blockAABB.min.y - playerHalfHeight;
						velocity_.y = 0.0f;
					}
					worldTransform_.TransferMatrix();
					playerAABB = GetAABB();
				}
			}
		}
	}
}

void Player::OnGround() {
	// 落下速度をリセット（下方向への移動を止める）
	velocity_.y = 0.0f;

	// 接地状態フラグをONにする
	onGround_ = true;

	hasAttackedInAir_ = false;
}

void Player::CheckMovingBlockCollision(const AABB& blockAABB, const KamataEngine::Vector3& blockVelocity) {
	(void)blockVelocity; // 未使用警告防止

	AABB playerAABB = GetAABB();

	// そもそも重なっていないなら無視
	if (!IsCollision(playerAABB, blockAABB)) {
		return;
	}

	// プレイヤーの足元と、ブロックの上面・中心高さ
	float playerBottom = playerAABB.min.y;
	float blockCenterY = (blockAABB.min.y + blockAABB.max.y) / 2.0f;

	// 【着地・乗車判定】
	// 落下中、またはすでに接地している（onGround_）場合は上乗せを最優先
	if (onGround_ || (playerBottom >= blockCenterY - 0.2f && velocity_.y <= 0.1f)) {
		// ブロックの上にピッタリ乗せる（Y位置固定）
		worldTransform_.translation_.y = blockAABB.max.y + (kHeight / 2.0f);
		onGround_ = true;
		velocity_.y = 0.0f;

		// 向きを変えた瞬間に横に弾かれないよう、乗っている間はここで処理を終了する！
		UpdateWorldTransform(worldTransform_);
		return;
	}

	float overlapLeft = playerAABB.max.x - blockAABB.min.x;
	float overlapRight = blockAABB.max.x - playerAABB.min.x;

	if (overlapLeft < overlapRight) {
		worldTransform_.translation_.x -= overlapLeft;
	} else {
		worldTransform_.translation_.x += overlapRight;
	}

	UpdateWorldTransform(worldTransform_);
}

Player::~Player() {
	for (ChargeParticle* chargeParticle : chargeParticles_) {
		delete chargeParticle;
	}

	chargeParticles_.clear();
}