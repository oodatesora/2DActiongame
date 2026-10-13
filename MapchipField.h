#pragma once
#include "KamataEngine.h"
#include <vector>

enum class MapChipType {
	kBlank,          // 空白
	kBlock,          // ブロック
	kPlayer,         // プレイヤー
	kEnemy,          // 敵
	kMovingPlatform, // 動く床
	kSpike,          // トゲ
	kEnemyOnlyWall,  // 敵専用の透明壁
	kWarp,           // ステージ移動用ポータル
	kSign,           // 看板
	kGoal,           // ゴール
};

struct MapChipDataUnit {
	MapChipType type; // マップチップの種別
	uint8_t subID;    // 種類五とのサブID
};

struct MapChipData {
	std::vector<std::vector<MapChipDataUnit>> data;
};

/// <summary>
/// マップチップフィールド
/// </summary>
class MapChipField {
public:
	// 初期化
	void ResetMapChipData();

	void LoadMapChipCsv(const std::string& filePath);

	MapChipType GetMapChipTypeByIndex(uint32_t xIndex, uint32_t yIndex);

	KamataEngine::Vector3 GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex);

	// 1ブロックのサイズ
	static inline const float kBlockWidth = 1.0f;
	static inline const float kBlockHeight = 1.0f;

	// ブロックの個数
	static inline const uint32_t kNumBlockVirtical = 20;
	static inline const uint32_t kNumBlockHorizontal = 100;

	struct IndexSet {
		uint32_t xIndex;
		uint32_t yIndex;
	};

	IndexSet GetMapChipIndexSetByPosition(const KamataEngine::Vector3& position);

	// 範囲矩形
	struct Rect {
		float left;
		float right;
		float bottom;
		float top;
	};

	Rect GetRectByIndex(uint32_t xIndex, uint32_t yIndex);

	uint8_t GetMapChipSubIDByIndex(uint32_t xIndex, uint32_t yIndex);

private:
	MapChipData mapChipData_;

	// マップチップCSVの文字番号
	enum MapChipCharIndex {
		kChipType = 0, // マップチップタイプ
		kChipSubID = 1 // タイプ五とのサブID
	};
};
