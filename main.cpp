#include "ClearScene.h"
#include "GameOverScene.h"
#include "GameScene.h"
#include "KamataEngine.h"
#include "StageManager.h"
#include "TitleScene.h"
#include <fstream>
#include <map>
#include <sstream>

GameScene* gameScene = nullptr;
TitleScene* titleScene = nullptr;
ClearScene* clearScene = nullptr;
GameOverScene* gameOverScene = nullptr;
StageManager* stageManager = nullptr;

// シーン（型）
enum class Scene {

	kUnknown = 0,

	ktitle,
	kGame,
	kClear,
	kGameOver,
};

// 現在のシーン（型）
Scene scene = Scene::kUnknown;

// シーン切り替え処理関数
void ChangeScene() {
	switch (scene) {
	case Scene::ktitle:
		if (titleScene->IsFinished()) {
			// シーン変更
			scene = Scene::kGame;
			titleScene->Finalize();
			delete titleScene;
			titleScene = nullptr;

			stageManager->SetCurrentStageIndex(0);

			// 新シーンの生成と初期化
			gameScene = new GameScene;
			gameScene->Initialize(stageManager);
		}
		break;
	case Scene::kGame:
		if (gameScene->IsTitleRequested()) {
			// ゲームシーン終了処理
			gameScene->Finalize();
			delete gameScene;
			gameScene = nullptr;

			// タイトルシーン生成・遷移
			scene = Scene::ktitle;
			titleScene = new TitleScene;
			titleScene->Initialize();
		}
		// 2. クリア・ゲームオーバー等のゲーム終了チェック
		else if (gameScene->IsFinished()) {
			bool isClear = gameScene->IsClear();
			bool isGameOver = gameScene->IsGameOver();

			// ゲームシーン終了処理
			gameScene->Finalize();
			delete gameScene;
			gameScene = nullptr;

			if (isClear) {
				// クリアシーンへ遷移
				scene = Scene::kClear;
				clearScene = new ClearScene;
				clearScene->Initialize();
			} else if (isGameOver) {
				// ゲームオーバーシーンへ遷移
				scene = Scene::kGameOver;
				gameOverScene = new GameOverScene;
				gameOverScene->Initialize();
			} else {
				// デフォルトはタイトルに戻る
				scene = Scene::ktitle;
				titleScene = new TitleScene;
				titleScene->Initialize();
			}
		}
		// 3. リロード要求のチェック
		else if (gameScene->GetReloadRequested()) {
			// シーンリロード
			gameScene->Finalize();
			delete gameScene;
			gameScene = nullptr;
			gameScene = new GameScene;
			gameScene->Initialize(stageManager);
		}
		break;

	case Scene::kClear:
		if (clearScene->IsFinished()) {
			// クリアシーン終了処理
			clearScene->Finalize();
			delete clearScene;
			clearScene = nullptr;

			// タイトルへ遷移
			scene = Scene::ktitle;
			titleScene = new TitleScene;
			titleScene->Initialize();
		}
		break;

	case Scene::kGameOver:
		if (gameOverScene->IsFinished()) {
			// GameOverScene からの遷移先判定を取得
			GameOverScene::NextScene next = gameOverScene->GetNextScene();

			// ゲームオーバーシーン終了処理
			gameOverScene->Finalize();
			delete gameOverScene;
			gameOverScene = nullptr;

			if (next == GameOverScene::NextScene::kRetry) {
				// ★ リトライ：直接 GameScene を再生成してゲーム画面へ
				scene = Scene::kGame;
				gameScene = new GameScene;
				gameScene->Initialize(stageManager);
			} else {
				// ★ タイトルへ戻る（デフォルト含む）
				scene = Scene::ktitle;
				titleScene = new TitleScene;
				titleScene->Initialize();
			}
		}
		break;
	}
}

// シーン更新
void UpdateScene() {
	switch (scene) {
	case Scene::ktitle:
		titleScene->Update();
		break;
	case Scene::kGame:
		gameScene->Update();
		break;
	case Scene::kClear:
		if (clearScene)
			clearScene->Update(); // 追加！
		break;
	case Scene::kGameOver:
		if (gameOverScene)
			gameOverScene->Update(); // 追加！
		break;
	}
}

// シーン描画
void DrawScene() {
	switch (scene) {
	case Scene::ktitle:
		titleScene->Draw();
		break;
	case Scene::kGame:
		gameScene->Draw();
		break;
	case Scene::kClear:
		if (clearScene)
			clearScene->Draw(); // 追加！
		break;
	case Scene::kGameOver:
		if (gameOverScene)
			gameOverScene->Draw(); // 追加！
		break;
	}
}

void LoadDebugSettings() {
	std::ifstream file("Resources/DebugSettings.ini");
	if (!file.is_open()) {
		// ファイルが見つからない場合は処理しない（デフォルトのまま）
		return;
	}

	std::string line;
	while (std::getline(file, line)) {

		if (line.empty() || line[0] == '#' || line[0] == ';') {
			continue;
		}

		std::istringstream lineStream(line);
		std::string key;
		std::string value;

		if (std::getline(lineStream, key, '=') && std::getline(lineStream, value)) {
			// ステージ設定
			if (key == "InitialStage") {
				stageManager->SetCurrentStageIndexByName(value);
			}
		}
	}

	file.close();
}

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {

	// エンジン初期化
	KamataEngine::Initialize(L"LE2C_04_オオダテ_ソラ_青い冒険者");
	KamataEngine::DebugText::GetInstance()->Initialize();

	using namespace KamataEngine;
	/// DirectXCommonインスタンスの取得
	DirectXCommon* dxCommon = DirectXCommon::GetInstance();

	// クラスインスタンスの生成
	stageManager = new StageManager;
	// ステージデータファイルを読み込む
	stageManager->LoadStageDataFile("Resources/stageDatas.csv");
#ifdef _DEBUG

	LoadDebugSettings();

	// ゲームシーンから直接開始
	scene = Scene::kGame;
	// ゲームシーンのインスタンス生成
	gameScene = new GameScene();
	// ゲームシーンの初期化
	gameScene->Initialize(stageManager);

	// タイトルシーン
	titleScene = nullptr;

#else

	// 最初のシーンの初期化
	scene = Scene::ktitle;
	// タイトルシーンのインスタンス生成
	titleScene = new TitleScene;
	// タイトルシーンの初期化
	titleScene->Initialize();

	gameScene = nullptr;

#endif

	// メインループ
	while (true) {
		// エンジンの更新
		if (KamataEngine::Update()) {
			break;
		}

		KamataEngine::ImGuiManager::GetInstance()->Begin();

		// 現在シーン更新
		UpdateScene();
		// シーンの切り替え
		ChangeScene();

		KamataEngine::ImGuiManager::GetInstance()->End();

		// 描画開始
		dxCommon->PreDraw();
		KamataEngine::DebugText::GetInstance()->DrawAll();

		// 現在シーンの描画
		DrawScene();
		KamataEngine::ImGuiManager::GetInstance()->Draw();

		// 描画終了
		dxCommon->PostDraw();
	}

	// ゲームシーンの処理
	delete gameScene;

	// タイトルシーンの処理
	delete titleScene;

	// クラスインスタンスの解放
	delete stageManager;

	// nullptrの代入
	gameScene = nullptr;

	// エンジンの終了処理
	KamataEngine::Finalize();

	return 0;
}
