#include "BasicShaderHeader.hlsli"

Output BasicVS( float4 pos : POSITION, float4 normal : NORMAL,float2 uv : TEXCOORD)
{
	Output output; // �s�N�Z���V�F�[�_�[�ɓn���l
	output.svpos = mul(mat, pos);
	output.svnor = mul(mat, normal);
	output.uv = uv;
	return output;
}