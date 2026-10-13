#include "GameScene.h"
#include "Collision.h"
#include "StageManager.h"
#include "WorldTransformEx.h"
#include <cmath>

using namespace KamataEngine;

void GameScene::Initialize(StageManager* stageDataManager) {

	// 引数をメンバ変数に記録する
	stageManager_ = stageDataManager;

	// ファイル名を指定してテクスチャファイルを読み込む
	textureHandle_ = TextureManager::Load("./Resources/player/player.png");

	enemyTextureHandle_ = TextureManager::Load("./Resources/enemy/enemy.png");

	shieldEnemyTextureHandle_ = TextureManager::Load("./Resources/shieldEnemy/shieldEnemy.png");

	blockTextureHandle_ = TextureManager::Load("./Resources/block/block.png");

	dethParticlesTextureHnandle_ = TextureManager::Load("./Resources/deathParticle/dethParticle.png");

	playerAttackTextureHandle_ = TextureManager::Load("./Resources/hit_effect/hit_effect.png");

	hitEffectTexturelHandle_ = TextureManager::Load("./Resources/particle/circle.png");

	guardEffectTextureHandle_ = TextureManager::Load("./Resources/ring/ring.png");

	spikeTextureHandle_ = TextureManager::Load("./Resources/spike/spike.png");

	signTextureHandles_.clear();
	signTextureHandles_.push_back(TextureManager::Load("./Resources/sign/sign0.png"));
	signTextureHandles_.push_back(TextureManager::Load("./Resources/sign/sign1.png"));
	signTextureHandles_.push_back(TextureManager::Load("./Resources/sign/sign2.png"));
	signTextureHandles_.push_back(TextureManager::Load("./Resources/sign/sign3.png"));

	goalTextureHandle_ = TextureManager::Load("./Resources/goal/goal.png");

	warpTextureHandle_ = TextureManager::Load("./Resources/warp/warp.png");

	pauseTextureHandle_ = TextureManager::Load("./Resources/pause/pause.png");
	pauseSprite_ = Sprite::Create(pauseTextureHandle_, {0.0f, 0.0f});
	pauseSprite_->SetSize({1280.0f, 720.0f});
	isTitleRequested_ = false;

	pauseTextTextureHandle_ = TextureManager::Load("./Resources/pause/pause_text.png");
	pauseTextSprite_ = Sprite::Create(pauseTextTextureHandle_, {-20.0f, -20.0f});

	soundHandleGoal_ = KamataEngine::Audio::GetInstance()->LoadWave("Audio/goal.wav");

	soundHandleBGM_ = KamataEngine::Audio::GetInstance()->LoadWave("Audio/gameScene.wav");
	voiceHandleBGM_ = KamataEngine::Audio::GetInstance()->PlayWave(soundHandleBGM_, true, 0.3f);

	// ゲームプレイフェーズから開始
	phase_ = Phase::kPlay;

	// スプライトの生成
	sprite_ = Sprite::Create(textureHandle_, {640, 360});

	// 3Dモデルの生成
	model_ = Model::CreateFromOBJ("player", true);

	// 3Dモデルの生成（ブロック用）
	modelBlock_ = Model::CreateFromOBJ("block", true);

	// 3Dモデルの生成（トゲ用）
	modelSpike_ = Model::CreateFromOBJ("spike", true);

	// 敵3Dモデルの生成
	modelEnemy_ = Model::CreateFromOBJ("enemy", true);

	// 盾持ち敵3Dモデルの生成
	modelShieldEnemy_ = Model::CreateFromOBJ("shieldEnemy", true);

	modeldethParticles_ = Model::CreateFromOBJ("deathParticle", true);

	modelAttack_ = Model::CreateFromOBJ("hit_effect", true);

	hitEffectModel_ = KamataEngine::Model::CreateFromOBJ("particle", true);

	// チャージパーティクル用モデル
	chargeParticleModel_ = Model::CreateFromOBJ("particle", true);

	guardEffectModel_ = Model::CreateFromOBJ("ring", true);

	// 看板モデルの読み込み
	signModels_.clear();
	for (size_t i = 0; i < signTextureHandles_.size(); ++i) {
		signModels_.push_back(KamataEngine::Model::CreateFromOBJ("sign", true));
	}

	modelGoal_ = Model::CreateFromOBJ("goal", true);

	modelWarp_ = Model::CreateFromOBJ("warp", true);

	// カメラの初期化
	camera_.Initialize();

	// デバッグカメラの生成
	debugCamera_ = new DebugCamera(1280, 720);

	// マップチップ
	mapChipField_ = new MapChipField;
	// 現在のステージデータを取得する
	const StageData& stageData = stageManager_->GetCurrentStageData();

	// ステージファイルパスの生成
	std::string stageFilename = "Resources/field/" + stageData.name + ".csv";

	// ステージファイルの読み込み
	mapChipField_->LoadMapChipCsv(stageFilename);

	GenerateObjects();

	// ヒットエフェクト
	HitEffect::SetModel(hitEffectModel_);
	HitEffect::SetCamera(&camera_);

	// チャージパーティクル
	ChargeParticle::SetModel(chargeParticleModel_);
	ChargeParticle::SetCamera(&camera_);

	GuardEffect::SetModel(guardEffectModel_);
	GuardEffect::SetCamera(&camera_);

	// カメラコントローラー生成
	cameraController_ = new CameraController();

	// カメラを渡して初期化
	cameraController_->Initialize(&camera_);

	// プレイヤーを追従対象にする
	cameraController_->SetTarget(player_);

	Rect area = {0.0f, 100.0f, 0.0f, 100.0f};
	cameraController_->SetMovableArea(area);

	// 初期位置合わせ
	cameraController_->Reset();

	// カメラの移動範囲を設定

	// 天球用３Dモデルの生成
	modelSkydome_ = Model::CreateFromOBJ("skydome", true);

	skydome_ = new skydome();

	skydome_->Initialize(modelSkydome_, &camera_);

	fade_ = new Fade();
	fade_->Initialize();
	fade_->Start(Fade::Status::FadeIn, fadeTime_);

	workWorldTransform_.Initialize();
}

