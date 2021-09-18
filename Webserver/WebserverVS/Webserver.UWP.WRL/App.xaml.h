//
// App.xaml.h
// Deklaration der App-Klasse
//

#pragma once

#include "App.g.h"
#include "../../Net/TLSSocketListener.h"

namespace Webserver_UWP_WRL
{
	ref class App sealed
	{
	public:
		void UsePrivatekey(Windows::Storage::StorageFile ^ file);
		void UseCertificate(Windows::Storage::StorageFile ^ file);
	protected:
		virtual void OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ e) override;

	internal:
		App();
		void Start_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

	private:
		std::shared_ptr<Net::TLSSocketListener> server;
		void OnSuspending(Platform::Object^ sender, Windows::ApplicationModel::SuspendingEventArgs^ e);
		void OnNavigationFailed(Platform::Object ^sender, Windows::UI::Xaml::Navigation::NavigationFailedEventArgs ^e);
		void OnBackRequested(Platform::Object ^sender, Windows::UI::Core::BackRequestedEventArgs ^args);
	};
}
