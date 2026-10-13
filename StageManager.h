#pragma once
#include <KamataEngine.h>
#include <string>

/// <summary>
/// ステージ管理
/// </summary>

struct StageData {
	std::string name;  // ステージ名
	int32_t timeLimit; // 制限時間
};

class StageManager {
private:
	// 全ステージデータ
	std::vector<StageData> stageDatas_;

public:
	/// <summary>
	/// ステージデータファイルの読み込み
	/// </summary>
	void LoadStageDataFile(const std::string& filePath);

	/// <summary>
	/// ステージデータの取得
	/// <?summary>
	/// <param name="index">ステージ番号</param>
	/// <returns>ステージデータ</returns>
	const StageData& GetStageData(int32_t index) const { return stageDatas_[index]; }

	// 現在のステージ番号
	int32_t currentStageIndex_ = 0;

	void SetCurrentStageIndex(int32_t index) { currentStageIndex_ = index; }

	int32_t GetCurrentStageIndex() const { return currentStageIndex_; }

	///< summary>
	/// 現在ステージのステージデータを取得
	///</summary>
	/// <param name="index">ステージ番号</param>
	/// <returns>ステージデータ</returns>
	const StageData& GetCurrentStageData() const { return GetStageData(currentStageIndex_); }

	/// <summary>
	/// ステージ名指定で現在ステージ番号設定
	/// </summary>
	/// <param name="stageName">ステージ名</param>
	void SetCurrentStageIndexByName(const std::string& name);

	void NextStage() {
		currentStageIndex_++;
		if (currentStageIndex_ >= static_cast<int32_t>(stageDatas_.size())) {
			currentStageIndex_ = 0;
		}
	}
};
