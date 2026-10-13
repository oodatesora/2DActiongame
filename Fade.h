#pragma once
#include <KamataEngine.h>

/// <summary>
/// フェード
/// </summary>
class Fade {
public:
	void Initialize();

	void Update();

	void Draw();

	enum class Status {
		Nome,    // フェードなし
		FadeIn,  // フェードイン中
		FadeOut, // フェードアウト抽
	};

	// 現在のフェード状態
	Status status_ = Status::Nome;

	void Start(Status status, float duration);

	bool IsFinished() const;

private:
	uint32_t textureHandle_ = 0;
	KamataEngine::Sprite* sprite_ = nullptr;

	// フェードの継続時間
	float duration_ = 0.0f;

	// 経過時間カンター
	float counter_ = 0.0f;
};
