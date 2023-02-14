#include "BasicShaderHeader.hlsl"
float4 BasicPS(Output input) : SV_TARGET
{
	float4 result =  float4(tex.Sample(smp,input.uv));
	return result;
}