// フェーズの切り替え
void GameScene::ChengePhase() {
	switch (phase_) {
	case Phase::kPlay:
		// ゲームプレイのフェーズ

		if (player_->isDead()) {
			// 脂肪演出フェーズに切り替え
			phase_ = Phase::kDeath;
			// 自キャラの座標を取得
			const Vector3& deathParticlesPosition = player_->GetWorldPosition();

			// デスパーティクル発生初期化
			dethParticles_ = new DeathParticles;
			dethParticles_->Initialize(modeldethParticles_, &camera_, deathParticlesPosition);
		}

		break;
	case Phase::kDeath:
		// デス演出フェーズの処理
		if (dethParticles_ && dethParticles_->IsFinished()) {
			isGameOver_ = true;
			finished_ = true;
		}
		break;
	}
}

void GameScene::Update() {

	// === Pキーでポーズの切り替え ===
	if (Input::GetInstance()->TriggerKey(DIK_P)) {
		if (phase_ == Phase::kPlay) {
			phase_ = Phase::kPause;
		} else if (phase_ == Phase::kPause) {
			phase_ = Phase::kPlay;
		}
	}

	// === ポーズ中の更新処理 ===
	if (phase_ == Phase::kPause) {

		if (Input::GetInstance()->TriggerKey(DIK_R)) {
			ReloadStage();
			return;
		}

		// Tキーを押したらタイトルへ戻るフラグを立てる
		if (Input::GetInstance()->TriggerKey(DIK_T)) {
			isTitleRequested_ = true;
		}

		if (fade_) {
			fade_->Update();
		}
		return;
	}

	ChengePhase();

	switch (phase_) {
	case Phase::kPlay:
		// ゲームプレイフェーズの処理
		if (player_->IsWarpRequested()) {
			ChangeStage();
			return; // 今フレームの残り更新処理をスキップ
		}
		break;
	case Phase::kDeath:
		// デス演出フェーズの処理
		break;
	}

	for (Enemy* enemy : enemies_) {
		enemy->Update();
	}

	for (HitEffect* hitEffect : hitEffects_) {
		hitEffect->Update();
	}

	enemies_.remove_if([](Enemy* enemy) {
		if (enemy->isDead_) {
			delete enemy;
			return true;
		}
		return false;
	});

	shieldEnemies_.remove_if([](ShieldEnemy* shieldEnemy) {
		if (shieldEnemy->isDead_) {
			delete shieldEnemy;
			return true;
		}
		return false;
	});

	for (ShieldEnemy* shieldEnemy : shieldEnemies_) {
		shieldEnemy->Update();
	}

	for (GuardEffect* guardEffect : guardEffects_) {
		guardEffect->Update();
	}

	// デスパーティクル
	if (dethParticles_) {
		dethParticles_->Update();
	}

	for (DeathParticles* deathParticles : enemyDeathParticles_) {
		deathParticles->Update();
	}

	// デバッグカメラの更新
	debugCamera_->Update();

	// カメラの処理
	if (isDebugCameraActive_) {

		camera_.matView = debugCamera_->GetCamera().matView;

		camera_.matProjection = debugCamera_->GetCamera().matProjection;

		camera_.translation_ = debugCamera_->GetCamera().translation_;

		// ビュープロジェクション行列の転送
		camera_.TransferMatrix();

	} else {

		// カメラコントローラー更新
		cameraController_->Update();

		// ダメージを受けたら画面シェイクを開始
		if (player_->ConsumeDamageFlag()) {
			cameraShakeTimer_ = cameraShakeDuration_;
		}

		// 画面シェイク
		if (cameraShakeTimer_ > 0.0f) {
			cameraShakeTimer_ -= 1.0f / 60.0f;

			float shakeTime = cameraShakeTimer_ * 60.0f;

			camera_.translation_.x += std::sin(shakeTime * 2.5f) * cameraShakeStrength_;

			camera_.translation_.y += std::cos(shakeTime * 3.5f) * cameraShakeStrength_;
		}

		// カメラ行列更新
		camera_.UpdateMatrix();
	}

	// ブロックの更新
	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			if (!worldTransformBlock)
				continue;

			// 定数バッファに転送する
			UpdateWorldTransform(*worldTransformBlock);
		}
	}

	// 動くブロックの更新
	for (auto& platform : movingPlatforms_) {

		// 前フレームの基準位置を保存
		platform.previousPos = platform.basePosition;

		// 移動量を計算
		Vector3 moveAmount = {0.0f, 0.0f, 0.0f};

		// 上下移動
		if (platform.direction == 0) {
			if (platform.moveUp) {
				moveAmount.y = 0.01f;
				platform.basePosition.y += 0.01f;
				if (platform.basePosition.y >= platform.startY + 3.0f) {
					platform.moveUp = false;
				}
			} else {
				moveAmount.y = -0.01f;
				platform.basePosition.y -= 0.01f;
				if (platform.basePosition.y <= platform.startY) {
					platform.moveUp = true;
				}
			}
		}
		// 左右移動
		else if (platform.direction == 1) {
			if (platform.moveUp) {
				moveAmount.x = 0.01f;
				platform.basePosition.x += 0.01f;
				if (platform.basePosition.x >= platform.startX + 3.0f) {
					platform.moveUp = false;
				}
			} else {
				moveAmount.x = -0.01f;
				platform.basePosition.x -= 0.01f;
				if (platform.basePosition.x <= platform.startX) {
					platform.moveUp = true;
				}
			}
		}

		//  描画用の各ブロック全員を移動量ぶん動かす
		for (WorldTransform* block : platform.blocks) {
			block->translation_ += moveAmount;
			UpdateWorldTransform(*block);
		}
	}

	// 自キャラの更新
	player_->Update();

	// トゲの更新
	for (WorldTransform* worldTransformSpike : worldTransformSpikes_) {

		if (!worldTransformSpike)
			continue;

		UpdateWorldTransform(*worldTransformSpike);
	}

	// 看板の更新
	for (SignInfo& sign : signs_) {
		UpdateWorldTransform(*sign.worldTransform);
	}

	// ワープの更新
	for (WarpInfo& warp : warps_) {
		warp.worldTransform->rotation_.z += 0.05f;
		UpdateWorldTransform(*warp.worldTransform);
	}

	for (GoalInfo& goal : goals_) {
		goal.worldTransform->rotation_.y += 0.03f;
		UpdateWorldTransform(*goal.worldTransform);
	}

	// すべての当たり判定を行う
	CheckAllCollisions();

	// 天球の更新
	skydome_->Update();

	fade_->Update();

	 #ifdef _DEBUG
	
		ImGui::Begin("Debug");
	
		if (ImGui::Button("Reload")) {
			reloadRequested_ = true;
		}

		ImGui::End();
	
	 #endif // DEBUG
}

