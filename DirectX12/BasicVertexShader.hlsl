#include "BasicShaderHeader.hlsli"

Output BasicVS( float4 pos : POSITION, float4 normal : NORMAL,float2 uv : TEXCOORD)
{
	Output output; // �s�N�Z���V�F�[�_�[�ɓn���l
	output.svpos = mul(mul(viewproj, world), pos);
	normal.w = 0;//�����d�v(���s�ړ������𖳌��ɂ���)
	output.normal = mul(world, normal);
	output.uv = uv;
	return output;
}

Output PlaneVS(float4 pos : POSITION, float4 normal : NORMAL, float2 uv : TEXCOORD)
{
	Output output; // �s�N�Z���V�F�[�_�[�ɓn���l
	output.svpos = mul(mul(planeViewproj, planeWorld), pos);
	normal.w = 0;//�����d�v(���s�ړ������𖳌��ɂ���)
	output.normal = mul(planeWorld, normal);
	output.uv = uv;
	return output;
}

Output shadowVS(float4 pos : POSITION, float4 normal : NORMAL, float2 uv : TEXCOORD)
{
	Output output; // �s�N�Z���V�F�[�_�[�ɓn���l
	output.svpos = mul(mul(lightCamera, world), pos);
	normal.w = 0;//�����d�v(���s�ړ������𖳌��ɂ���)
	output.normal = mul(world, normal);
	output.uv = uv;
	return output;
}