#pragma once
#include "ChargeParticle.h"
#include "Collision.h"
#include "KamataEngine.h"
#include "LRDirection.h"
#include <list>

class MapChipField;

class Enemy;

class ShieldEnemy;

class Player {
public:
	~Player();

	/// <summary>
	/// 初期化
	/// </summary>

	void Initialize(KamataEngine::Model* model, KamataEngine::Model* modelAttack, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);

	/// 更新
	void Update();

	/// 描画
	void Draw();

	// ワールドトランスフォームを取得するゲッター
	const KamataEngine::WorldTransform& GetWorldTransform() const { return worldTransform_; }

	const KamataEngine::Vector3& GetVelocity() const { return velocity_; }

	void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; }

	// マップとあたり判定情報
	struct CollisionMapInfo {
		// 天井衝突フラグ
		bool ceilingCollision = false;
		// 着地フラグ
		bool landing = false;
		// 壁接触フラグ
		bool hitWall = false;
		// 移動量
		KamataEngine::Vector3 moveAmount = {0, 0, 0};
	};

	// 角
	enum Corner {
		kRightBottom, // 右下
		kLeftBottom,  // 左下
		kRightTop,    // 右上
		kLeftTop,     // 左上

		kNumCorner // 要素数
	};

	void ApplyMapCollision(const CollisionMapInfo& info);

	void CheckMceilingCollision(const CollisionMapInfo& info);

	// AABBを取得する関数
	AABB GetAABB();

	// 衝突応答
	void OnCollision(const Enemy* enemy);

	// 衝突応答
	void OnCollision(const ShieldEnemy* ShieldEnemy);

	// デスフラグのゲッター
	bool isDead() const { return isDead_; }

	// ワールド座標を取得
	KamataEngine::Vector3 GetWorldPosition() const;

	bool IsAttack() const;

	LRDirection lrDirection_ = LRDirection::kRight;

	LRDirection GetDirection() const { return lrDirection_; }

	bool IsKnockBack_ = false;

	void GetKnockBack();

	void AddPosition(const KamataEngine::Vector3& amount);

	// 位置を直接設定するセッター
	void SetPosition(const KamataEngine::Vector3& position) { worldTransform_.translation_ = position; }

	// 動く床に乗った際の着地・落下停止処理
	void OnGround();

	void AddVelocity(const KamataEngine::Vector3& amount) { velocity_ += amount; }

	// 動く壁（ブロック）との押し戻し処理
	void CheckMovingBlockCollision(const AABB& blockAABB, const KamataEngine::Vector3& blockVelocity);

	void SetMoveBlocks(const std::vector<KamataEngine::WorldTransform*>* moveBlocks) { moveBlocks_ = moveBlocks; }

	bool IsWarpRequested() const { return isWarpRequested_; }
	void ResetWarpRequest() { isWarpRequested_ = false; }

	uint32_t soundHandleAttck_ = 0;
	uint32_t soundHandleDoun_ = 0;
	uint32_t soundHandleWarp_ = 0;

	bool isDamage_ = false;

	bool ConsumeDamageFlag() {
		if (isDamage_) {
			isDamage_ = false;
			return true;
		}
		return false;
	}

