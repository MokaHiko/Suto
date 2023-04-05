#include "Client.h"

#include "Common.h"

Client::Client()
{
}

Client::~Client()
{
}

int main()
{
	try
	{
		asio::io_context io_context;

		// Resolve endpoint
		asio::ip::udp::resolver resolver(io_context);
		asio::ip::udp::endpoint receiever_endpoint = *resolver.resolve(asio::ip::udp::v4(), "127.0.0.1", "daytime").begin();

		// Create udp socket
		asio::ip::udp::socket socket(io_context);
		socket.open(asio::ip::udp::v4());

		// Send data to endpoint
		std::cout << "Sending data to server w endpoint: " << receiever_endpoint << std::endl;
		std::array<char, 1> send_buf = {{0}};
		socket.send_to(asio::buffer(send_buf), receiever_endpoint);

		// recieve data
		std::array<char, 128> recv_buf;
		asio::ip::udp::endpoint sender_endpoint;
		size_t len = socket.receive_from(asio::buffer(recv_buf), sender_endpoint);

		std::cout << "Recieved data from server w endpoint: " << receiever_endpoint << std::endl;
		std::cout.write(recv_buf.data(), len);

		std::cin.get();
	}
	catch (std::exception e)
	{
		std::cerr << e.what() << std::endl;
	}
}