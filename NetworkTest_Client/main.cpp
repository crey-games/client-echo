#include <enet/enet.h>
#include <vector>
#include <iostream>
#include <algorithm>

const std::string ipAddressArgumentString = "--ipAddress=";
const std::string portArgumentString = "--port=";

constexpr const size_t helpArgumentsArraySize = 3;
const std::string helpArgumentStrings[helpArgumentsArraySize] = { "--help","-h","/?" };

std::string serverIPAddress = "127.0.0.1";
uint16_t serverPort = 12345;
bool printHelpAndQuit = false;


void parseArguments(const std::vector<std::string>& arguments)
{
	for (const auto& argument : arguments)
	{
		if (argument.compare(0, ipAddressArgumentString.length(),ipAddressArgumentString) == 0)
		{
			serverIPAddress = argument.substr(ipAddressArgumentString.length());
		}
		else if (argument.compare(0, portArgumentString.length(), portArgumentString) == 0)
		{
			serverPort = (uint16_t)std::strtoul(argument.substr(portArgumentString.length()).c_str(),nullptr,0);
		}
		else
		{
			for (size_t i = 0; i < helpArgumentsArraySize; ++i)
			{
				if (argument == helpArgumentStrings[i])
				{
					printHelpAndQuit = true;
					break;
				}
			}
		}
	}
}

void printToolHelp()
{
	std::cout << "NetworkTest_Client.exe" << std::endl;
	std::cout << "enet test echo service client. Supports the following command line arguments:" << std::endl;
	std::cout << ipAddressArgumentString << "[server IPv4 address]" << std::endl;
	std::cout << portArgumentString << "[server UDP port]" << std::endl;
	for (size_t i = 0; i < helpArgumentsArraySize; ++i)
	{
		std::cout << helpArgumentStrings[i] << " print this help message to console and exit." << std::endl;
	}
}

int main(int argc, char* argv[])
{
	if (argc == 1)
	{
		printToolHelp();
	}
	std::vector<std::string> argumentsVector;
	for (int i = 1; i < argc; ++i)
	{
		argumentsVector.emplace_back(argv[i]);
	}

	parseArguments(argumentsVector);
	if (printHelpAndQuit)
	{
		printToolHelp();
		return 0;
	}

	std::cout << "Connecting to: " << serverIPAddress << ":" << serverPort << std::endl;;

	enet_initialize();

	
	ENetHost* server = enet_host_create(nullptr, 1, 2, 0, 0);

	ENetAddress address;
	address.port = serverPort;

	enet_address_set_host(&address, serverIPAddress.c_str());

	ENetPeer* peer = enet_host_connect(server, &address,2,0);

	//Wait for connection to succeed/fail.

	bool quit = false;
	bool connected = false;

	while (!connected)
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
				enet_address_get_host_ip(&event.peer->address, hostName.data(), 15);
				std::cout << "Remote address: " << hostName << std::endl;
				std::cout << "Port:" << event.peer->address.port << std::endl;
				connected = true;
				break;
			}
			case ENET_EVENT_TYPE_NONE:
			{
				continue;
			}
			default:
			{
				std::cerr << "Unexpected event type, exiting...";
				return -1;
			}
			}
		}
	}

	while (!quit && connected)
	{
		
		std::cout << "Please type message to echo and press enter. Type quit to quit client and server." << std::endl;
		std::string currentMessage;
		std::cin >> currentMessage;

		//DO NOT DESTROY PACKET, SEND WILL OWN IT!
		ENetPacket* packet = enet_packet_create(currentMessage.c_str(), currentMessage.size(), ENET_PACKET_FLAG_RELIABLE);

		bool ok = (enet_peer_send(peer, 0, packet) == 0);
		if (!ok)
		{
			std::cerr << "Unable to send message, exiting." << std::endl;
			break;
		}

		if (currentMessage == "quit")
		{
			enet_peer_disconnect_now(peer,0);
			quit = true;
			break;
		}

		Sleep(10);

		//Check messages.
		bool replyReceived = false;

		while (!replyReceived && connected)
		{
			std::vector<ENetEvent> events;
			ENetEvent currentEvent;
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
					enet_address_get_host_ip(&event.peer->address, hostName.data(), 15);
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
					connected = false;
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

					std::copy(event.packet->data, event.packet->data + event.packet->dataLength, dataAsString.data());

					std::cout << "Data: " << dataAsString << std::endl;

					replyReceived = true;

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
	}

	enet_deinitialize();
}