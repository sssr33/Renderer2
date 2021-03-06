#include "pch.h"
#include "Renderer.h"

#include <libhelpers\H.h>

Renderer::Renderer(const std::shared_ptr<DX::DeviceResources> &dx)
	: IRenderer(dx), rotationAngle(0)
{
}

Renderer::~Renderer() {

}

void Renderer::CreateDeviceDependentResources() {
	HRESULT hr = S_OK;
	auto d3dDev = this->dx->GetD3DDevice();

	auto vsData = H::System::LoadPackageFile(L"SampleVertexShader.cso");
	auto psData = H::System::LoadPackageFile(L"SamplePixelShader.cso");

	hr = d3dDev->CreateVertexShader(vsData.data(), vsData.size(), nullptr, this->vertexShader.GetAddressOf());
	H::System::ThrowIfFailed(hr);

	hr = d3dDev->CreatePixelShader(psData.data(), psData.size(), nullptr, this->pixelShader.GetAddressOf());
	H::System::ThrowIfFailed(hr);

	static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	hr = d3dDev->CreateInputLayout(vertexDesc, ARRAY_SIZE(vertexDesc), vsData.data(), vsData.size(), this->inputLayout.GetAddressOf());
	H::System::ThrowIfFailed(hr);

	std::vector<DirectX::XMFLOAT3> vertices, normals;
	std::vector<uint16_t> indices;

	this->MakePlane(
		DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f),
		DirectX::XMFLOAT3(0, 0, -1),
		(uint16_t)vertices.size(),
		vertices, normals, indices);

	this->MakePlane(
		DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f),
		DirectX::XMFLOAT3(-1, 0, 0),
		(uint16_t)vertices.size(),
		vertices, normals, indices);

	this->MakePlane(
		DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f),
		DirectX::XMFLOAT3(0, 1, 0),
		(uint16_t)vertices.size(),
		vertices, normals, indices);

	this->MakePlane(
		DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f),
		DirectX::XMFLOAT3(0, 0, 1),
		(uint16_t)vertices.size(),
		vertices, normals, indices);

	this->MakePlane(
		DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f),
		DirectX::XMFLOAT3(1, 0, 0),
		(uint16_t)vertices.size(),
		vertices, normals, indices);

	this->MakePlane(
		DirectX::XMFLOAT3(0.5f, 0.5f, 0.5f),
		DirectX::XMFLOAT3(0, -1, 0),
		(uint16_t)vertices.size(),
		vertices, normals, indices);

	D3D11_BUFFER_DESC bufDesc;
	D3D11_SUBRESOURCE_DATA bufData;

	bufDesc.ByteWidth = (uint32_t)(vertices.size() * sizeof(DirectX::XMFLOAT3));
	bufDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	bufDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
	bufDesc.CPUAccessFlags = 0;
	bufDesc.MiscFlags = 0;
	bufDesc.StructureByteStride = 0;

	bufData.pSysMem = vertices.data();
	bufData.SysMemPitch = bufData.SysMemSlicePitch = 0;

	hr = d3dDev->CreateBuffer(&bufDesc, &bufData, this->vertexBuffer.GetAddressOf());
	H::System::ThrowIfFailed(hr);

	bufDesc.ByteWidth = (uint32_t)(normals.size() * sizeof(DirectX::XMFLOAT3));
	bufData.pSysMem = normals.data();

	hr = d3dDev->CreateBuffer(&bufDesc, &bufData, this->normalBuffer.GetAddressOf());
	H::System::ThrowIfFailed(hr);

	bufDesc.ByteWidth = (uint32_t)(indices.size() * sizeof(uint16_t));
	bufDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER;
	bufData.pSysMem = indices.data();

	hr = d3dDev->CreateBuffer(&bufDesc, &bufData, this->indexBuffer.GetAddressOf());
	H::System::ThrowIfFailed(hr);

	bufDesc.ByteWidth = sizeof(Renderer2::ModelViewProjectionConstantBuffer);
	bufDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	bufDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER;

	hr = d3dDev->CreateBuffer(&bufDesc, nullptr, this->constantBuffer.GetAddressOf());
	H::System::ThrowIfFailed(hr);

	this->indexCount = indices.size();


	D3D11_RASTERIZER_DESC rsDesc;

	rsDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_WIREFRAME;
	rsDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_FRONT;
	rsDesc.FrontCounterClockwise = FALSE;
	rsDesc.DepthBias = 0;
	rsDesc.SlopeScaledDepthBias = 0.0f;
	rsDesc.DepthBiasClamp = 0.0f;
	rsDesc.DepthClipEnable = TRUE;
	rsDesc.ScissorEnable = FALSE;
	rsDesc.MultisampleEnable = FALSE;
	rsDesc.AntialiasedLineEnable = FALSE;

	hr = d3dDev->CreateRasterizerState(&rsDesc, this->rsState.GetAddressOf());
	H::System::ThrowIfFailed(hr);

	this->CreateObjResources();
	this->CreateTexRectResources();
}

