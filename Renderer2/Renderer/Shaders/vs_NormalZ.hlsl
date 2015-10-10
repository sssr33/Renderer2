cbuffer Cbuffer0 : register(b0) {
	matrix World;
	matrix View;
	matrix Proj;
};

struct VsInput {
	float3 pos : POSITION;
	float3 normal : NORMAL0;
};

struct PsInput {
	float4 pos : SV_POSITION;
	float4 normalZ : TEXCOORD0;
};

PsInput main(VsInput input) {
	PsInput output;
	float4 pos = float4(input.pos, 1.0f);
	float4 normal = float4(input.normal, 0.0f);

	float4 posView = mul(pos, World);
	float4 normalView = mul(normal, World);

	posView = mul(posView, View);
	normalView = mul(normalView, View);

	output.pos = mul(posView, Proj);
	output.normalZ = float4(normalView.xyz, output.pos.z / output.pos.w);

	return output;
}