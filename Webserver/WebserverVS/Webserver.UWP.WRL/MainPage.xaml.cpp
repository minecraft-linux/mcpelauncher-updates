#include "MainPage.xaml.h"
#include "SettingsPage.xaml.h"
#include "AboutPage.xaml.h"
#include "App.xaml.h"
#include "FileProgress.xaml.h"
#include <ppltasks.h>
#include <process.h>  

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

std::string UTF16ToUTF8(const wchar_t * string, size_t length)
{
	std::vector<char> result(WideCharToMultiByte(CP_UTF8, 0, string, length, 0, 0, 0, 0));
	return std::string(result.data(), WideCharToMultiByte(CP_UTF8, 0, string, length, result.data(), result.size(), 0, 0));
}

MainPage::MainPage()
{
	InitializeComponent();
	StartButton->Click += ref new Windows::UI::Xaml::RoutedEventHandler(dynamic_cast<App^>(Application::Current), &Webserver_UWP_WRL::App::Start_Click);

	_beginthread([](void*) -> void {
		HMODULE lib = LoadPackagedLibrary(L"uftpd.dll", 0);
		{
			void(*SetOnExit)(void(*)(int code)) = (void(*)(void(*)(int)))GetProcAddress(lib, "SetOnExit");
			SetOnExit([](int code) {
				ExitThread(code);
			});
		}
		/*{
			void(*SetOnFileProgress)(void(*OnFileProgress)(int fd, const char * filename, unsigned long long cur_offset, unsigned long long blocks)) = (void(*)(void(*OnFileProgress)(int fd, const char * filename, unsigned long long cur_offset, unsigned long long blocks)))GetProcAddress(lib, "SetOnFileProgress");
			SetOnFileProgress([](int fd, const char * filename, unsigned long long cur_offset, unsigned long long blocks) {
				Windows::ApplicationModel::Core::CoreApplication::MainView->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler([fd,filename,cur_offset,blocks]() {
					auto frame = dynamic_cast<Windows::UI::Xaml::Controls::Frame^>(Window::Current->Content);
					MainPage^ page = dynamic_cast<MainPage^>(frame->Content);
					page->Fileprogress(fd, (intptr_t)filename, cur_offset, blocks);
				}));
			});
		}*/
		int(*client_main)(int argc, char * argv[]) = (int(*)(int, char **))GetProcAddress(lib, "main");
		const char * argv[6];
		argv[0] = "uftpd.exe";
		argv[1] = "-L";
		Windows::Storage::ApplicationData ^appdata = Windows::Storage::ApplicationData::Current;
		String ^tmppath = appdata->TemporaryFolder->Path;
		String ^localpath = appdata->LocalFolder->Path;
		std::string logfile = UTF16ToUTF8(tmppath->Data(), tmppath->Length()) + "\\log.txt";
		std::string download = UTF16ToUTF8(localpath->Data(), localpath->Length());
		argv[2] = logfile.data();
		argv[3] = "-t";
		argv[4] = "-D";
		argv[5] = download.data();
		int r = client_main(6, (char**)argv);
		FreeLibrary(lib);
	}, 0,0);
}

void MainPage::Fileprogress(int fd, intptr_t filename, unsigned long long cur_offset, unsigned long long blocks)
{
	if (Log->Children->Size > 0)
	{
		FileProgress ^last = dynamic_cast<FileProgress^>(Log->Children->GetAt(Log->Children->Size - 1));
		if (last->FileDescriptor == fd)
		{
			last->Progress = (double)cur_offset / (double)blocks;
			return;
		}
	}
	FileProgress ^ prog = ref new FileProgress();
	std::vector<wchar_t> buffer(MultiByteToWideChar(CP_UTF8, 0, (char*)filename, -1, 0, 0));
	MultiByteToWideChar(CP_UTF8, 0, (char*)filename, -1, buffer.data(), buffer.size());
	prog->FileDescriptor = fd;
	prog->FileName = ref new String(buffer.data(), buffer.size());
	Log->Children->Append(prog);
}

void MainPage::Settings_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	dynamic_cast<Windows::UI::Xaml::Controls::Frame^>(Window::Current->Content)->Navigate(Windows::UI::Xaml::Interop::TypeName(SettingsPage::typeid), this);
}

void MainPage::About_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	dynamic_cast<Windows::UI::Xaml::Controls::Frame^>(Window::Current->Content)->Navigate(Windows::UI::Xaml::Interop::TypeName(AboutPage::typeid));
}