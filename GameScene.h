#pragma once
#include "CameraController.h"
#include "Collision.h"
#include "DeathParticles.h"
#include "Enemy.h"
#include "Fade.h"
#include "GuardEffect.h"
#include "HitEffect.h"
#include "KamataEngine.h"
#include "MapchipField.h"
#include "Player.h"
#include "ShieldEnemy.h"
#include "skydome.h"
#include <vector>

class StageManager;

// 看板情報構造体
struct SignInfo {
	std::unique_ptr<KamataEngine::WorldTransform> worldTransform;
	uint8_t subID = 0;
};

// ワープ情報構造体
struct WarpInfo {
	std::unique_ptr<KamataEngine::WorldTransform> worldTransform;
	uint8_t subID = 0;
};

struct GoalInfo {
	std::unique_ptr<KamataEngine::WorldTransform> worldTransform;
};

struct MovingPlatformGroup {
	std::vector<KamataEngine::WorldTransform*> blocks; // 描画用の各マス用Transform
	KamataEngine::Vector3 basePosition;                // 基準座標（一番左の座標）
	float totalWidth = 0.0f;                           // 全体の横幅
	uint8_t direction = 0;                             // 移動方向 (0:上下, 1:左右)
	float startX = 0.0f;                               // 初期X
	float startY = 0.0f;                               // 初期Y
	bool moveUp = true;                                // 移動向きフラグ
	KamataEngine::Vector3 previousPos;                 // 前フレームの基準座標
};

// ゲームシーン
class GameScene {

public:
	// 初期化
	void Initialize(StageManager* stageDataManager);

	// 更新
	void Update();

	void Finalize();

	// 描画
	void Draw();

	std::vector<std::vector<KamataEngine::WorldTransform*>> worldTransformBlocks_;

	std::vector<KamataEngine::WorldTransform*> worldTransformSpikes_;

	std::vector<KamataEngine::WorldTransform*> worldTransformMoveBlocks_;

	std::vector<uint8_t> moveBlockDirection_;
	std::vector<float> moveBlockStartX_;
	std::vector<float> moveBlockStartY_;
	std::vector<bool> moveBlockMoveUp_;
	std::vector<float> moveBlockWidths_;

	std::vector<KamataEngine::Vector3> moveBlockPreviousPositions_;

	std::vector<MovingPlatformGroup> movingPlatforms_;

	// 敵のデスパーティクル
	std::list<DeathParticles*> enemyDeathParticles_;

	// デバッグカメラ有効
	bool isDebugCameraActive_ = false;

	void SetMoveRange(KamataEngine::Vector3 min, KamataEngine::Vector3 max);

	void CheckAllCollisions();

	// フェーズの切り替え
	void ChengePhase();

	// エフェクトを発生
	void CreateHitEffect(const KamataEngine::Vector3& position);

	// ガードエフェクトを発生
	void CreateGuardEffect(const KamataEngine::Vector3& position);

	// 敵の死亡パーティクルを生成
	void CreateDeathParticles(const KamataEngine::Vector3& position);

	// 次のステージへ切り替える関数
	void ChangeStage();

	// タイトル遷移要請の取得用
	bool IsTitleRequested() const { return isTitleRequested_; }

	void ReloadStage();

private:
	// テクスチャハンドル
	uint32_t textureHandle_ = 0;

	uint32_t modelHandle_ = 0;

	uint32_t blockTextureHandle_ = 0;

	uint32_t enemyTextureHandle_ = 0;

	uint32_t enemyModelHandle_ = 0;

	uint32_t shieldEnemyTextureHandle_ = 0;

	uint32_t shieldEnemyModelHandle_ = 0;

	uint32_t dethParticlesTextureHnandle_ = 0;

	uint32_t dethParticlesModelHandle_ = 0;

	uint32_t modelAttackHandle_ = 0;

	uint32_t playerAttackTextureHandle_ = 0;

	uint32_t hitEffectTexturelHandle_ = 0;


	uint32_t guardEffectTextureHandle_ = 0;

	uint32_t spikeTextureHandle_ = 0;

	uint32_t goalTextureHandle_ = 0;

	uint32_t warpTextureHandle_ = 0;

	uint32_t pauseTextureHandle_ = 0;
	KamataEngine::Sprite* pauseSprite_ = nullptr;

