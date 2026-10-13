#pragma once
#include "Collision.h"
#include "KamataEngine.h"

class MapChipField;

class Player;

class GameScene;

class Enemy {

public:
	/// <summary>
	/// 初期化
	/// </summary>

	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);

	/// 更新
	void Update();

	/// 描画
	void Draw();

	// ワールドトランスフォームを取得するゲッター
	const KamataEngine::WorldTransform& GetWorldTransform() const { return worldTransform_; }

	void SetMapChipField(MapChipField* mapChipField) { mapChipField_ = mapChipField; }

	// AABBを取得
	AABB GetAABB();

	// ワールド座標を取得
	KamataEngine::Vector3 GetWorldPosition();

	// 衝突応答
	void OnCollision(const Player* player);

	// デスフラグ
	bool isDead_ = false;

	bool isDead() const { return isDead_; }

	bool isCollisionDisabled_ = false;

	bool IsCollisionDisabled() const { return isCollisionDisabled_; }

	void SetGameScene(GameScene* gameScene) { gameScene_ = gameScene; }

	// 四角の定義
	enum Corner {
		kRightBottom, // 右下
		kLeftBottom,  // 左下
		kRightTop,    // 右上
		kLeftTop,     // 左上
		kNumCorner    // 要素数
	};

	struct CollisionMapInfo {
		KamataEngine::Vector3 moveAmount;
		bool hitWall = false;
	};

private:
	// ワールド変換データ
	KamataEngine::WorldTransform worldTransform_;
	// モデル
	KamataEngine::Model* model_ = nullptr;

	// マップチップによるフィールド
	MapChipField* mapChipField_ = nullptr;

	KamataEngine::Camera* camera_ = nullptr;

	// キャラクターのあたり判定サイズ
	static inline const float kWidth = 0.8f;
	static inline const float kHeight = 0.8f;

	// 歩行の速さ
	static inline const float kWalkSpeed = 0.05f;

	// 速度
	KamataEngine::Vector3 velocity_ = {};

	// 最初の角度「度」
	static inline const float kWalkMotionAngleStart = -45.0f;
	// 最後の角度「度」
	static inline const float kWalkMotionAngleEnd = 90.0f;
	// アニメーションの周期となる時間「秒」
	static inline const float kWalkMotionTime = 1.0f;

	// 経過時間
	float walkTimer_ = 0.0f;

	// 通常行動更新
	void BehaviorRootUpdate();

	// 通常行動初期化
	void BehaviorRootInitialize();

	// 攻撃行動
	void BehaviorDeathUpdate();
	// 攻撃行動初期化
	void BehaviorDeathInitialize();

	enum class Beahavior {
		kRoot,  // 通常状態
		kDeath, // 脂肪演出中
		kUnnown // リクエスト無し
	};

	// 振る舞い
	Beahavior behavior_ = Beahavior::kRoot;

	// 次のふるまいをリクエスト
	Beahavior behaviorRequest_ = Beahavior::kUnnown;

	uint32_t deathParameter_ = 0;

	// ヒットエフェクトを出すまでの時間
	uint32_t hitEffectDelay_ = 15;

	// ヒットエフェクトを生成したか
	bool isHitEffectCreated_ = false;

	KamataEngine::WorldTransform worldTransformDeath_;

	GameScene* gameScene_ = nullptr;

	// 角の座標計算
	KamataEngine::Vector3 CornerPosition(const KamataEngine::Vector3& center, Corner corner);

	// マップ衝突判定（左右）
	bool CheckMapCollisionRight(CollisionMapInfo& info);
	bool CheckMapCollisionLeft(CollisionMapInfo& info);

	// 反転関数
	void Turn();

	uint32_t soundHandleEnemyDoun_ = 0;

	// 死亡時の吹っ飛び速度
	KamataEngine::Vector3 deathVelocity_ = {0.0f, 0.0f, 0.0f};

	// 死亡時の重力
	float deathGravity_ = 0.02f;

	// 死亡時の回転角度
	float deathRotationSpeed_ = 0.0f;
};