void Renderer::ReleaseDeviceDependentResources() {

}

void Renderer::CreateWindowSizeDependentResources() {
	DirectX::XMStoreFloat4(&this->constantBufferData.eyePos, DirectX::XMVectorSet(-2, 2, 0, 1));
	DirectX::XMStoreFloat4(&this->constantBufferData.lightPos, DirectX::XMVectorSet(-1, 2, 0, 1));

	auto view = DirectX::XMMatrixLookAtLH(DirectX::XMLoadFloat4(&this->constantBufferData.eyePos), DirectX::XMVectorSet(0, 0, 2, 1), DirectX::g_XMIdentityR1);
	
	DirectX::XMStoreFloat4x4(&this->constantBufferData.model, DirectX::XMMatrixTranspose(DirectX::XMMatrixTranslation(0, 0, 2)));
	DirectX::XMStoreFloat4x4(&this->constantBufferData.view, DirectX::XMMatrixTranspose(view));
	DirectX::XMStoreFloat4x4(&this->constantBufferData.projection, DirectX::XMMatrixIdentity());

	{
		Windows::Foundation::Size outputSize = this->dx->GetOutputSize();
		float aspectRatio = outputSize.Width / outputSize.Height;
		float fovAngleY = 90.0f * DirectX::XM_PI / 180.0f;

		// This is a simple example of change that can be made when the app is in
		// portrait or snapped view.
		/*if (aspectRatio < 1.0f)
		{
			fovAngleY *= 2.0f;
		}*/

		// Note that the OrientationTransform3D matrix is post-multiplied here
		// in order to correctly orient the scene to match the display orientation.
		// This post-multiplication step is required for any draw calls that are
		// made to the swap chain render target. For draw calls to other targets,
		// this transform should not be applied.

		// This sample makes use of a right-handed coordinate system using row-major matrices.
		DirectX::XMMATRIX perspectiveMatrix = DirectX::XMMatrixPerspectiveFovLH(
			fovAngleY,
			aspectRatio,
			0.01f,
			100.0f
			);

		DirectX::XMFLOAT4X4 orientation = this->dx->GetOrientationTransform3D();

		DirectX::XMMATRIX orientationMatrix = XMLoadFloat4x4(&orientation);

		DirectX::XMStoreFloat4x4(
			&this->constantBufferData.projection,
			XMMatrixTranspose(perspectiveMatrix * orientationMatrix)
			);
	}

	this->CreateLightPrePassTextures();
	this->CreateLightPrePassShaders();
}

void Renderer::Update(DX::StepTimer const& timer) {
	this->rotationAngle += (float)timer.GetElapsedSeconds() * 45.0f;
}