void GameScene::Draw() {
	// 3D描画開始
	Model::PreDraw();

	// 天球の描画
	skydome_->Draw();

	// 看板の描画
	for (SignInfo& sign : signs_) {
		uint32_t texIndex = (sign.subID < signTextureHandles_.size()) ? sign.subID : 0;

		// インデックスの安全チェックを行って描画
		if (texIndex < signModels_.size() && signModels_[texIndex]) {
			signModels_[texIndex]->Draw(*sign.worldTransform, camera_, signTextureHandles_[texIndex]);
		}
	}

	// ブロックの描画
	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {

		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			if (!worldTransformBlock)
				continue;
			modelBlock_->Draw(*worldTransformBlock, camera_, blockTextureHandle_);
		}
	}

	// 動くブロックの描画
	for (const auto& platform : movingPlatforms_) {
		for (WorldTransform* block : platform.blocks) {
			if (block) {
				modelBlock_->Draw(*block, camera_, blockTextureHandle_);
			}
		}
	}

	// トゲの描画
	for (WorldTransform* worldTransformSpike : worldTransformSpikes_) {

		if (!worldTransformSpike)
			continue;

		modelSpike_->Draw(*worldTransformSpike, camera_);
	}

	// ワープの描画
	for (WarpInfo& warp : warps_) {
		if (modelWarp_) {
			modelWarp_->Draw(*warp.worldTransform, camera_, warpTextureHandle_);
		}
	}

	for (GoalInfo& goal : goals_) {
		if (modelGoal_) {
			modelGoal_->Draw(*goal.worldTransform, camera_);
		}
	}

	// デスパーティクル
	if (dethParticles_) {
		dethParticles_->Draw();
	}

	player_->Draw();

	// 敵のデスパーティクル
	for (DeathParticles* deathParticles : enemyDeathParticles_) {
		deathParticles->Draw();
	}

	for (HitEffect* hitEffect : hitEffects_) {
		hitEffect->Draw();
	}

	for (GuardEffect* guardEffect : guardEffects_) {
		guardEffect->Draw();
	}

	for (Enemy* enemy : enemies_) {
		enemy->Draw();
	}

	for (ShieldEnemy* shieldEnemy : shieldEnemies_) {
		shieldEnemy->Draw();
	}

	// 3D描画終了
	Model::PostDraw();

	Sprite::PreDraw();

	if (pauseTextSprite_) {
		pauseTextSprite_->Draw();
	}

	// ポーズ画面の描画
	if (phase_ == Phase::kPause) {
		if (pauseSprite_) {
			pauseSprite_->Draw();
		}
	}

	Sprite::PostDraw();

	if (fade_) {
		fade_->Draw();
	}
}

