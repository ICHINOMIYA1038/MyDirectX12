#include "BasicShaderHeader.hlsl"
Output BasicVS(float4 pos : POSITION, float2 uv : TEXCOORD)
{
	Output o;
	o.svpos = pos;
	o.uv = uv;
	return o;
}