void Renderer::Render() {
	auto context = this->dx->GetD3DDeviceContext();

	DirectX::XMStoreFloat4(&this->constantBufferData.color, DirectX::Colors::White);

	{
		// Each vertex is one instance of the VertexPositionColor struct.
		UINT stride = sizeof(DirectX::XMFLOAT3);
		UINT offset = 0;
		context->IASetVertexBuffers(
			0,
			1,
			this->vertexBuffer.GetAddressOf(),
			&stride,
			&offset
			);

		context->IASetVertexBuffers(
			1,
			1,
			this->normalBuffer.GetAddressOf(),
			&stride,
			&offset
			);
	}

	context->IASetIndexBuffer(
		this->indexBuffer.Get(),
		DXGI_FORMAT_R16_UINT, // Each index is one 16-bit unsigned integer (short).
		0
		);

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->IASetInputLayout(this->inputLayout.Get());

	// Attach our vertex shader.
	context->VSSetShader(
		this->vertexShader.Get(),
		nullptr,
		0
		);

	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers(
		0,
		1,
		this->constantBuffer.GetAddressOf()
		);

	// Attach our pixel shader.
	context->PSSetShader(
		this->pixelShader.Get(),
		nullptr,
		0
		);

	this->DrawObjects();

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rt;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> ds;

	context->OMGetRenderTargets(1, rt.GetAddressOf(), ds.GetAddressOf());
	context->ClearDepthStencilView(ds.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	context->ClearRenderTargetView(this->normalZRtv.Get(), DirectX::Colors::Transparent);
	context->OMSetRenderTargets(1, this->normalZRtv.GetAddressOf(), ds.Get());

	// Attach our vertex shader.
	context->VSSetShader(
		this->normalZVs.Get(),
		nullptr,
		0
		);

	// Attach our pixel shader.
	context->PSSetShader(
		this->normalZPs.Get(),
		nullptr,
		0
		);

	this->DrawObjects();

	context->ClearDepthStencilView(ds.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	context->OMSetRenderTargets(1, rt.GetAddressOf(), ds.Get());

	DirectX::XMStoreFloat4x4(&this->objCBufferData.View, DirectX::XMMatrixIdentity());
	this->objCBufferData.Projection = this->constantBufferData.projection;

	float zscale = 0.5f;//1.0f / 10.0f;

	DirectX::XMMATRIX tmpColor = {
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f,
		zscale, zscale, zscale, zscale
	};

	DirectX::XMStoreFloat4x4(&this->texRectPsCBufferData.ColorMtrx, DirectX::XMMatrixTranspose(tmpColor));
	DirectX::XMStoreFloat4(&this->texRectPsCBufferData.ColorAdd, DirectX::g_XMZero);

	auto tmpSize = this->dx->GetRenderTargetSize();

	DirectX::XMFLOAT3 texRectPos;
	DirectX::XMFLOAT3 texRectSize = DirectX::XMFLOAT3(tmpSize.Width / tmpSize.Height * 0.5f, 0.5f, 1.0f);

	texRectPos.x = -(tmpSize.Width / tmpSize.Height) + texRectSize.x * 0.5f;
	texRectPos.y = 1.0f - texRectSize.y * 0.5f;
	texRectPos.z = 1.0f;

	this->DrawTexRect(
		texRectSize,
		DirectX::XMFLOAT3(0, 0, 0),
		texRectPos,
		this->normalZSrv.Get());

	return;

	//Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rt;
	//Microsoft::WRL::ComPtr<ID3D11DepthStencilView> ds;

	context->OMGetRenderTargets(1, rt.GetAddressOf(), ds.GetAddressOf());
	context->ClearDepthStencilView(ds.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);


	DirectX::XMStoreFloat4(&this->constantBufferData.color, DirectX::Colors::Red);

	Microsoft::WRL::ComPtr<ID3D11RasterizerState> prevRsState;

	context->RSGetState(prevRsState.GetAddressOf());

	context->RSSetState(this->rsState.Get());

	this->DrawObjects();

	context->RSSetState(prevRsState.Get());
}

void Renderer::DrawObjects() {
	DirectX::XMMATRIX baseTransform = DirectX::XMMatrixIdentity();

	this->DrawObject(
		DirectX::XMFLOAT3(4.0f, 0.25f, 4.0f),
		DirectX::XMFLOAT3(0, DirectX::XMConvertToRadians(this->rotationAngle), 0),
		DirectX::XMFLOAT3(0, 0, 2),
		baseTransform);

	this->DrawObject(
		DirectX::XMFLOAT3(1, 1, 1),
		DirectX::XMFLOAT3(0, 0, 0),
		DirectX::XMFLOAT3(0, 0.5f + 0.125f, 1.5f),
		baseTransform);

	this->DrawObject(
		DirectX::XMFLOAT3(0.25f, 0.25f, 0.25f),
		DirectX::XMFLOAT3(0, 0, 0),
		DirectX::XMFLOAT3(0, 0.5f + 0.125f, 0.5f - 0.125f),
		baseTransform);
}

void Renderer::DrawObject(
	DirectX::XMFLOAT3 size,
	DirectX::XMFLOAT3 rot,
	DirectX::XMFLOAT3 pos,
	DirectX::XMMATRIX &baseTransform)
{
	auto ctx = this->dx->GetD3DDeviceContext();
	auto objTransform = DirectX::XMMatrixMultiply(
		DirectX::XMMatrixRotationRollPitchYaw(rot.x, rot.y, rot.z), 
		DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z));

	baseTransform = DirectX::XMMatrixMultiply(objTransform, baseTransform);

	objTransform = DirectX::XMMatrixMultiplyTranspose(DirectX::XMMatrixScaling(size.x, size.y, size.z), baseTransform);
	DirectX::XMStoreFloat4x4(&this->constantBufferData.model, objTransform);

	// Prepare the constant buffer to send it to the graphics device.
	ctx->UpdateSubresource(
		this->constantBuffer.Get(),
		0,
		NULL,
		&this->constantBufferData,
		0,
		0
		);


	// Draw the objects.
	ctx->DrawIndexed(
		this->indexCount,
		0,
		0
		);
}

void Renderer::DrawTexRect(
	DirectX::XMFLOAT3 size,
	DirectX::XMFLOAT3 rot,
	DirectX::XMFLOAT3 pos,
	ID3D11ShaderResourceView *tex) 
{
	auto objTransform = DirectX::XMMatrixMultiply(
		DirectX::XMMatrixRotationRollPitchYaw(rot.x, rot.y, rot.z),
		DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z));

	objTransform = DirectX::XMMatrixMultiplyTranspose(DirectX::XMMatrixScaling(size.x, size.y, size.z), objTransform);

	DirectX::XMStoreFloat4x4(&this->objCBufferData.World, objTransform);

	auto context = this->dx->GetD3DDeviceContext();

	context->UpdateSubresource(
		this->objCBuffer.Get(),
		0,
		NULL,
		&this->objCBufferData,
		0,
		0
		);

	context->UpdateSubresource(
		this->texRectPsCBuffer.Get(),
		0,
		NULL,
		&this->texRectPsCBufferData,
		0,
		0
		);

	{
		// Each vertex is one instance of the VertexPositionColor struct.
		UINT stride = sizeof(DirectX::XMFLOAT2);
		UINT offset = 0;
		context->IASetVertexBuffers(
			0,
			1,
			this->texRectVertexBuf.GetAddressOf(),
			&stride,
			&offset
			);
	}

	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	context->IASetInputLayout(this->texRectInputLayout.Get());

	// Attach our vertex shader.
	context->VSSetShader(
		this->texRectVs.Get(),
		nullptr,
		0
		);

	// Send the constant buffer to the graphics device.
	context->VSSetConstantBuffers(
		0,
		1,
		this->objCBuffer.GetAddressOf()
		);

	// Attach our pixel shader.
	context->PSSetShader(
		this->texRectPs.Get(),
		nullptr,
		0
		);

	// Send the constant buffer to the graphics device.
	context->PSSetConstantBuffers(
		0,
		1,
		this->texRectPsCBuffer.GetAddressOf()
		);

	context->PSSetShaderResources(0, 1, &tex);

	context->Draw(4, 0);
}