private:
	// ワールド変換データ
	KamataEngine::WorldTransform worldTransform_;
	// モデル
	KamataEngine::Model* model_ = nullptr;

	KamataEngine::Camera* camera_ = nullptr;

	// 速度
	KamataEngine::Vector3 velocity_ = {0, 0, 0};
	// 加速度
	static inline const float kAcceleration = 0.05f;
	// 最大速度制限
	static inline const float kLimitRunSpeed = 0.1f;
	// 逆方向入力時の減衰率
	static inline const float kAttenuation = 0.1f;

	// 旋回開始時の角度
	float turnFirstRotationY_ = 0.0f;
	// 旋回タイマー
	float turnTimer_ = 0.0f;
	// 旋回時間<秒>
	static inline const float kTurnTime = 0.1f;

	// 接地状態フラグ
	bool onGround_ = true;

	// 重力加速度（下方向)
	static inline const float kGravityAcceleration = 0.05f;
	// 最大落下速度（下方向）
	static inline const float KLimitFallSpeed = 0.3f;
	// ジャンプ初速（上方向）
	static inline const float kJumpAcceleration = 0.6f;

	// マップチップによるフィールド
	MapChipField* mapChipField_ = nullptr;

	// キャラクターのあたり判定サイズ
	static inline const float kWidth = 0.8f;
	static inline const float kHeight = 0.8f;

	// 着地時の速度減衰率
	static inline const float kAttenuationWall = 0.5f;

	// プレイヤーの移動関数
	void Move();

	// マップの当たり判定
	void CheckMapCollision(CollisionMapInfo& info);

	void CheckSpikeCollision();

	// 各方向のマップ当たり判定
	bool CheckMapCollisionUp(CollisionMapInfo& info);
	bool CheckMapCollisionDown(CollisionMapInfo& info);
	bool CheckMapCollisionRight(CollisionMapInfo& info);
	bool CheckMapCollisionLeft(CollisionMapInfo& info);

	KamataEngine::Vector3 CornerPosition(const KamataEngine::Vector3& center, Player::Corner corner);

	static inline const float kBlank = 0.1f;
	static inline const float kGroundBlank = 1.0f;

	// 接地時の速度減衰率
	static inline const float kAttenuationLanding = 0.5f;

	// 接地状態の切り替え関数
	void ChangingGround(const CollisionMapInfo& info);

	// 壁に接触している場合の処理
	void ChekWallCollision(const CollisionMapInfo& info);

	// デスフラグ
	bool isDead_ = false;

	// 通常行動更新
	void BehaviorRootUpdate();

	// 通常行動初期化
	void BehaviorRootInitialize();

	// 攻撃行動
	void BehaviorAttackUpdate();
	// 攻撃行動初期化
	void BehaviorAttackInitialize();

	// 溜め攻撃行動初期化
	void BehaviorChargeAttackInitialize();

	// 溜め攻撃行動
	void BehaviorChargeAttackUpdate();

	// ノックバック行動の初期化
	void BehaviorKnockBackInitialize();

	// ノックバック行動
	void BehaviorKnockBackUpdate();

	enum class Beahavior {
		kRoot,         // 通常状態
		kAttack,       // 攻撃中
		kUnnown,       // リクエスト無し
		knockBack,     // ノックバック
		kchargeAttack, // 溜攻撃
	};

	// ふるまい
	Beahavior behavior_ = Beahavior::kRoot;

	// 次のふるまいをリクエスト
	Beahavior behaviorRequest_ = Beahavior::kUnnown;

	// 攻撃ギミックの経過時間カウンター
	uint32_t attackParameter_ = 0;

	// 攻撃フェーズ(型)
	enum class AttackPhase {
		charge,   // 溜め
		dash,     // 突進
		recovery, // 余韻
	};

	// 現在の攻撃フェーズ(変数)
	AttackPhase attackPhase_;

	// 攻撃中の速度
	float attackVelocity = 0.4f;
	KamataEngine::Model* modelAttack_ = nullptr;
	KamataEngine::WorldTransform worldTransformAttack_;

	// 溜攻撃フェーズ（型）
	enum class ChrageAttackPhase {
		charge,   // 溜め
		relese,   // 解放
		recovery, // 余韻
	};

	// 現在の溜め攻撃フェーズ（変数）
	ChrageAttackPhase chargeAttackPhase_;
	// 溜め攻撃中の速度
	float chargeAttackVelocity = 0.5f;

	int knockBackTParameter_ = 0;
	static const int knockBackTime_ = 10;
	float KnockBackVelocity_ = 0.4f;

	const std::vector<KamataEngine::WorldTransform*>* moveBlocks_ = nullptr;

	bool isWarpRequested_ = false;

	bool hasAttackedInAir_ = false;

	bool isPlayedDounSound_ = false;

	// 攻撃ボタンを押しているか
	bool isAttackButtonHeld_ = false;

	bool wasSpacePressed_ = false;

	// プレイヤーの体力
	float playerHp_ = 3.0f;

	// 敵にぶつかった時に浮けるダメージ
	float damage_ = 1.0f;

	// 無敵抽か
	bool isInvincible_ = false;

	// プレイヤーが敵にぶつかった時の無敵時間
	float invincibleTime_ = 0.0f;

	// 攻撃ボタンを押してる時間
	float attackHoldParameter_ = 0.0f;

	// 溜め攻撃に切り替える時間
	float chargeAttackLimit_ = 20.0f;

	// 溜め中の伸び縮み用
	float chargeScale_ = 0.0f;

	// チャージパーティクル
	std::list<ChargeParticle*> chargeParticles_;

	// パーティクル生成用
	float chargeParticleSpawnTimer_ = 0.0f;
	const float kChargeParticleSpawnInterval_ = 0.1f;
};