void GameScene::Finalize() {
	KamataEngine::Audio::GetInstance()->StopWave(voiceHandleBGM_);

	// ３Dモデルの解放
	delete model_;

	model_ = nullptr;
	// 自キャラの解放
	delete player_;

	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTramsformBlock : worldTransformBlockLine) {
			delete worldTramsformBlock;
		}
	}
	worldTransformBlocks_.clear();

	delete debugCamera_;

	delete cameraController_;

	delete modelSkydome_;

	delete mapChipField_;

	delete skydome_;

	delete dethParticles_;

	delete fade_;

	delete modelBlock_;

	delete modelEnemy_;

	delete modeldethParticles_;

	delete modelAttack_;

	delete hitEffectModel_;

	delete chargeParticleModel_;
	chargeParticleModel_ = nullptr;

	for (KamataEngine::Model* model : signModels_) {
		delete model;
	}
	signModels_.clear();

	delete modelWarp_;
	modelWarp_ = nullptr;

	delete pauseSprite_;
	pauseSprite_ = nullptr;

	delete pauseTextSprite_;
	pauseTextSprite_ = nullptr;

	for (auto& platform : movingPlatforms_) {
		for (WorldTransform* block : platform.blocks) {
			delete block;
		}
		platform.blocks.clear();
	}
	movingPlatforms_.clear();

	for (Enemy* enemy : enemies_) {
		delete enemy;
	}

	for (ShieldEnemy* shieldEnemy : shieldEnemies_) {
		delete shieldEnemy;
	}

	for (HitEffect* hitEffect : hitEffects_) {
		delete hitEffect;
	}

	for (GuardEffect* guardEffect : guardEffects_) {
		delete guardEffect;
	}

	for (DeathParticles* deathParticles : enemyDeathParticles_) {
		delete deathParticles;
	}
	enemyDeathParticles_.clear();
}