void Renderer::MakePlane(
	DirectX::XMFLOAT3 size,
	DirectX::XMFLOAT3 vector,
	uint16_t baseIndex,
	std::vector<DirectX::XMFLOAT3> &vertices,
	std::vector<DirectX::XMFLOAT3> &normals,
	std::vector<uint16_t> &indices)
{
	DirectX::XMVECTOR normal = DirectX::XMLoadFloat3(&vector);

	normal = DirectX::XMVector3Normalize(normal);

	//auto dp = DirectX::XMVector3Dot(normal, DirectX::g_XMIdentityR2); // 0 - normal = +-Y
	//auto equ = DirectX::XMVectorEqual(dp, DirectX::g_XMZero);

	auto v1 = DirectX::g_XMIdentityR1; // always Y because want to get perpendicular in XZ plane
	DirectX::XMVECTOR v2;// = DirectX::XMVectorSelect(normal, DirectX::g_XMIdentityR2, equ); // if normal is Y then switch it to Z

	if (std::fabsf(normal.YF) == 1) {
		v2 = DirectX::XMVectorScale(DirectX::g_XMIdentityR2, -1);
	}
	else {
		v2 = normal;
	}

	/*auto vx = DirectX::XMVector3Cross(v1, v2);
	auto vy = DirectX::XMVector3Cross(v2, vx);*/

	auto vx = DirectX::XMVector3Cross(v1, v2);
	auto vy = DirectX::XMVector3Cross(normal, vx);

	vx = DirectX::XMVector3Normalize(vx);
	vy = DirectX::XMVector3Normalize(vy);

	vx = DirectX::XMVectorScale(vx, size.x);
	vy = DirectX::XMVectorScale(vy, size.y);
	DirectX::XMVECTOR center = DirectX::XMVectorScale(normal, size.z);

	DirectX::XMVECTOR nvx = DirectX::XMVectorScale(vx, -1.0f);
	DirectX::XMVECTOR nvy = DirectX::XMVectorScale(vy, -1.0f);

	auto lt = DirectX::XMVectorAdd(center, vx);
	auto lb = DirectX::XMVectorAdd(center, vx);

	auto rt = DirectX::XMVectorAdd(center, nvx);
	auto rb = DirectX::XMVectorAdd(center, nvx);

	lt = DirectX::XMVectorAdd(lt, vy);
	lb = DirectX::XMVectorAdd(lb, nvy);

	rt = DirectX::XMVectorAdd(rt, vy);
	rb = DirectX::XMVectorAdd(rb, nvy);

	DirectX::XMFLOAT3 tmpN;
	DirectX::XMFLOAT3 tmpLt, tmpLB, tmpRt, tmpRb;

	DirectX::XMStoreFloat3(&tmpN, normal);
	DirectX::XMStoreFloat3(&tmpLt, lt);
	DirectX::XMStoreFloat3(&tmpLB, lb);
	DirectX::XMStoreFloat3(&tmpRt, rt);
	DirectX::XMStoreFloat3(&tmpRb, rb);

	vertices.push_back(tmpLB);
	vertices.push_back(tmpLt);
	vertices.push_back(tmpRt);
	vertices.push_back(tmpRb);

	normals.push_back(tmpN);
	normals.push_back(tmpN);
	normals.push_back(tmpN);
	normals.push_back(tmpN);

	indices.push_back(0 + baseIndex);
	indices.push_back(1 + baseIndex);
	indices.push_back(2 + baseIndex);

	indices.push_back(0 + baseIndex);
	indices.push_back(2 + baseIndex);
	indices.push_back(3 + baseIndex);
}

