#pragma once
#include "IRenderer.h"

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
};