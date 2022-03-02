#include "BasicShaderHeader.hlsli"

float4 BasicPS(Output input) : SV_TARGET
{
	float3 lightPower = float3(0.f,0.f,10.0f);
	float3 lightDir = normalize(float3(1, 1, 1));
	float lightRate = dot(input.normal, -lightDir);
	float3 lightColor = lightPower * lightRate;
	float4 textureColor = float4(tex.Sample(smp, input.uv));
	float3 finalColor = lightColor * textureColor;
	return float4(finalColor.x, finalColor.y, finalColor.z, 0.f);
}