void Renderer::CreateLightPrePassTextures() {
	HRESULT hr = S_OK;
	D3D11_TEXTURE2D_DESC texDesc;
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
	auto d3dDev = this->dx->GetD3DDevice();
	auto size = this->dx->GetRenderTargetSize();

	texDesc.Width = (uint32_t)size.Width;
	texDesc.Height = (uint32_t)size.Height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R16G16B16A16_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = 
		D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE | 
		D3D11_BIND_FLAG::D3D11_BIND_RENDER_TARGET;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	hr = d3dDev->CreateTexture2D(&texDesc, nullptr, this->normalZ.ReleaseAndGetAddressOf());
	H::System::ThrowIfFailed(hr);

	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION::D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;

	hr = d3dDev->CreateShaderResourceView(this->normalZ.Get(), &srvDesc, this->normalZSrv.ReleaseAndGetAddressOf());
	H::System::ThrowIfFailed(hr);

	rtvDesc.Format = texDesc.Format;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION::D3D11_RTV_DIMENSION_TEXTURE2D;
	rtvDesc.Texture2D.MipSlice = 0;

	hr = d3dDev->CreateRenderTargetView(this->normalZ.Get(), &rtvDesc, this->normalZRtv.ReleaseAndGetAddressOf());
	H::System::ThrowIfFailed(hr);

	//texDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;

	hr = d3dDev->CreateTexture2D(&texDesc, nullptr, this->lighting.ReleaseAndGetAddressOf());
	H::System::ThrowIfFailed(hr);

	srvDesc.Format = texDesc.Format;

	hr = d3dDev->CreateShaderResourceView(this->lighting.Get(), &srvDesc, this->lightingSrv.ReleaseAndGetAddressOf());
	H::System::ThrowIfFailed(hr);

	rtvDesc.Format = texDesc.Format;

	hr = d3dDev->CreateRenderTargetView(this->lighting.Get(), &rtvDesc, this->lightingRtv.ReleaseAndGetAddressOf());
	H::System::ThrowIfFailed(hr);
}