void GameScene::CheckAllCollisions() {
#pragma region 自キャラと敵キャラの当たり判定
	// 判定対象１と２の座標
	AABB aabb1, aabb2;

	// 自キャラの座標
	aabb1 = player_->GetAABB();

	// 自キャラと敵弾すべての当たり判定
	for (Enemy* enemy : enemies_) {
		if (enemy->Enemy::IsCollisionDisabled())
			continue;

		// 敵弾の座標
		aabb2 = enemy->GetAABB();
		// AABB同士の交差判定
		if (IsCollision(aabb1, aabb2)) {
			// 自キャラの衝突時関数を呼び出す
			player_->OnCollision(enemy);
			// 敵の衝突時関数を呼び出す
			enemy->OnCollision(player_);
		}
	}

	for (ShieldEnemy* shieldEnemy : shieldEnemies_) {
		if (shieldEnemy->ShieldEnemy::IsCollisionDisabled())
			continue;

		// 敵弾の座標
		aabb2 = shieldEnemy->GetAABB();
		// AABB同士の交差判定
		if (IsCollision(aabb1, aabb2)) {
			// 自キャラの衝突時関数を呼び出す
			player_->OnCollision(shieldEnemy);
			// 敵の衝突時関数を呼び出す
			shieldEnemy->OnCollision(player_);
		}
	}

#pragma endregion

#pragma region 自キャラと動く床の当たり判定

	AABB playerAABB = player_->GetAABB();

	for (auto& platform : movingPlatforms_) {

		Vector3 leftPos = platform.basePosition;
		float currentWidth = platform.totalWidth;
		float halfBlockWidth = MapChipField::kBlockWidth / 2.0f;
		float halfBlockHeight = MapChipField::kBlockHeight / 2.0f;

		AABB blockAABB;
		blockAABB.min.x = leftPos.x - halfBlockWidth;
		blockAABB.min.y = leftPos.y - halfBlockHeight;
		blockAABB.min.z = leftPos.z - halfBlockWidth;

		blockAABB.max.x = leftPos.x - halfBlockWidth + currentWidth;
		blockAABB.max.y = leftPos.y + halfBlockHeight;
		blockAABB.max.z = leftPos.z + halfBlockWidth;

		if (IsCollision(playerAABB, blockAABB)) {

			float overlapLeft = playerAABB.max.x - blockAABB.min.x;
			float overlapRight = blockAABB.max.x - playerAABB.min.x;
			float overlapTop = blockAABB.max.y - playerAABB.min.y;
			float overlapBottom = playerAABB.max.y - blockAABB.min.y;

			float minOverlap = (std::min)({overlapLeft, overlapRight, overlapTop, overlapBottom});

			Vector3 playerPos = player_->GetWorldPosition();
			float playerHalfHeight = (playerAABB.max.y - playerAABB.min.y) / 2.0f;
			float playerHalfWidth = (playerAABB.max.x - playerAABB.min.x) / 2.0f;

			if (minOverlap == overlapTop) {
				Vector3 moveAmount = platform.basePosition - platform.previousPos;

				// 2. プレイヤーの位置を「動いた後の床の上面」に固定する
				playerPos.y = blockAABB.max.y + playerHalfHeight;

				// 3. X軸（横方向）の移動量だけを加算する（Y軸は上で固定したため）
				playerPos.x += moveAmount.x;

				player_->SetPosition(playerPos);
				player_->OnGround();
			} else if (minOverlap == overlapBottom) {
				playerPos.y = blockAABB.min.y - playerHalfHeight;
				player_->SetPosition(playerPos);
				player_->OnGround();
			} else if (minOverlap == overlapLeft) {
				playerPos.x = blockAABB.min.x - playerHalfWidth;
				player_->SetPosition(playerPos);
			} else if (minOverlap == overlapRight) {
				playerPos.x = blockAABB.max.x + playerHalfWidth;
				player_->SetPosition(playerPos);
			}
		}
	}

#pragma endregion

#pragma region 自キャラとゴールの当たり判定

	for (GoalInfo& goal : goals_) {
		Vector3 pos = goal.worldTransform->translation_;

		// ゴールの当たり判定範囲（1x1x1ブロック分の場合）
		AABB goalAABB;
		goalAABB.min = {pos.x - MapChipField::kBlockWidth / 2.0f, pos.y - MapChipField::kBlockHeight / 2.0f, pos.z - MapChipField::kBlockWidth / 2.0f};
		goalAABB.max = {pos.x + MapChipField::kBlockWidth / 2.0f, pos.y + MapChipField::kBlockHeight / 2.0f, pos.z + MapChipField::kBlockWidth / 2.0f};

		if (IsCollision(playerAABB, goalAABB)) {
			// ゴール到達時の処理（クリアフラグを立てる、シーン遷移を要請するなど）
			KamataEngine::Audio::GetInstance()->PlayWave(soundHandleGoal_, false, 0.5f);
			isClear_ = true;
			finished_ = true; // 例：ゲーム終了・クリアフラグ
		}
	}

#pragma endregion
}

