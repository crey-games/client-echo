#include <enet/enet.h>
#include <vector>
#include <iostream>
#include <algorithm>

int main(int argc, char* argv[])
{
	enet_initialize();

	ENetAddress address;
	address.host = ENET_HOST_ANY;
	address.port = 12345;
	ENetHost* server = enet_host_create(&address, 32, 2, 0, 0);
	
	std::cout << "Server listening on address: 0.0.0.0:" << address.port<<std::endl;

	bool quit = false;

	while (!quit)
	{
		std::vector<ENetEvent> events;
		ENetEvent currentEvent;
		events.clear();
		events.reserve(512);
		while (enet_host_service(server, &currentEvent, 0) > 0)
		{
			events.push_back(currentEvent);
		}

		for (const auto& event : events)
		{
			switch (event.type)
			{
			case ENET_EVENT_TYPE_CONNECT:
			{
				std::cout << "Remote connection established." << std::endl;
				std::string hostName;
				hostName.resize(15);
				enet_address_get_host_ip(&event.peer->address,hostName.data(),15);
				std::cout << "Remote address: " << hostName << std::endl;
				std::cout << "Port:" << event.peer->address.port << std::endl;
				break;
			}
			case ENET_EVENT_TYPE_DISCONNECT:
			{
				std::cout << "Remote connection disconnected." << std::endl;
				std::string hostName;
				hostName.resize(15);
				enet_address_get_host_ip(&event.peer->address, hostName.data(), 15);
				std::cout << "Remote address: " << hostName << std::endl;
				std::cout << "Port:" << event.peer->address.port << std::endl;
				break;
			}
			case ENET_EVENT_TYPE_RECEIVE:
			{
				std::cout << "Received the following data from." << std::endl;
				std::string hostName;
				hostName.resize(15);
				enet_address_get_host_ip(&event.peer->address, hostName.data(), 15);
				std::cout << "Remote address: " << hostName << std::endl;
				std::cout << "Port:" << event.peer->address.port << std::endl;
				
				std::string dataAsString;

				dataAsString.resize(event.packet->dataLength);

				std::copy(event.packet->data,event.packet->data + event.packet->dataLength, dataAsString.data());

				std::cout << "Data: " << dataAsString << std::endl;

				//Send back data to source.

				enet_peer_send(event.peer,0,event.packet);

				if (dataAsString == "quit")
				{
					quit = true;
				}

				break;
			}
			case ENET_EVENT_TYPE_NONE:
			{
				continue;
			}
			default:
			{
				std::cerr << "Unknown event type, exiting...";
				return -1;
			}
			}
		}
	}

	enet_deinitialize();
}