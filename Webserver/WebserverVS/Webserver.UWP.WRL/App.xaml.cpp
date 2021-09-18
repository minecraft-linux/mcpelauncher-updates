#include "App.xaml.h"
#include "MainPage.xaml.h"
#include <ppltasks.h>
#include <robuffer.h>
#include <wrl.h>
#include <unordered_map>
#include <fstream>
#include <openssl/err.h>
#include <../../../Http/ErrorCode.h>
#include <../../../Http/Connection.h>
#include <../../../Http/Stream.h>
#include <../../../Http/Frame.h>
#include <../../../Http/Setting.h>
#include <../../../Http/HPackDecoder.h>
#include <../../../Http/HPackEncoder.h>
#include <experimental/filesystem>

using namespace Webserver_UWP_WRL;

using namespace Platform;
using namespace Windows::ApplicationModel;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Interop;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

using namespace std::experimental::filesystem;

void RequestHandler(std::shared_ptr<Net::Http::Connection> connection)
{
	auto * request = &connection->request;
	auto * response = &connection->response;
	HPack::Encoder encoder;
	std::vector<uint8_t> buffer;
	if (request->method == "GET")
	{
		if (request->path == "/websocket.html")
		{
			auto & socket = connection->socket;
			response->status = 200;
			response->headerlist.push_back({ "content-length", "23" });
			connection->SendResponse();
			connection->SendData((uint8_t*)"Http/1-2 Server Running", 23, true);
		}
		else
		{
			path filepath = L"C:\\Users\\Christopher\\Documents\\Webserver\\Website" / request->path;
			if (is_regular_file(filepath))
			{
				uintmax_t filesize = file_size(filepath);
				response->status = 200;
				response->headerlist.push_back({ "content-length", std::to_string(filesize) });
				connection->SendResponse();
				{
					std::vector<uint8_t> buffer(10240);
					std::ifstream filestream(filepath, std::ios::binary);
					for (uintmax_t i = filesize; i > 0;)
					{
						int count = std::min((uintmax_t)buffer.size(), i);
						filestream.read((char*)buffer.data(), count);
						connection->SendData(buffer.data(), count, count == i);
						i -= count;
					}
				}
			}
		}
	}
	else
	{
		if (request->path == "/upload")
		{
			connection->SetOnData([connection](const uint8_t * buffer, uint32_t length) {
				//std::cout << "-------------------------------\n" << std::string((char*)buffer, length) << "-------------------------------\n";
				connection->request.length -= length;
				if (connection->request.length == 0)
				{
					connection->response.status = 200;
					connection->response.headerlist.push_back({ "content-length", "0" });
					connection->SendResponse();
				}
			});
		}
	}
}

