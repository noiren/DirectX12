#include "shadowMapHeader.hlsli"

float4 BasicPS(Output input) : SV_TARGET
{
	float3 light = normalize(float3(1,1,1));
	float brightness = dot(-light, input.normal);
	float4 textureColor = float4(tex.Sample(smp, input.uv));
	return float4(mul(brightness, textureColor.x), mul(brightness, textureColor.y), mul(brightness, textureColor.z), 0.f);
}
