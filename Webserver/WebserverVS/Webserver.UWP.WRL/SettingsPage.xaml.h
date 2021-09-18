//
// SettingsPage.xaml.h
// Deklaration der SettingsPage-Klasse
//

#pragma once

#include "SettingsPage.g.h"
#include "MainPage.xaml.h"

namespace Webserver_UWP_WRL
{
	/// <summary>
	/// Eine leere Seite, die eigenständig verwendet oder zu der innerhalb eines Rahmens navigiert werden kann.
	/// </summary>
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class SettingsPage sealed
	{
	private:
		MainPage ^page;
	protected:
		void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs ^e) override;
	public:
		SettingsPage();
	private:
		void Privatekey_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void Certificate_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	};
}
