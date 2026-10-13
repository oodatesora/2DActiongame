#include "StageManager.h"
#include <fstream>
#include <map>
#include <sstream>

// ステージデータファイルのパス
const std::string filePath = "Resources/stageDatas.csv";

// ifStreamでステージデータファイルを開く
void StageManager::LoadStageDataFile(const std::string& filePathStage) {

	// フィルを開く
	std::ifstream file;
	file.open(filePathStage);

	// ファイルの内容を格納するstringstreamの宣言
	std::stringstream stageDataCsv;

	// ファイルの内容をstringstreamにコピー
	stageDataCsv << file.rdbuf();

	// ファイルを閉じる
	file.close();

	// ステージデータを最終行まで１行ずつ読み込む
	// 1行分の内容を格納するstringStreamを宣言して,stringから変換
	std::string line;

	while (std::getline(stageDataCsv, line)) {
		// １行を解析するためのストリーム
		std::stringstream LineStream(line);

		std::string name;
		while (std::getline(LineStream, name, ',')) {
			// ステージデータを格納する構造体
			StageData stageData;
			// カンマ区切りの一つ分を格納するstringの宣言
			std::string timeLimitStr;
			// カンマ区切りで次のデータを取得する
			std::getline(LineStream, timeLimitStr, ',');
			// ステージ名を格納する
			stageData.name = name;

			// カンマ区切りで次のデータを取得する
			std::string stageNumberStr;
			// 整数に変換して制限時間を格納する
			stageData.timeLimit = std::stoi(timeLimitStr);

			// ステージデータテーブルに格納する
			stageDatas_.push_back(stageData);
		}
	}
}

void StageManager::SetCurrentStageIndexByName(const std::string& name) {
	// 全ステージデータを検索
	for (size_t i = 0; i < stageDatas_.size(); ++i) {
		// ステージ名が一致したら現在のステージ番号を設定する
		if (stageDatas_[i].name == name) {
			currentStageIndex_ = static_cast<int32_t>(i);
			// 目的を達したので関数を抜ける
			return;
		}
	}
}