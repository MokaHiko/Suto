#include "Suto.h"

#include <string>
#include <array>

enum class BasicMsg : uint32_t
{
	Ping,
	Message,
	MessageAll,
};


std::thread chatInputThread;

class CustomClient : public sto::client<BasicMsg>
{
public:
	bool Ping()
	{
		sto::message<BasicMsg> msg;
		msg.header.id = BasicMsg::Ping;
		std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
		msg << timeNow;
		send(msg);

		return true;
	}

	bool MessageAll()
	{
		sto::message<BasicMsg> msg;
		msg.header.id = BasicMsg::MessageAll;

		std::string msgTxt = "pussy";
		std::getline(std::cin, msgTxt);

		uint32_t txtSize = static_cast<uint32_t>(msgTxt.size());

		// Send messages in reverse to be read (popped) in order
		for(auto c = msgTxt.rbegin(); c != msgTxt.rend(); ++c)
		{
			msg << *c;
		}

		msg << txtSize;

		send(msg);

		return true;
	}
};

int main(int argc, char** argv)
{
	if(argc != 2)
	{
		std::cout << "Usage Error: Follow Format => Client.exe %server_ip%" << std::endl;
		std::cin.get();
		return -1;
	}

	CustomClient c;
	c.connect(argv[1], 60000);

	bool isQuit = false;

	bool prevState = 0;
	bool state = 0;

	bool isStarted = false;
	std::array<char, 32000> messageBuffer = {};

	while (!isQuit)
	{
		if (c.isConnected())
		{
			// prevState = state;
			// if (GetKeyState('A') & 0x8000 /*Check if high-order bit is set (1 << 15)*/)
			// 	state = 1;
			// else
			// 	state = 0;

			// if(state != prevState && state == 1)
			// 	c.Ping();

			if (!isStarted)
			{
				std::cout << "===== WELCOME TO SUTO CHAT =====" << std::endl;
				chatInputThread = std::thread([&](){
					while(!isQuit)
						c.MessageAll();
				});
				isStarted = true;
			}

			if (!c.incomingMessages().empty())
			{
				sto::message<BasicMsg> msg = c.incomingMessages().pop_front().msg;

				switch (msg.header.id)
				{
				case BasicMsg::Ping:
				{
					std::chrono::system_clock::time_point timeNow = std::chrono::system_clock::now();
					std::chrono::system_clock::time_point timeThen;
					msg >> timeThen;
					std::cout << "PING: " << std::chrono::duration<double>(timeNow - timeThen).count() << std::endl;
				} break;
				case BasicMsg::Message:
				{
					uint32_t user = {};
					uint32_t messageSize = {};

					msg >> user;
					msg >> messageSize;

					std::cout << user << " : ";
					for(uint32_t i = 0; i < messageSize; i++)
					{
						msg >> messageBuffer[i];
						std::cout << messageBuffer[i];
					}
					std::cout << std::endl;
				} break;
				default:
					break;
				}
			}
		}
		else
		{
			isQuit = true;
			std::cout << "Disconnected from server!" << std::endl;
		}
	}


	if(chatInputThread.joinable())
		chatInputThread.join();

	return 0;
}