#include "BasicShaderHeader.hlsli"

Output BasicVS( float4 pos : POSITION, float4 normal : NORMAL,float2 uv : TEXCOORD)
{
	Output output; // ピクセルシェーダーに渡す値
	output.svpos = mul(mat, pos);
	output.svnor = mul(mat, normal);
	output.uv = uv;
	return output;
}