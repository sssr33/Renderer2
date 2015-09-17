#include "pch.h"
#include "Renderer.h"

Renderer::Renderer(const std::shared_ptr<DX::DeviceResources> &dx)
	: IRenderer(dx) 
{
}

Renderer::~Renderer() {

}

void Renderer::CreateDeviceDependentResources() {

}

void Renderer::ReleaseDeviceDependentResources() {

}

void Renderer::CreateWindowSizeDependentResources() {

}

void Renderer::Update(DX::StepTimer const& timer) {

}

void Renderer::Render() {

}