#include "shadowMapHeader.hlsli"

Output BasicVS(float4 pos : POSITION, float4 normal : NORMAL, float2 uv : TEXCOORD)
{
	Output output; // ピクセルシェーダーに渡す値
	output.svpos = mul(mul(viewproj, world), pos);
	normal.w = 0;//ここ重要(平行移動成分を無効にする)
	output.normal = mul(world, normal);
	output.uv = uv;
	return output;
}