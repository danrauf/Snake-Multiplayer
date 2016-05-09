#include <ws2tcpip.h>
#include "ServerNetwork.h"
#include "Networking.h"


ServerNetwork::ServerNetwork(void)
{
	WSADATA wsaData;
	ListenSocket = INVALID_SOCKET;
	ClientSocket = INVALID_SOCKET;
	struct addrinfo *result = nullptr, hints;

	// initialize winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		printf("WSAStartup failed with error: %d\n", iResult);
		exit(1);
	}

	// set address information
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP; // TCP connection
	hints.ai_flags = AI_PASSIVE;

	// resolve the server address and port
	iResult = getaddrinfo(nullptr, DEFAULT_PORT, &hints, &result);

	if (iResult != 0)
	{
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		exit(1);
	}

	// create a socket for connecting to server
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	if (ListenSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		exit(1);
	}

	// set the socket to be nonblocking
	u_long iMode = 1;
	iResult = ioctlsocket(ListenSocket, FIONBIO, &iMode);

	if (iResult == SOCKET_ERROR)
	{
		printf("ioctlsocket failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		exit(1);
	}

	// setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, int(result->ai_addrlen));

	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		exit(1);
	}

	// no longer need address information
	freeaddrinfo(result);

	// start listening for new clients attempting to connect
	iResult = listen(ListenSocket, SOMAXCONN);

	if (iResult == SOCKET_ERROR)
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		exit(1);
	}
}


ServerNetwork::~ServerNetwork(void)
{
}


bool ServerNetwork::acceptNewClient(unsigned int &id)
{
	// accept new connection and save the socket
	ClientSocket = accept(ListenSocket, nullptr, nullptr);

	if (ClientSocket != INVALID_SOCKET)
	{
		// disable Nagle's algorithm to send actions immediately instead of combining packets
		char value = 1;
		setsockopt(ClientSocket, IPPROTO_TCP, TCP_NODELAY, &value, sizeof(value));

		// save new client's in session id table
		sessions.insert(std::pair<unsigned int, SOCKET>(id, ClientSocket));

		return true;
	}

	return false;
}


int ServerNetwork::receiveData(unsigned int client_id, char *recvbuf)
{
	if (sessions.find(client_id) != sessions.end())
	{
		SOCKET currentSocket = sessions[client_id];
		iResult = NetworkServices::receiveMessage(currentSocket, recvbuf, MAX_PACKET_SIZE);

		if (iResult == 0)
		{
			printf("Connection closed\n");
			closesocket(currentSocket);
		}

		return iResult;
	}

	return 0;
}


void ServerNetwork::sendToAll(char *packets, int totalSize)
{
	SOCKET currentSocket;
	std::map<unsigned int, SOCKET>::iterator iter;
	int iSendResult;

	for (iter = sessions.begin(); iter != sessions.end(); ++iter)
	{
		currentSocket = iter->second;
		iSendResult = NetworkServices::sendMessage(currentSocket, packets, totalSize);

		if (iSendResult == SOCKET_ERROR)
		{
			printf("send failed with error: %d\n", WSAGetLastError());
			closesocket(currentSocket);
		}
	}
}