App::App()
{
    InitializeComponent();
    Suspending += ref new SuspendingEventHandler(this, &App::OnSuspending);
	server = std::make_shared<Net::TLSSocketListener>();
	server->SetConnectionHandler([this](std::shared_ptr<Net::Socket> socket) {
		std::vector<uint8_t> buffer(65535);
		if (socket->GetProtocol() == "h2")
		{
			try
			{
				HPack::Decoder decoder;
				std::shared_ptr<HPack::Encoder> encoder = std::make_shared<HPack::Encoder>();
				std::unordered_map<Setting, uint32_t> settings = {
					{ Setting::HEADER_TABLE_SIZE, 4096 },
					{ Setting::ENABLE_PUSH, 1 },
					{ Setting::MAX_CONCURRENT_STREAMS, std::numeric_limits<uint32_t>::max() },
					{ Setting::INITIAL_WINDOW_SIZE, 65535 },
					{ Setting::MAX_FRAME_SIZE, 16384 },
					{ Setting::MAX_HEADER_LIST_SIZE, std::numeric_limits<uint32_t>::max() },
				};
				std::unordered_map<uint32_t, std::shared_ptr<Net::Http::V2::Stream>> streams;
				socket->ReceiveAll(buffer.data(), 24);
				while (true)
				{
					socket->ReceiveAll(buffer.data(), 9);
					Net::Http::V2::Frame frame = Net::Http::V2::Frame::Parse(buffer.data());
					if (frame.length > buffer.size())
					{
						throw std::runtime_error("Buffer Overflow");
					}
					if (!streams[frame.streamidentifier])
						streams[frame.streamidentifier] = std::make_shared<Net::Http::V2::Stream>(frame.streamidentifier);
					socket->ReceiveAll(buffer.data(), frame.length);
					auto pos = buffer.data(), end = pos + frame.length;
					uint8_t padlength = 0;
					if (frame.HasFlag(Net::Http::V2::Frame::Flag::PADDED))
					{
						padlength = *pos++;
						if (padlength > frame.length)
							throw ErrorCode::PROTOCOL_ERROR;
						end -= padlength;
					}
					switch (frame.type)
					{
					case Net::Http::V2::Frame::Type::DATA:
					{
						if (frame.streamidentifier == 0)
							throw ErrorCode::PROTOCOL_ERROR;
						if (!(streams[frame.streamidentifier]->state == Net::Http::V2::Stream::State::open || streams[frame.streamidentifier]->state == Net::Http::V2::Stream::State::half_closed_local))
							throw ErrorCode::STREAM_CLOSED;
						if ((end - pos) > 0)
							streams[frame.streamidentifier]->OnData(pos, end - pos);
						break;
					}
					case Net::Http::V2::Frame::Type::HEADERS:
					{
						if (frame.streamidentifier == 0)
							throw ErrorCode::PROTOCOL_ERROR;
						if (frame.HasFlag(Net::Http::V2::Frame::Flag::END_STREAM))
						{
							streams[frame.streamidentifier]->state = Net::Http::V2::Stream::State::half_closed_remote;
						}
						else
						{
							streams[frame.streamidentifier]->state = Net::Http::V2::Stream::State::open;
						}
						if (frame.HasFlag(Net::Http::V2::Frame::Flag::PRIORITY))
						{
							streams[frame.streamidentifier]->priority.exclusive = *pos & 0x80;
							streams[frame.streamidentifier]->priority.dependency = GetUInt31(&*pos);
							pos += 4;
							streams[frame.streamidentifier]->priority.weight = *pos++;
						}
						std::shared_ptr<Net::Http::V2::Connection> connection = std::make_shared<Net::Http::V2::Connection>();
						connection->request.AppendHttp2(decoder, pos, end - pos);
						connection->encoder = encoder;
						connection->frame = frame;
						connection->socket = socket;
						connection->stream = streams[frame.streamidentifier];
						if (frame.HasFlag(Net::Http::V2::Frame::Flag::END_HEADERS))
						{
							RequestHandler(connection);
						}
						else
						{
							streams[frame.streamidentifier]->SetOnContinuation([connection, &decoder](Net::Http::V2::Frame & frame, const uint8_t * buffer, uint32_t length) {
								connection->request.AppendHttp2(decoder, buffer, length);
								if (frame.HasFlag(Net::Http::V2::Frame::Flag::END_HEADERS))
								{
									RequestHandler(connection);
								}
							});
						}
						break;
					}
					case Net::Http::V2::Frame::Type::PRIORITY:
					{
						streams[frame.streamidentifier]->priority.exclusive = *pos & 0x80;
						streams[frame.streamidentifier]->priority.dependency = GetUInt31(&*pos);
						pos += 4;
						streams[frame.streamidentifier]->priority.weight = *pos++;
						break;
					}
					case Net::Http::V2::Frame::Type::RST_STREAM:
					{
						ErrorCode code;
						std::reverse_copy(pos, pos + 4, (unsigned char*)&code);
						pos += 4;
						streams[frame.streamidentifier]->state = Net::Http::V2::Stream::State::closed;
						//Abord Work
						break;
					}
					case Net::Http::V2::Frame::Type::SETTINGS:
					{
						if (!frame.HasFlag(Net::Http::V2::Frame::Flag::ACK))
						{
							while (pos != end)
							{
								Setting id = (Setting)ntohs(*(uint16_t*)&*pos);
								pos += 2;
								uint32_t value = ntohl(*(uint32_t*)&*pos);
								pos += 4;
								settings[id] = value;
							}
							{
								Net::Http::V2::Frame response;
								response.length = 0;
								response.type = Net::Http::V2::Frame::Type::SETTINGS;
								response.flags = Net::Http::V2::Frame::Flag::ACK;
								response.streamidentifier = frame.streamidentifier;
								socket->SendAll(response.ToArray());
							}
						}
						break;
					}
					case Net::Http::V2::Frame::Type::PING:
					{
						if (frame.flags != Net::Http::V2::Frame::Flag::ACK)
						{
							Net::Http::V2::Frame response;
							response.length = frame.length;
							response.type = Net::Http::V2::Frame::Type::PING;
							response.flags = Net::Http::V2::Frame::Flag::ACK;
							response.streamidentifier = frame.streamidentifier;
							socket->SendAll(response.ToArray());
							socket->SendAll(buffer.data(), response.length);
						}
						break;
					}
					case Net::Http::V2::Frame::Type::GOAWAY:
					{
						uint32_t laststreamid = ntohl(*(uint32_t*)&*pos);
						pos += 4;
						ErrorCode code = (ErrorCode)ntohl(*(uint32_t*)&*pos);
						pos += 4;
						{
							Net::Http::V2::Frame response;
							response.length = 8;
							response.type = Net::Http::V2::Frame::Type::GOAWAY;
							response.flags = (Net::Http::V2::Frame::Flag)0;
							response.streamidentifier = frame.streamidentifier;
							socket->SendAll(response.ToArray());
							auto wpos = buffer.begin();
							(*(uint32_t*)&*wpos) = htonl(laststreamid);
							wpos += 4;
							(*(uint32_t*)&*wpos) = htonl((uint32_t)ErrorCode::NO_ERROR);
							socket->SendAll(buffer.data(), response.length);
						}
						throw std::runtime_error("GOAWAY");
					}
					case Net::Http::V2::Frame::Type::WINDOW_UPDATE:
					{
						uint32_t WindowSizeIncrement = ntohl(((*(uint32_t*)&*pos) << 1) & 0xfffffffe);
						buffer.resize(buffer.size() + WindowSizeIncrement);
						break;
					}
					case Net::Http::V2::Frame::Type::CONTINUATION:
						streams[frame.streamidentifier]->OnContinuation(frame, pos, end - pos);
						break;
					}
				}
			}
			catch (const std::runtime_error & error)
			{

			}
		}
		else
		{
			int content = 0;
			std::shared_ptr<Net::Http::V1::Connection> connection;
			while (true)
			{
				int count = socket->Receive(buffer.data(), content == 0 ? buffer.size() : std::min(content, (int)buffer.size()));
				if (count <= 0)
				{
					int error = GetLastError();
					break;
				}
				else
				{
					if (content <= 0)
					{
						connection = std::make_shared<Net::Http::V1::Connection>();
						connection->request = Request::ParseHttp1(buffer.data(), count);
						connection->socket = socket;
						content = connection->request.length;
						RequestHandler(connection);
					}
					else
					{
						content -= count;
						connection->OnData(buffer.data(), count);
					}
				}
			}
		}
	});
}

