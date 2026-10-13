#pragma once
#include "KamataEngine.h"

/// <summary>
/// スケール、回転、平行移動からアフィン変換行列を計算する
/// </summary>
KamataEngine::Matrix4x4 MakeAffineMatrix(KamataEngine::Vector3 scale, KamataEngine::Vector3 rotation, KamataEngine::Vector3 translation);

/// <summary>
/// 行列を計算,転送する
/// </summary>
void UpdateWorldTransform(KamataEngine::WorldTransform& worldTransform);

// X軸回転行列の作成関数の宣言
KamataEngine::Matrix4x4 MakeRotateXMatrix(float angle);

// 行列の乗算関数の宣言
KamataEngine::Matrix4x4 MultiplyMatrix(const KamataEngine::Matrix4x4& m1, const KamataEngine::Matrix4x4& m2);