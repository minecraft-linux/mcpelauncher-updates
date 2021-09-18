#include <Http/HttpServer.h>
#include <Http/Http2.h>
#include <experimental/filesystem>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <condition_variable>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

std::condition_variable controlc;

void handle_ctrlc_signal(int sig)
{
	controlc.notify_all();
	return;
}

#ifdef _WIN32
BOOL windows_ctrl_handler(DWORD fdwCtrlType)
{
	switch (fdwCtrlType)
	{
	case CTRL_C_EVENT:
		handle_ctrlc_signal(0);
		return TRUE;
	case CTRL_BREAK_EVENT:
	case CTRL_CLOSE_EVENT:
	case CTRL_LOGOFF_EVENT:
	case CTRL_SHUTDOWN_EVENT:
		break;
	}
	return FALSE;
}
#endif

namespace fs = std::experimental::filesystem;

int main(int argc, const char** argv)
{
#ifdef _WIN32
	auto _cicp = GetConsoleCP(), _cocp = GetConsoleOutputCP();
	SetConsoleCP(CP_UTF8);
	SetConsoleOutputCP(CP_UTF8);
#endif
#ifndef _WIN32
	signal(SIGINT, handle_ctrlc_signal);
#else
	SetConsoleCtrlHandler((PHANDLER_ROUTINE)windows_ctrl_handler, TRUE);
#endif
	try
	{
		int cmd = 0;
		fs::path executablefolder = fs::canonical(fs::path(*argv++).parent_path()), privatekey = executablefolder / "privkey.pem", publiccertificate = executablefolder / "publicchain.pem", webroot = executablefolder;
		--argc;
		while (argc > 0)
		{
			if (strcmp(*argv, "-help") == 0)
			{
				++argv;
				--argc;
				cmd = 0;
			}
			else if (strcmp(*argv, "-h1") == 0)
			{
				++argv;
				--argc;
				cmd = 1;
			}
			else if (strcmp(*argv, "-h2") == 0)
			{	
				++argv;
				--argc;
				cmd = 2;
			}
			else if (strcmp(*argv, "-prikey") == 0)
			{
				++argv;
				privatekey = *argv++;
				argc -= 2;
			}
			else if (strcmp(*argv, "-pubcert") == 0)
			{
				++argv;
				publiccertificate = *argv++;
				argc -= 2;
			}
			else if (strcmp(*argv, "-webroot") == 0)
			{
				++argv;
				webroot = *argv++;
				argc -= 2;
			}
			else
			{
				throw std::runtime_error("Unbekanntes Argument \"" + std::string(*argv) + "\"");
			}
		}
		switch (cmd)
		{
		case 1:
		{
			std::mutex wait;
			std::unique_lock<std::mutex> lock(wait);
			Http::Server server(privatekey, publiccertificate, webroot);
			std::cout << "Webserver gestartet\n";
			controlc.wait(lock);
			std::cout << "Webserver beendet\n";
			break;
		}
		case 2:
		{
			std::mutex wait;
			std::unique_lock<std::mutex> lock(wait);
			Http2::Server server(privatekey, publiccertificate, webroot);
			std::cout << "Webserver gestartet\n";
			controlc.wait(lock);
			std::cout << "Webserver beendet\n";
			break;
		}
		default:
			std::cout << "Benutzung vom Server\n";
			std::cout << "-help Zeige diese Hilfe\n";
			std::cout << "-h1 HTTP/1.1 Server\n";
			std::cout << "-h2 HTTP/2.0 Server\n";
			std::cout << "-prikey <pfad> privater Schlüssel\n";
			std::cout << "-pubcert <pfad> öffentliches Zertifikat\n";
			std::cout << "-webroot <pfad> Webroot\n";
			break;
		}
	}
	catch (std::exception &ex) {
		std::cout << "Fehler:" << ex.what() << "\r\n";
	}
#ifdef _WIN32
	SetConsoleCP(_cicp);
	SetConsoleOutputCP(_cocp);
#endif
	return 0;
}