void GameScene::GenerateObjects() {

	uint32_t numVertical = MapChipField::kNumBlockVirtical;

	uint32_t numHorizontal = MapChipField::kNumBlockHorizontal;

	worldTransformBlocks_.resize(numVertical);

	for (uint32_t i = 0; i < numVertical; ++i) {

		worldTransformBlocks_[i].resize(numHorizontal);

		for (uint32_t j = 0; j < numHorizontal; ++j) {

			WorldTransform* worldTransform = new WorldTransform();
			worldTransform->Initialize();
			worldTransform->translation_ = mapChipField_->GetMapChipPositionByIndex(j, i);

			MapChipType type = mapChipField_->GetMapChipTypeByIndex(j, i);

			switch (type) {
			case MapChipType::kBlock:
				switch (mapChipField_->GetMapChipSubIDByIndex(j, i)) {
				case 0:
					// B0
					worldTransformBlocks_[i][j] = worldTransform;
					break;
				}

				break;

			case MapChipType::kMovingPlatform: {
				// 横に何マス連続しているかカウント
				int blockCount = 0;
				while ((j + blockCount) < numHorizontal && mapChipField_->GetMapChipTypeByIndex(j + blockCount, i) == MapChipType::kMovingPlatform) {
					blockCount++;
				}

				uint8_t subID = mapChipField_->GetMapChipSubIDByIndex(j, i);
				float blockWidth = MapChipField::kBlockWidth;

				MovingPlatformGroup group;
				group.direction = subID;
				group.moveUp = true;
				group.totalWidth = blockWidth * blockCount;

				// 1マスごとに描画用 Transform を生成して配列に入れる
				for (int b = 0; b < blockCount; ++b) {
					WorldTransform* blockTransform = new WorldTransform();
					blockTransform->Initialize();

					// 1マスずつ右にズラした初期座標を設定
					Vector3 pos = mapChipField_->GetMapChipPositionByIndex(j + b, i);
					blockTransform->translation_ = pos;

					group.blocks.push_back(blockTransform);
				}

				// 基準座標（一番左のブロックの座標）
				group.basePosition = group.blocks[0]->translation_;
				group.startX = group.basePosition.x;
				group.startY = group.basePosition.y;
				group.previousPos = group.basePosition;

				movingPlatforms_.push_back(group);

				// 連結分インデックスを進める
				j += (blockCount - 1);
				break;
			}

			case MapChipType::kSpike: {
				WorldTransform* worldTransformSpike = new WorldTransform();
				worldTransformSpike->Initialize();

				worldTransformSpike->translation_ = mapChipField_->GetMapChipPositionByIndex(j, i);

				worldTransformSpikes_.push_back(worldTransformSpike);

				break;
			}
			case MapChipType::kPlayer: {

				Vector3 playerPosition = mapChipField_->GetMapChipPositionByIndex(j, i);

				// 自キャラの生成
				player_ = new Player();
				// 自キャラの初期化
				player_->Initialize(model_, modelAttack_, &camera_, playerPosition);

				player_->SetMapChipField(mapChipField_);

				break;
			}

			case MapChipType::kEnemy: {
				uint8_t subID = mapChipField_->GetMapChipSubIDByIndex(j, i);
				switch (subID) {
				case 0: {
					// 歩行的の生成
					Enemy* newEnemy = new Enemy();

					newEnemy->SetGameScene(this);

					Vector3 enemyPosition = mapChipField_->GetMapChipPositionByIndex(j, i);

					newEnemy->Initialize(modelEnemy_, &camera_, enemyPosition);

					newEnemy->SetMapChipField(mapChipField_);

					enemies_.push_back(newEnemy);
					break;
				}
				case 1: {
					// 盾敵の生成
					ShieldEnemy* newShieldEnemy = new ShieldEnemy();

					newShieldEnemy->SetGameScene(this);

					Vector3 shieldEnemyPosition = mapChipField_->GetMapChipPositionByIndex(j, i);

					newShieldEnemy->Initialize(modelShieldEnemy_, &camera_, shieldEnemyPosition);

					newShieldEnemy->SetMapChipField(mapChipField_);

					shieldEnemies_.push_back(newShieldEnemy);
					break;
				}
				}
				delete worldTransform;
				break;
			}

			case MapChipType::kSign: {
				uint8_t subID = mapChipField_->GetMapChipSubIDByIndex(j, i);

				SignInfo sign;
				sign.worldTransform = std::make_unique<KamataEngine::WorldTransform>();
				sign.worldTransform->Initialize();

				Vector3 pos = mapChipField_->GetMapChipPositionByIndex(j, i);
				pos.z += 1.0f; // ブロックより手前
				sign.worldTransform->translation_ = pos;
				sign.worldTransform->scale_ = {2.0f, 2.0f, 2.0f};
				sign.worldTransform->rotation_.y = -1.5707963f;

				// CSVから取得したsubID（看板の番号）を記録
				sign.subID = subID;

				UpdateWorldTransform(*sign.worldTransform);
				signs_.push_back(std::move(sign));

				delete worldTransform;
				break;
			}

			case MapChipType::kWarp: {
				WarpInfo warp;
				warp.worldTransform = std::make_unique<KamataEngine::WorldTransform>();
				warp.worldTransform->Initialize();

				Vector3 pos = mapChipField_->GetMapChipPositionByIndex(j, i);
				warp.worldTransform->translation_ = pos;
				warp.subID = 0; // 1種類に統一

				warps_.push_back(std::move(warp));

				delete worldTransform;
				break;
			}

			case MapChipType::kGoal: {
				GoalInfo goal;
				goal.worldTransform = std::make_unique<KamataEngine::WorldTransform>();
				goal.worldTransform->Initialize();

				Vector3 pos = mapChipField_->GetMapChipPositionByIndex(j, i);
				goal.worldTransform->translation_ = pos;

				goals_.push_back(std::move(goal));

				delete worldTransform;
				break;
			}

			default:
				delete worldTransform;
				break;
			}
		}
	}
}