void Renderer::CreateLightPrePassShaders() {
	HRESULT hr = S_OK;
	auto d3dDev = this->dx->GetD3DDevice();

	auto vsData = H::System::LoadPackageFile(L"vs_NormalZ.cso");
	auto psData = H::System::LoadPackageFile(L"ps_NormalZ.cso");

	hr = d3dDev->CreateVertexShader(vsData.data(), vsData.size(), nullptr, this->normalZVs.GetAddressOf());
	H::System::ThrowIfFailed(hr);

	hr = d3dDev->CreatePixelShader(psData.data(), psData.size(), nullptr, this->normalZPs.GetAddressOf());
	H::System::ThrowIfFailed(hr);
}

void Renderer::CreateObjResources() {
	HRESULT hr = S_OK;
	auto d3dDev = this->dx->GetD3DDevice();
	D3D11_BUFFER_DESC bufDesc;

	bufDesc.ByteWidth = sizeof(ObjectCBuffer);
	bufDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	bufDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER;
	bufDesc.CPUAccessFlags = 0;
	bufDesc.MiscFlags = 0;
	bufDesc.StructureByteStride = 0;

	hr = d3dDev->CreateBuffer(&bufDesc, nullptr, this->objCBuffer.GetAddressOf());
	H::System::ThrowIfFailed(hr);

	/*D3D11_SAMPLER_DESC samplerDesc;

	hr = d3dDev->CreateSamplerState(&samplerDesc, this->linearSampler.GetAddressOf());
	H::System::ThrowIfFailed(hr);*/
}

void Renderer::CreateTexRectResources() {
	HRESULT hr = S_OK;
	auto d3dDev = this->dx->GetD3DDevice();
	D3D11_BUFFER_DESC bufDesc;
	D3D11_SUBRESOURCE_DATA subResData;

	bufDesc.ByteWidth = sizeof(PsTexRectCbuffer);
	bufDesc.Usage = D3D11_USAGE::D3D11_USAGE_DEFAULT;
	bufDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER;
	bufDesc.CPUAccessFlags = 0;
	bufDesc.MiscFlags = 0;
	bufDesc.StructureByteStride = 0;

	hr = d3dDev->CreateBuffer(&bufDesc, nullptr, this->texRectPsCBuffer.GetAddressOf());
	H::System::ThrowIfFailed(hr);

	const DirectX::XMFLOAT2 quadStrip[] = {
		DirectX::XMFLOAT2(-0.5f, 0.5f),
		DirectX::XMFLOAT2(0.5f, 0.5f),
		DirectX::XMFLOAT2(-0.5f, -0.5f),
		DirectX::XMFLOAT2(0.5f, -0.5f),
	};

	bufDesc.ByteWidth = sizeof(quadStrip);
	bufDesc.Usage = D3D11_USAGE::D3D11_USAGE_IMMUTABLE;
	bufDesc.BindFlags = D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;

	subResData.pSysMem = quadStrip;
	subResData.SysMemPitch = subResData.SysMemSlicePitch = 0;

	hr = d3dDev->CreateBuffer(&bufDesc, &subResData, this->texRectVertexBuf.GetAddressOf());
	H::System::ThrowIfFailed(hr);


	auto vsData = H::System::LoadPackageFile(L"vs_TexRect.cso");
	auto psData = H::System::LoadPackageFile(L"ps_TexRect.cso");

	hr = d3dDev->CreateVertexShader(vsData.data(), vsData.size(), nullptr, this->texRectVs.GetAddressOf());
	H::System::ThrowIfFailed(hr);

	hr = d3dDev->CreatePixelShader(psData.data(), psData.size(), nullptr, this->texRectPs.GetAddressOf());
	H::System::ThrowIfFailed(hr);

	static const D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	hr = d3dDev->CreateInputLayout(vertexDesc, ARRAY_SIZE(vertexDesc), vsData.data(), vsData.size(), this->texRectInputLayout.GetAddressOf());
	H::System::ThrowIfFailed(hr);
}