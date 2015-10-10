cbuffer Cbuffer0 : register(b0) {
	matrix World;
	matrix View;
	matrix Proj;
};

struct VsInput {
	float2 pos : POSITION;
};

struct PsInput {
	float4 pos : SV_POSITION;
	float2 tex : TEXCOORD0;
};

PsInput main(VsInput input) {
	PsInput output;
	
	output.pos = float4(input.pos, 0.0f, 1.0f);
	output.tex = input.pos;

	output.pos = mul(output.pos, World);
	output.pos = mul(output.pos, View);
	output.pos = mul(output.pos, Proj);

	output.tex *= float2(1.0f, -1.0f);
	output.tex += float2(0.5f, 0.5f);

	return output;
}