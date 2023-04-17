#include <iostream>
#include "server.h"

enum class BasicMsg: uint32_t
{
	Ping,
	Message,
	MessageAll,
};

class CustomServer : public sto::server<BasicMsg>
{
public:
	CustomServer(uint16_t port)
		:sto::server<BasicMsg>(port)
	{
	}

protected:
	virtual bool onClientConnect(std::shared_ptr<sto::connection<BasicMsg>> client) override
	{
		return true;
	}

	virtual void onClientDisconnect(std::shared_ptr<sto::connection<BasicMsg>> client) override
	{

	}

    virtual void OnMessage(std::shared_ptr<sto::connection<BasicMsg>> client, sto::message<BasicMsg>& msg) override
	{
		switch(msg.header.id)
		{
			case BasicMsg::Ping:
			{
				std::cout << "[" << client->getId() << "]: " << "Sever Pinged by Client: " << client->getId() << "\n";
				client->send(msg);
			}break;
			case BasicMsg::MessageAll:
			{
				msg.header.id = BasicMsg::Message;
				uint32_t userId = client->getId();
				msg << userId;

				messageAllClients(msg, client);
			}break;
			default:
				break;
		}
	}
};

void main()
{
	CustomServer server(60000);
	server.start();

	while(1)
	{
		server.update(-1, true);
	}
}