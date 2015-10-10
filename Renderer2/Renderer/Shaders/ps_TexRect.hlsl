cbuffer Cbuffer0 : register(b0) {
	matrix ColorMtrx;
	float4 ColorAdd;
};

Texture2D tex : register(t0);
SamplerState texSampler : register(s0);

struct PsInput {
	float4 pos : SV_POSITION;
	float2 tex : TEXCOORD0;
};

float4 main(PsInput input) : SV_TARGET {
	float4 color = tex.Sample(texSampler, input.tex);

	color = mul(color, ColorMtrx);
	color += ColorAdd;

	return color;
}