Texture2D txDiffuse : register(t0);        
SamplerState samLinear : register(s0);


Texture2D testTex1 : register(t1);
SamplerState testSamp1 : register(s1);

cbuffer ConstantBuffer : register( b0 )
{
	matrix World;
	matrix WVP;
	float4 blend;
}

cbuffer ConstantBufferPointLight : register ( b2 )
{
	float4 vPos[100];
	float4 vLightPointColor[100];
}

cbuffer ConstantBufferLight : register( b1 )
{
	float4 vLightDir[100];
	float4 vLightColor[100];
	float4 vOutputColor;
	matrix VWPL[10];
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
	float2 Tex : TEXCOORD0;
	float3 Norm : TEXCOORD1;
	float4 lightViewPosition : TEXCOORD2;

};


PS_INPUT VS(VS_INPUT input) 
{
	PS_INPUT output = (PS_INPUT)0;
	input.Pos.w = 1.f;
	output.lightViewPosition = input.Pos;
	output.Pos = mul( input.Pos, WVP );
	output.Norm = mul( input.Norm, World);
	output.Tex = input.Tex;
	return output;
}

