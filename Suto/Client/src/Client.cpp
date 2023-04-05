#include "Client.h"

#include "Suto.h"

Client::Client()
{
}

Client::~Client()
{
}

enum class BasicMsg: uint32_t
{
	Ping,
	EatingAss
};

int main()
{
	sto::message<BasicMsg> msg;
	msg.header.id = BasicMsg::Ping;

	int a = 1;
	bool b = true;
	float c = 3.141592f;
	struct 
	{
		float x = 1.0f;
		float y = 1.0f;
	} d[5];

	msg << a;
	msg << b << c;
	msg << d;

	a = 99;
	b = false;
	c = 0.01f;

	msg >> d >> c >> b >> a;

	return 0;
}