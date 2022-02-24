#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <string>

struct ConstantBuffer
{
	DirectX::XMFLOAT4X4 World;
	DirectX::XMFLOAT4X4 View;
	DirectX::XMFLOAT4X4 Projection;
	DirectX::XMFLOAT4	LightVector;
	DirectX::XMFLOAT4   LightColor;
	DirectX::XMFLOAT4	MaterialAmbient;
	DirectX::XMFLOAT4	MaterialDiffuse;
	DirectX::XMFLOAT4	MaterialSpecular;
};

struct Vector3
{
	Vector3()
	{
		this->X = 0.0f;
		this->Y = 0.0f;
		this->Z = 0.0f;
	}

	Vector3(float x, float y, float z)
	{
		this->X = x;
		this->Y = y;
		this->Z = z;
	}

	float X;
	float Y;
	float Z;
};

struct Vector2
{
	Vector2()
	{
		this->X = 0.0f;
		this->Y = 0.0f;
	}

	Vector2(float x, float y)
	{
		this->X = x;
		this->Y = y;
	}

	float X;
	float Y;
};

struct Color
{
	float Red;
	float Green;
	float Blue;
	float Alpha;

	Color(float red, float green, float blue, float alpha)
	{
		Red = red;
		Green = green;
		Blue = blue;
		Alpha = alpha;
	}

	Color()
	{
		Red = Green = Blue = Alpha = 1.0f;
	}

};

//=====================================================================//
//! ポリゴン出力用カスタムバーテックス構造体
//=====================================================================//
struct CustomVertex
{
	Vector3 Position;		// 座標(x, y, z)
	Vector3 Normal;			// 法線
};

struct ObjMaterial
{
	float Ambient[4];
	float Diffuse[4];
	float Specular[4];
};