/// <summary>
/// Wird aufgerufen, wenn die Anwendung durch den Endbenutzer normal gestartet wird. Weitere Einstiegspunkte
/// werden z. B. verwendet, wenn die Anwendung gestartet wird, um eine bestimmte Datei zu öffnen.
/// </summary>
/// <param name="e">Details über Startanforderung und -prozess.</param>
void App::OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ e)
{
	auto currentView = Windows::UI::Core::SystemNavigationManager::GetForCurrentView();
	currentView->AppViewBackButtonVisibility = Windows::UI::Core::AppViewBackButtonVisibility::Visible;
	currentView->BackRequested += ref new Windows::Foundation::EventHandler<Windows::UI::Core::BackRequestedEventArgs ^>(this, &Webserver_UWP_WRL::App::OnBackRequested);

    auto rootFrame = dynamic_cast<Frame^>(Window::Current->Content);

    // App-Initialisierung nicht wiederholen, wenn das Fenster bereits Inhalte enthält.
    // Nur sicherstellen, dass das Fenster aktiv ist.
    if (rootFrame == nullptr)
    {
        // Frame erstellen, der als Navigationskontext fungiert, und ihn mit
        // einem SuspensionManager-Schlüssel verknüpfen
        rootFrame = ref new Frame();

        rootFrame->NavigationFailed += ref new Windows::UI::Xaml::Navigation::NavigationFailedEventHandler(this, &App::OnNavigationFailed);

        if (e->PreviousExecutionState == ApplicationExecutionState::Terminated)
        {
            // TODO: Den gespeicherten Sitzungszustand nur bei Bedarf wiederherstellen. Dabei die
            // abschließenden Schritte zum Start planen, nachdem die Wiederherstellung abgeschlossen ist

        }

        if (e->PrelaunchActivated == false)
        {
            if (rootFrame->Content == nullptr)
            {
                // Wenn der Navigationsstapel nicht wiederhergestellt wird, zur ersten Seite navigieren
                // und die neue Seite konfigurieren, indem die erforderlichen Informationen als Navigationsparameter
                // übergeben werden
                rootFrame->Navigate(TypeName(MainPage::typeid), e->Arguments);
            }
            // Den Frame im aktuellen Fenster platzieren
            Window::Current->Content = rootFrame;
            // Sicherstellen, dass das aktuelle Fenster aktiv ist
            Window::Current->Activate();
        }
    }
    else
    {
        if (e->PrelaunchActivated == false)
        {
            if (rootFrame->Content == nullptr)
            {
                // Wenn der Navigationsstapel nicht wiederhergestellt wird, zur ersten Seite navigieren
                // und die neue Seite konfigurieren, indem die erforderlichen Informationen als Navigationsparameter
                // übergeben werden
                rootFrame->Navigate(TypeName(MainPage::typeid), e->Arguments);
            }
            // Sicherstellen, dass das aktuelle Fenster aktiv ist
            Window::Current->Activate();
        }
    }
}

