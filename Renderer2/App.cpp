#include "pch.h"
#include "App.h"
#include "Renderer\Renderer.h"

#include <ppltasks.h>

using namespace Renderer2;

using namespace concurrency;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Core;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;

// The main function is only used to initialize our IFrameworkView class.
[Platform::MTAThread]
int main(Platform::Array<Platform::String^>^)
{
	auto direct3DApplicationSource = ref new Direct3DApplicationSource();
	CoreApplication::Run(direct3DApplicationSource);
	return 0;
}

IFrameworkView^ Direct3DApplicationSource::CreateView()
{
	return ref new App();
}

App::App() :
	m_windowClosed(false)
{
}

// The first method called when the IFrameworkView is being created.
void App::Initialize(CoreApplicationView^ applicationView)
{
	// Register event handlers for app lifecycle. This example includes Activated, so that we
	// can make the CoreWindow active and start rendering on the window.
	applicationView->Activated +=
		ref new TypedEventHandler<CoreApplicationView^, IActivatedEventArgs^>(this, &App::OnActivated);

	CoreApplication::Suspending +=
		ref new EventHandler<SuspendingEventArgs^>(this, &App::OnSuspending);

	CoreApplication::Resuming +=
		ref new EventHandler<Platform::Object^>(this, &App::OnResuming);

	// At this point we have access to the device. 
	// We can create the device-dependent resources.
	m_deviceResources = std::make_shared<DX::DeviceResources>();
}

// Called when the CoreWindow object is created (or re-created).
void App::SetWindow(CoreWindow^ window)
{
	window->SizeChanged +=
		ref new TypedEventHandler<CoreWindow^, WindowSizeChangedEventArgs^>(this, &App::OnWindowSizeChanged);

	window->VisibilityChanged +=
		ref new TypedEventHandler<CoreWindow^, VisibilityChangedEventArgs^>(this, &App::OnVisibilityChanged);

	window->Closed +=
		ref new TypedEventHandler<CoreWindow^, CoreWindowEventArgs^>(this, &App::OnWindowClosed);

	DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();

	currentDisplayInformation->DpiChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &App::OnDpiChanged);

	currentDisplayInformation->OrientationChanged +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &App::OnOrientationChanged);

	DisplayInformation::DisplayContentsInvalidated +=
		ref new TypedEventHandler<DisplayInformation^, Object^>(this, &App::OnDisplayContentsInvalidated);

	m_deviceResources->SetWindow(window);
}

// Initializes scene resources, or loads a previously saved app state.
void App::Load(Platform::String^ entryPoint)
{
	if (!this->renderer) {
		this->renderer = std::unique_ptr<IRenderer>(new Renderer(this->m_deviceResources));
		this->renderer->CreateResources();
	}
}

// This method is called after the window becomes active.
void App::Run()
{
	while (!m_windowClosed) {
		CoreWindow::GetForCurrentThread()->Dispatcher->ProcessEvents(CoreProcessEventsOption::ProcessOneAndAllPending);
	}
}

// Required for IFrameworkView.
// Terminate events do not cause Uninitialize to be called. It will be called if your IFrameworkView
// class is torn down while the app is in the foreground.
void App::Uninitialize()
{
	this->StopRenderThread();
}

// Application lifecycle event handlers.

void App::OnActivated(CoreApplicationView^ applicationView, IActivatedEventArgs^ args)
{
	// Run() won't start until the CoreWindow is activated.
	CoreWindow::GetForCurrentThread()->Activate();
}

void App::OnSuspending(Platform::Object^ sender, SuspendingEventArgs^ args)
{
	// Save app state asynchronously after requesting a deferral. Holding a deferral
	// indicates that the application is busy performing suspending operations. Be
	// aware that a deferral may not be held indefinitely. After about five seconds,
	// the app will be forced to exit.
	SuspendingDeferral^ deferral = args->SuspendingOperation->GetDeferral();

	this->StopRenderThread();

	create_task([this, deferral]()
	{
		concurrency::critical_section::scoped_lock lk(this->cs);
		m_deviceResources->Trim();

		// Insert your code here.

		deferral->Complete();
	});
}

void App::OnResuming(Platform::Object^ sender, Platform::Object^ args)
{
	// Restore any data or state that was unloaded on suspend. By default, data
	// and state are persisted when resuming from suspend. Note that this event
	// does not occur if the app was previously terminated.

	// Insert your code here.
}

// Window event handlers.

void App::OnWindowSizeChanged(CoreWindow^ sender, WindowSizeChangedEventArgs^ args)
{
	concurrency::critical_section::scoped_lock lk(this->cs);
	m_deviceResources->SetLogicalSize(Size(sender->Bounds.Width, sender->Bounds.Height));
	this->renderer->CreateResources(true);
}

void App::OnVisibilityChanged(CoreWindow^ sender, VisibilityChangedEventArgs^ args)
{
	if (args->Visible) {
		this->StartRenderThread();
	}
	else {
		this->StopRenderThread();
	}
}

void App::OnWindowClosed(CoreWindow^ sender, CoreWindowEventArgs^ args)
{
	m_windowClosed = true;
}

// DisplayInformation event handlers.

void App::OnDpiChanged(DisplayInformation^ sender, Object^ args)
{
	concurrency::critical_section::scoped_lock lk(this->cs);
	m_deviceResources->SetDpi(sender->LogicalDpi);
	this->renderer->CreateResources(true);
}

void App::OnOrientationChanged(DisplayInformation^ sender, Object^ args)
{
	concurrency::critical_section::scoped_lock lk(this->cs);
	m_deviceResources->SetCurrentOrientation(sender->CurrentOrientation);
	this->renderer->CreateResources(true);
}

void App::OnDisplayContentsInvalidated(DisplayInformation^ sender, Object^ args)
{
	concurrency::critical_section::scoped_lock lk(this->cs);
	m_deviceResources->ValidateDevice();
}

void App::StartRenderThread() {
	if (this->renderTask && this->renderTask->Status != Windows::Foundation::AsyncStatus::Canceled) {
		return;
	}

	Windows::System::Threading::WorkItemHandler ^task =
		ref new Windows::System::Threading::WorkItemHandler(
			[=](Windows::Foundation::IAsyncAction ^op)
	{
		while (op->Status != Windows::Foundation::AsyncStatus::Canceled && !m_windowClosed) {
			concurrency::critical_section::scoped_lock lk(this->cs);
			this->renderer->UpdateMain();

			if (this->renderer->RenderMain()) {
				m_deviceResources->Present();
			}
		}
	});

	this->renderTask = Windows::System::Threading::ThreadPool::RunAsync(
		task,
		Windows::System::Threading::WorkItemPriority::High,
		Windows::System::Threading::WorkItemOptions::TimeSliced);
}

void App::StopRenderThread() {
	if (this->renderTask && this->renderTask->Status != Windows::Foundation::AsyncStatus::Canceled) {
		this->renderTask->Cancel();
	}
}