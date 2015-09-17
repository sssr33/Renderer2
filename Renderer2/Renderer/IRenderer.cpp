#include "pch.h"
#include "IRenderer.h"

IRenderer::IRenderer(const std::shared_ptr<DX::DeviceResources> &dx)
	: dx(dx) 
{
	// Register to be notified if the Device is lost or recreated
	this->dx->RegisterDeviceNotify(this);
}

IRenderer::~IRenderer() {
	// Deregister device notification
	this->dx->RegisterDeviceNotify(nullptr);
}

void IRenderer::CreateResources(bool onlySizeDependent) {
	if (!onlySizeDependent) {
		this->CreateDeviceDependentResources();
	}
	
	this->CreateWindowSizeDependentResources();
}

void IRenderer::UpdateMain() {
	// Update scene objects.
	this->timer.Tick([&]() {
		this->Update(this->timer);
	});
}

bool IRenderer::RenderMain() {
	// Don't try to render anything before the first Update.
	if (this->timer.GetFrameCount() == 0) {
		return false;
	}

	auto context = this->dx->GetD3DDeviceContext();

	// Reset the viewport to target the whole screen.
	auto viewport = this->dx->GetScreenViewport();
	context->RSSetViewports(1, &viewport);

	// Reset render targets to the screen.
	ID3D11RenderTargetView *const targets[1] = { this->dx->GetBackBufferRenderTargetView() };
	context->OMSetRenderTargets(1, targets, this->dx->GetDepthStencilView());

	// Clear the back buffer and depth stencil view.
	context->ClearRenderTargetView(this->dx->GetBackBufferRenderTargetView(), DirectX::Colors::CornflowerBlue);
	context->ClearDepthStencilView(this->dx->GetDepthStencilView(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

	this->Render();

	return true;
}

// IDeviceNotify
void IRenderer::OnDeviceLost() {
	this->ReleaseDeviceDependentResources();
}

void IRenderer::OnDeviceRestored() {
	this->CreateResources();
}