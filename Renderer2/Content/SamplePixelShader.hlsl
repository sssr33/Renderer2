// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL0;
	float3 light : TEXCOORD0;
	float3 eye : TEXCOORD1;
	float4 color : COLOR0;
};

// A pass-through function for the (interpolated) color data.
float4 main(PixelShaderInput input) : SV_TARGET
{
	const float specPower = 30.0;

float3 n2 = normalize(input.normal);
float3 l2 = normalize(input.light);
float3 v2 = normalize(input.eye);
float3 r = reflect(-v2, n2);
float4 diff = input.color * max(dot(n2, l2), 0.0);
float4 spec = input.color * pow(max(dot(l2, r), 0.0), specPower);

return diff + spec;

	//return float4(input.normal.rgb, 1.0f);

	//return float4(input.color, 1.0f);
}
