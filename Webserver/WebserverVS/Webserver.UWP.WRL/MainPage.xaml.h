#pragma once

#include "MainPage.g.h"
#include "../../Net/TLSSocketListener.h"

namespace Webserver_UWP_WRL
{
	public ref class MainPage sealed
	{
	public:
		MainPage();
		void Fileprogress(int fd, intptr_t filename, unsigned long long cur_offset, unsigned long long blocks);
	private:
		void Settings_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void About_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
	};
}
