// 頂点シェーダーからピクセルシェーダーへのやり取りに使用する構造体

struct Output {
	float4 svpos : SV_POSITION; // system用頂点座標
	float4 normal : NORMAL;		// system用法線座標
	float2 uv : TEXCOORD;		// uv値
};

Texture2D<float4> tex : register(t0); // 0番スロットに設定されたテクスチャ
Texture2D<float> depthTex : register(t1); // 深度テクスチャ
Texture2D<float> lightDepthTex : register(t2); // ライト深度テクスチャ
SamplerState smp : register(s0);		  // 0番スロットに設定されたサンプラー

cbuffer cbuff0 : register(b0) // 0番スロットに設定された定数
{
	matrix world; // ワールド変換行列
	matrix viewproj; // ワールド変換行列
	matrix lightCamera; // ライトビュープロジェクション
}

cbuffer cbuff1 : register(b1) // 1番スロットに設定された定数
{
	matrix planeWorld; // ワールド変換行列
	matrix planeViewproj; // ワールド変換行列
}