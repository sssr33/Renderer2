#pragma once
#include "IRenderer.h"
#include "CBuffer\ObjectCBuffer.h"
#include "CBuffer\PsTexRectCbuffer.h"
#include "..\Content\ShaderStructures.h"

#include <vector>

class Renderer : public IRenderer {
public:
	Renderer(const std::shared_ptr<DX::DeviceResources> &dx);
	virtual ~Renderer();

	virtual void CreateDeviceDependentResources() override;
	virtual void ReleaseDeviceDependentResources() override;

	virtual void CreateWindowSizeDependentResources() override;

	virtual void Update(DX::StepTimer const& timer) override;
	virtual void Render() override;

private:

	Microsoft::WRL::ComPtr<ID3D11InputLayout>	inputLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		normalBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		indexBuffer;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>	vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	pixelShader;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		constantBuffer;

	Microsoft::WRL::ComPtr<ID3D11RasterizerState> rsState;

	Renderer2::ModelViewProjectionConstantBuffer	constantBufferData;
	uint32	indexCount;
	float rotationAngle;

	// obj resources
	ObjectCBuffer objCBufferData;
	Microsoft::WRL::ComPtr<ID3D11Buffer> objCBuffer;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> linearSampler;

	// texRect
	PsTexRectCbuffer texRectPsCBufferData;
	Microsoft::WRL::ComPtr<ID3D11Buffer> texRectPsCBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> texRectVertexBuf;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> texRectInputLayout;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> texRectVs;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> texRectPs;

	// light pre-pass
	Microsoft::WRL::ComPtr<ID3D11Texture2D> normalZ;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> normalZSrv;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> normalZRtv;

	Microsoft::WRL::ComPtr<ID3D11Texture2D> lighting;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> lightingSrv;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> lightingRtv;

	Microsoft::WRL::ComPtr<ID3D11VertexShader> normalZVs;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> normalZPs;

	void DrawObjects();

	void DrawObject(
		DirectX::XMFLOAT3 size,
		DirectX::XMFLOAT3 rot,
		DirectX::XMFLOAT3 pos, 
		DirectX::XMMATRIX &baseTransform);

	void DrawTexRect(
		DirectX::XMFLOAT3 size,
		DirectX::XMFLOAT3 rot,
		DirectX::XMFLOAT3 pos,
		ID3D11ShaderResourceView *tex);

	void MakePlane(
		DirectX::XMFLOAT3 size,
		DirectX::XMFLOAT3 vector,
		uint16_t baseIndex,
		std::vector<DirectX::XMFLOAT3> &vertices, 
		std::vector<DirectX::XMFLOAT3> &normals, 
		std::vector<uint16_t> &indices);

	void CreateLightPrePassTextures();
	void CreateLightPrePassShaders();

	void CreateObjResources();
	void CreateTexRectResources();
};