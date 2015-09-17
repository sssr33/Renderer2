#pragma once
#include "..\Common\StepTimer.h"
#include "..\Common\DeviceResources.h"

#include <memory>

class IRenderer : private DX::IDeviceNotify {
public:
	IRenderer(const std::shared_ptr<DX::DeviceResources> &dx);
	virtual ~IRenderer();

	void CreateResources(bool onlySizeDependent = false);
	void UpdateMain();
	bool RenderMain();

protected:

	std::shared_ptr<DX::DeviceResources> dx;

	virtual void CreateDeviceDependentResources() = 0;
	virtual void ReleaseDeviceDependentResources() = 0;

	virtual void CreateWindowSizeDependentResources() = 0;

	virtual void Update(DX::StepTimer const& timer) = 0;
	virtual void Render() = 0;
private:
	DX::StepTimer timer;

	// IDeviceNotify
	virtual void OnDeviceLost() override;
	virtual void OnDeviceRestored() override;
};