/// <summary>
/// Wird aufgerufen, wenn die Ausführung der Anwendung angehalten wird.  Der Anwendungszustand wird gespeichert,
/// ohne zu wissen, ob die Anwendung beendet oder fortgesetzt wird und die Speicherinhalte dabei
/// unbeschädigt bleiben.
/// </summary>
/// <param name="sender">Die Quelle der Anhalteanforderung.</param>
/// <param name="e">Details zur Anhalteanforderung.</param>
void App::OnSuspending(Object^ sender, SuspendingEventArgs^ e)
{
    (void) sender;  // Nicht verwendeter Parameter
    (void) e;   // Nicht verwendeter Parameter

    //TODO: Anwendungszustand speichern und alle Hintergrundaktivitäten beenden
}

/// <summary>
/// Wird aufgerufen, wenn die Navigation auf eine bestimmte Seite fehlschlägt
/// </summary>
/// <param name="sender">Der Rahmen, bei dem die Navigation fehlgeschlagen ist</param>
/// <param name="e">Details über den Navigationsfehler</param>
void App::OnNavigationFailed(Platform::Object ^sender, Windows::UI::Xaml::Navigation::NavigationFailedEventArgs ^e)
{
    throw ref new FailureException("Failed to load Page " + e->SourcePageType.Name);
}

void App::OnBackRequested(Platform::Object ^sender, Windows::UI::Core::BackRequestedEventArgs ^args)
{
	if (!args->Handled)
	{
		auto rootFrame = dynamic_cast<Frame^>(Window::Current->Content);
		if (rootFrame->CanGoBack)
		{
			rootFrame->GoBack();
			args->Handled = true;
		}
	}
}

uint8_t * GetDataPointer(Windows::Storage::Streams::IBuffer^ buffer)
{
	Microsoft::WRL::ComPtr<Windows::Storage::Streams::IBufferByteAccess> bufferByteAccess;
	reinterpret_cast<IInspectable*>(buffer)->QueryInterface(IID_PPV_ARGS(&bufferByteAccess));
	byte* data = nullptr;
	bufferByteAccess->Buffer(&data);
	return data;
}

Concurrency::task<Windows::Storage::Streams::IBuffer^> ReadAll(Windows::Storage::StorageFile ^ file)
{
	return Concurrency::create_task(file->GetBasicPropertiesAsync()).then([file](Windows::Storage::FileProperties::BasicProperties ^properties) {
		return Concurrency::create_task(file->OpenSequentialReadAsync()).then([file, size = properties->Size](Windows::Storage::Streams::IInputStream ^ istream)
		{
			Windows::Storage::Streams::Buffer ^ buffer = ref new Windows::Storage::Streams::Buffer(size);
			return Concurrency::create_task(istream->ReadAsync(buffer, size, Windows::Storage::Streams::InputStreamOptions::None));
		});
	});
}

void App::UsePrivatekey(Windows::Storage::StorageFile ^ file)
{
	ReadAll(file).then([this](Windows::Storage::Streams::IBuffer ^ buffer) {
		server->UsePrivateKey(GetDataPointer(buffer), buffer->Length);
	});
}

void App::UseCertificate(Windows::Storage::StorageFile ^ file)
{
	ReadAll(file).then([this, file](Windows::Storage::Streams::IBuffer ^ buffer) {
		server->UseCertificate(GetDataPointer(buffer), buffer->Length);
	});
}

void App::Start_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	server->Listen();
}
