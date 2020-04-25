Texture2D txDiffuse : register(t0);        
SamplerState samLinear : register(s0);

cbuffer ConstantBuffer : register( b0 )
{
	matrix World;
	matrix View;
	matrix Projection;
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
	float4 PosX : POSITION;
};


PS_INPUT VS(VS_INPUT input) 
{
	PS_INPUT output = (PS_INPUT)0;
	output.Pos = mul( input.Pos, World );
	output.PosX = output.Pos;
	output.Pos = mul( output.Pos, View);
	output.Pos = mul( output.Pos, Projection);
	output.Norm = mul( input.Norm, World);
	output.Tex = input.Tex;
	return output;
}

float4 PS(PS_INPUT input) : SV_Target
{
	float4 finalcolor = 0.1f;
	if ((int)vOutputColor.x == 0 && (int)vOutputColor.y == 0) finalcolor = 1.f;
	else
	{
		for (int i = 0; i < (int)vOutputColor.x; i++)
		{
			finalcolor += saturate(dot((float3)vLightDir[i], input.Norm) * vLightDir[i].a * vLightColor[i]);
		}



		for (int i = 0; i < (int)vOutputColor.y; i++) {
			float R = vPos[i].a;
			float3 vLightPointDir = (float3)vPos[i] - (float3)input.PosX;
			float k = length(vLightPointDir);
			if (k < R) {
				k = 1 - k / R;
				finalcolor += saturate(dot(vLightPointDir, input.Norm) * k * vLightPointColor[i]);
			}
		}
	}

	finalcolor *= txDiffuse.Sample(samLinear, input.Tex);
	finalcolor *= blend;
	finalcolor.a = blend.a;
	return finalcolor;
}

float4 PSfL(PS_INPUT input) : SV_Target
{
	return float4(1.f, 1.f, 1.f, 1.f);
}