#pragma once
#include "Socket.h"
#include <cstdint>
#include <memory>
#include <functional>
#include <thread>

namespace Net
{
	class SocketListener
	{
	private:
		bool OnConnection(std::shared_ptr<Socket> socket);
		std::function<void(std::shared_ptr<Socket>)> _onconnection;
		std::shared_ptr<std::thread> listener;
		bool cancel;
		//int clients;
		std::shared_ptr<sockaddr> address;
	protected:
		virtual std::shared_ptr<Socket> Accept();
		SOCKET handle;
	public:
		SocketListener();
		virtual ~SocketListener();
		virtual std::shared_ptr<std::thread> Listen(const std::shared_ptr<sockaddr> &address, socklen_t addresslen);
		void Cancel();
		void SetConnectionHandler(std::function<void(std::shared_ptr<Socket>)> onconnection);
	};
}