void GameScene::CreateHitEffect(const KamataEngine::Vector3& position) {
	HitEffect* newHitEffect = HitEffect::Create(position);
	hitEffects_.push_back(newHitEffect);
}

void GameScene::CreateGuardEffect(const KamataEngine::Vector3& position) {
	GuardEffect* newGuardEffect = GuardEffect::Create(position);
	guardEffects_.push_back(newGuardEffect);
}

void GameScene::CreateDeathParticles(const KamataEngine::Vector3& position) {
	DeathParticles* newDeathParticles = new DeathParticles;
	newDeathParticles->Initialize(modeldethParticles_, &camera_, position);
	enemyDeathParticles_.push_back(newDeathParticles);
}

void GameScene::ChangeStage() {
	stageManager_->NextStage();

	// 1. 既存の動的オブジェクトを破棄・クリア
	for (Enemy* enemy : enemies_) {
		delete enemy;
	}
	enemies_.clear();

	for (ShieldEnemy* shieldEnemy : shieldEnemies_) {
		delete shieldEnemy;
	}
	shieldEnemies_.clear();

	for (HitEffect* hitEffect : hitEffects_) {
		delete hitEffect;
	}
	hitEffects_.clear();

	

	for (GuardEffect* guardEffect : guardEffects_) {
		delete guardEffect;
	}
	guardEffects_.clear();

	// ブロックのメモリ解放
	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			delete worldTransformBlock;
		}
	}
	worldTransformBlocks_.clear();

	for (WorldTransform* moveBlock : worldTransformMoveBlocks_) {
		delete moveBlock;
	}
	worldTransformMoveBlocks_.clear();
	moveBlockWidths_.clear();
	moveBlockDirection_.clear();
	moveBlockStartX_.clear();
	moveBlockStartY_.clear();
	moveBlockMoveUp_.clear();
	moveBlockPreviousPositions_.clear();
	warps_.clear();
	signs_.clear();
	goals_.clear();

	for (WorldTransform* spike : worldTransformSpikes_) {
		delete spike;
	}
	worldTransformSpikes_.clear();

	// プレイヤーの削除（GenerateObjects で再生成するため）
	delete player_;
	player_ = nullptr;

	// 2. 次のステージCSVの読み込み
	const StageData& stageData = stageManager_->GetCurrentStageData();
	std::string stageFilename = "Resources/field/" + stageData.name + ".csv";
	mapChipField_->LoadMapChipCsv(stageFilename);

	// 3. オブジェクトの再生成
	GenerateObjects();

	// 4. カメラのターゲット再設定とリセット
	cameraController_->SetTarget(player_);
	cameraController_->Reset();

	// 5. フェードイン演出を開始（必要であれば）
	if (fade_) {
		fade_->Start(Fade::Status::FadeIn, fadeTime_);
	}
}

