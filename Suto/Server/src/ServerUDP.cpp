#include "ServerUDP.h"

std::string GetDayTimeString()
{
	using namespace std; // For time_t, time and ctime;
	time_t now = time(0);
	return ctime(&now);
}

ServerUDP::ServerUDP()
	: m_Socket(m_Context, asio::ip::udp::endpoint(asio::ip::udp::v4(), 13))
{
	// Begin listening for messages
	StartReceive();
}

ServerUDP::~ServerUDP()
{
}

void ServerUDP::Run()
{
	// Start listening for connections
	try
	{
		m_Context.run();
	}
	catch (std::exception &e)
	{
		std::cerr << e.what() << std::endl;
	}
}

void ServerUDP::StartReceive()
{
	m_Socket.async_receive_from(asio::buffer(m_RecieveBuffer), m_RemoteEndpoint, [&](const asio::error_code &e, std::size_t length)
								{ HandleReceive(e, length); });
}

void ServerUDP::HandleReceive(const asio::error_code &e, size_t length)
{
	if (!e)
	{
		std::cout << "Recieved: " << length << " bytes | Value: " << e.value() << "| From Endpoint: " << m_RemoteEndpoint << std::endl;

		std::string message = GetDayTimeString();
		std::copy(message.begin(), message.end(), m_SendBuffer.data());

		m_Socket.async_send_to(asio::buffer(m_SendBuffer.data(), message.size()), m_RemoteEndpoint, [&](const asio::error_code& e, size_t length) {
			HandleSend(e, length);
		});
	}

	StartReceive();
}

void ServerUDP::HandleSend(const asio::error_code& e, size_t length)
{
	if(!e)
	{
		std::cout << "Sent: " << length << " bytes" << std::endl;
	}
}
