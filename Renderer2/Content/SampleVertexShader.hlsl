// A constant buffer that stores the three basic column-major matrices for composing geometry.
cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
	float4 color;
	float4 eyePos;
	float4 lightPos;
};

// Per-vertex data used as input to the vertex shader.
struct VertexShaderInput
{
	float3 pos : POSITION;
	float3 normal : NORMAL0;
};

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL0;
	float3 light : TEXCOORD0;
	float3 eye : TEXCOORD1;
	float4 color : COLOR0;
};

// Simple shader to do vertex processing on the GPU.
PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;
	float4 pos = float4(input.pos, 1.0f);
	float4 normal = float4(input.normal, 0.0f);

	// Transform the vertex position into projected space.
	pos = mul(pos, model);

	output.light = lightPos.xyz - pos.xyz;
	output.eye = eyePos.xyz - pos.xyz;
	output.normal = mul(input.normal, model).xyz;

	pos = mul(pos, view);
	pos = mul(pos, projection);
	output.pos = pos;

	// Pass the color through without modification.
	output.color = color;

	return output;
}