	KamataEngine::Sprite* sprite_ = nullptr;

	std::vector<uint32_t> signTextureHandles_;

	// 3Dのモデル
	KamataEngine::Model* model_ = nullptr;

	// 3Dのモデル（ブロック用）
	KamataEngine::Model* modelBlock_ = nullptr;

	KamataEngine::Model* modelEnemy_ = nullptr;

	KamataEngine::Model* modelShieldEnemy_ = nullptr;

	KamataEngine::Model* modeldethParticles_ = nullptr;

	KamataEngine::Model* modelAttack_ = nullptr;

	KamataEngine::Model* hitEffectModel_ = nullptr;

	KamataEngine::Model* chargeParticleModel_ = nullptr;

	KamataEngine::Model* guardEffectModel_ = nullptr;

	KamataEngine::Model* modelSpike_ = nullptr;

	uint32_t pauseTextTextureHandle_ = 0;
	KamataEngine::Sprite* pauseTextSprite_ = nullptr;

	// カメラ
	KamataEngine::Camera camera_;

	CameraController* cameraController_ = nullptr;

	// 敵の複数化
	std::list<Enemy*> enemies_;

	// 盾持ち敵の複数化
	std::list<ShieldEnemy*> shieldEnemies_;

	// ヒットエフェクト
	std::list<HitEffect*> hitEffects_;

	// ガードエフェクト
	std::list<GuardEffect*> guardEffects_;

	std::vector<KamataEngine::Model*> signModels_; // 看板の3Dモデル
	std::vector<SignInfo> signs_;                  // ステージ内の看板リスト

	KamataEngine::Model* modelWarp_ = nullptr; // ワープの3Dモデル（ポータル等）
	std::vector<WarpInfo> warps_;              // ステージ内のワープリスト

	std::vector<GoalInfo> goals_;
	KamataEngine::Model* modelGoal_ = nullptr;

	bool isClear_ = false;
	bool isGameOver_ = false;
	bool isTitleRequested_ = false;

	KamataEngine::WorldTransform workWorldTransform_;

	enum class SceneState {
		kPlay,     // 通常プレイ中
		kPause,    // ポーズ中6
		kGameOver, // ゲームオーバー
	};

	SceneState sceneState_ = SceneState::kPlay;

	uint32_t soundHandleGoal_ = 0;

	uint32_t soundHandleBGM_ = 0;
	uint32_t voiceHandleBGM_ = 0;

public:
	// デスフラグのgetter
	bool IsFinished() const { return finished_; }

	// クリアしたかどうか
	bool IsClear() const { return isClear_; }

	// ゲームオーバーになったかどうか
	bool IsGameOver() const { return isGameOver_; }

	// デバッグカメラ
	KamataEngine::DebugCamera* debugCamera_ = nullptr;

	// 自キャラ
	Player* player_ = nullptr;

	// 天球
	skydome* skydome_ = nullptr;

	// 敵キャラ
	Enemy* enemy_ = nullptr;

	// シールド持ち敵キャラ
	ShieldEnemy* shieldEnemy_ = nullptr;

	// 天球用3Dモデル
	KamataEngine::Model* modelSkydome_ = nullptr;

	// マップチップフィールド
	MapChipField* mapChipField_;
	void GenerateObjects();

	KamataEngine::Vector3 minCameraPos_ = {0, 0, 0};
	KamataEngine::Vector3 maxCameraPos_ = {0, 0, 0};

	DeathParticles* dethParticles_ = nullptr;

	// ゲームのフェーズ（型）
	enum class Phase {
		kPlay,  // ゲームプレイ
		kDeath, // デス演出
		kPause, // ポーズ
	};

	// ゲームの現在フェーズ(変数)
	Phase phase_;

	// 終了フラグ
	bool finished_ = false;

	// フェード
	Fade* fade_ = nullptr;
	float fadeTime_ = 1.0f;

	// リロード要求フラグ
	bool reloadRequested_ = false;

	// リロード要求フラグのgetter
	bool GetReloadRequested() const { return reloadRequested_; }

	// ステージマネージャ参照用のポインタ
	StageManager* stageManager_ = nullptr;

	// 画面シェイク
	float cameraShakeTimer_ = 0.0f;
	float cameraShakeDuration_ = 0.25f;
	float cameraShakeStrength_ = 0.15f;
};