void GameScene::ReloadStage() {
	// 1. 既存の動的オブジェクトを破棄・クリア
	for (Enemy* enemy : enemies_) {
		delete enemy;
	}
	enemies_.clear();

	for (ShieldEnemy* shieldEnemy : shieldEnemies_) {
		delete shieldEnemy;
	}
	shieldEnemies_.clear();

	for (HitEffect* hitEffect : hitEffects_) {
		delete hitEffect;
	}
	hitEffects_.clear();

	for (GuardEffect* guardEffect : guardEffects_) {
		delete guardEffect;
	}
	guardEffects_.clear();

	for (std::vector<WorldTransform*>& worldTransformBlockLine : worldTransformBlocks_) {
		for (WorldTransform* worldTransformBlock : worldTransformBlockLine) {
			delete worldTransformBlock;
		}
	}
	worldTransformBlocks_.clear();

	for (auto& platform : movingPlatforms_) {
		for (WorldTransform* block : platform.blocks) {
			delete block;
		}
		platform.blocks.clear();
	}
	movingPlatforms_.clear();

	for (WorldTransform* spike : worldTransformSpikes_) {
		delete spike;
	}
	worldTransformSpikes_.clear();

	warps_.clear();
	signs_.clear();
	goals_.clear();

	if (dethParticles_) {
		delete dethParticles_;
		dethParticles_ = nullptr;
	}

	delete player_;
	player_ = nullptr;

	// 2. 現在のステージCSVを再読み込み
	const StageData& stageData = stageManager_->GetCurrentStageData();
	std::string stageFilename = "Resources/field/" + stageData.name + ".csv";
	mapChipField_->LoadMapChipCsv(stageFilename);

	// 3. オブジェクトの再生成
	GenerateObjects();

	// 4. カメラの再設定
	cameraController_->SetTarget(player_);
	cameraController_->Reset();

	// 5. フラグとフェードのリセット
	phase_ = Phase::kPlay;
	isGameOver_ = false;
	finished_ = false;

	if (fade_) {
		fade_->Start(Fade::Status::FadeIn, fadeTime_);
	}
}