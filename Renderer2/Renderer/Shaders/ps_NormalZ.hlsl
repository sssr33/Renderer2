
struct PsInput {
	float4 pos : SV_POSITION;
	float4 normalZ : TEXCOORD0;
};

float4 main(PsInput input) : SV_TARGET {
	return input.normalZ;
}