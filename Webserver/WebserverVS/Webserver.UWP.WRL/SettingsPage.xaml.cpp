#include "SettingsPage.xaml.h"
#include "App.xaml.h"
#include <ppltasks.h>

using namespace Webserver_UWP_WRL;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

SettingsPage::SettingsPage()
{
	InitializeComponent();
}

void SettingsPage::OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs ^e)
{
	Page::OnNavigatedTo(e);
	if (e->Parameter != nullptr)
	{
		page = dynamic_cast<MainPage^>(e->Parameter);
	}
}

void SettingsPage::Privatekey_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	Windows::Storage::Pickers::FileOpenPicker ^fopicker = ref new Windows::Storage::Pickers::FileOpenPicker();
	fopicker->CommitButtonText = L"Use as Privatekey";
	fopicker->FileTypeFilter->Append(L".pem");
	fopicker->SuggestedStartLocation = Windows::Storage::Pickers::PickerLocationId::ComputerFolder;
	Concurrency::create_task(fopicker->PickSingleFileAsync()).then([this, fopicker](Windows::Storage::StorageFile ^file) {
		dynamic_cast<App^>(Application::Current)->UsePrivatekey(file);
	});
}

void SettingsPage::Certificate_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	Windows::Storage::Pickers::FileOpenPicker ^fopicker = ref new Windows::Storage::Pickers::FileOpenPicker();
	fopicker->CommitButtonText = L"Use as Certificate";
	fopicker->FileTypeFilter->Append(L".pem");
	fopicker->SuggestedStartLocation = Windows::Storage::Pickers::PickerLocationId::ComputerFolder;
	Concurrency::create_task(fopicker->PickSingleFileAsync()).then([this, fopicker](Windows::Storage::StorageFile ^file) {
		dynamic_cast<App^>(Application::Current)->UseCertificate(file);
	});
}
