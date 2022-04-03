#include "BasicShaderHeader.hlsli"

float4 BasicPS(Output input) : SV_TARGET
{
	float3 light = normalize(float3(1,-1,1));
	float brightness = dot(-light, input.normal);
	float4 textureColor = float4(tex.Sample(smp, input.uv));
	return float4(mul(brightness, textureColor.x), mul(brightness, textureColor.y), mul(brightness, textureColor.z), 0.f);
}

float4 PlanePS(Output input) : SV_TARGET
{
	// ÇøÇÂÇ¡Ç∆îZÇ¢ÇﬂÇ…ÇµÇƒèoóÕ
	float Cubedep = lightDepthTex.Sample(smp,input.uv);
	float LightDep = lightPeraDepthTex.Sample(smp, input.uv);

	float dep = 1.f;
	if (Cubedep < LightDep)
	{
		dep = 0.0f;
	}
	return float4(dep, dep, dep, 1);
}


//float3 lightPower = float3(0.f, 0.f, 10.0f);
//float3 lightDir = normalize(float3(1, 1, 1));
//float lightRate = dot(input.normal, -lightDir);
//float3 lightColor = lightPower * lightRate;
//float4 textureColor = float4(tex.Sample(smp, input.uv));
//float4 finalColor = float4(mul(lightColor.x, textureColor.x), mul(lightColor.y, textureColor.y), mul(lightColor.z, textureColor.z), 0.f);