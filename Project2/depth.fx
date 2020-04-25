Texture2D txDiffuse : register(t0);
SamplerState samLinear : register(s0);

cbuffer ConstantBuffer : register(b0)
{
	matrix World;
	matrix WVP;
	float4 blend;
}

cbuffer ConstantBufferPointLight : register (b2)
{
	float4 vPos[100];
	float4 vLightPointColor[100];
}

cbuffer ConstantBufferLight : register(b1)
{
	float4 vLightDir[100];
	float4 vLightColor[100];
	float4 vOutputColor;
	matrix ViewL[10];
	matrix ProjectionL[10];
	Texture2D tex[10];
	SamplerState samp[10];
}

struct VS_INPUT
{
	float4 Pos : POSITION;
	float2 Tex : TEXCOORD0;
	float3 Norm : NORMAL;
};

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float Depth : TEXTURE0;
	
};


PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output;
	input.Pos.w = 1.f;
	output.Pos = mul(input.Pos, WVP);
	output.Depth = output.Pos.z;
	return output;
}

float4 PS(PS_INPUT input) : SV_Target
{
	return input.Depth;
}