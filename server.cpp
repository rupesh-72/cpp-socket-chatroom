#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include<thread>
#include<vector>

using namespace std;

void listenIncomingConnections(int listenAcceptedSocketFd);
void createNewThread(struct socketStructure *pSocket);
void printIncomingMessage(int socketFd);
void sendMessageToOtherClients(char *buffer, int socketFd);
socketStructure* acceptConnections(int listenAcceptedSocketFd);

// structure for the client socket file discriptor
struct socketStructure
{
    int acceptedSocketFd;
    sockaddr_in address;
    int error;
    bool acceptedCheck;
};

vector<socketStructure> acceptedClients; // vector to store the connected clients

// listening the incoming connections
void listenIncomingConnections(int listenAcceptedSocketFd)
{    
    while(true)
    {
        socketStructure *clientSocket = acceptConnections(listenAcceptedSocketFd);
        acceptedClients.push_back(*clientSocket);

        createNewThread(clientSocket);
    }
}

// creating new thread for every new client
void createNewThread(struct socketStructure *pSocket)
{
    thread t(printIncomingMessage, pSocket->acceptedSocketFd);
    t.detach();
}

// printing the received messages to the server
void printIncomingMessage(int socketFd)
{
    char buffer[1024];
    while(true)
    {
        ssize_t amountReceived = recv(socketFd, buffer, 1024, 0);
        if(amountReceived == 0)
        {
            break;
        }
        buffer[amountReceived] = 0; // clear the previous message from the buffer
        cout << buffer << endl;
        sendMessageToOtherClients(buffer, socketFd);
    }
    close(socketFd);
}

// sending the received messages to other clients
void sendMessageToOtherClients(char *buffer, int socketFd)
{
    for(int i = 0; i < acceptedClients.size(); i++)
    {
        if(acceptedClients[i].acceptedSocketFd != socketFd)
            send(acceptedClients[i].acceptedSocketFd, buffer, strlen(buffer), 0);
    }
}

// accepting the new connections
socketStructure* acceptConnections(int listenAcceptedSocketFd)
{
    sockaddr_in clientAddress;
    int clientAddressSize = sizeof(sockaddr_in);
    int clientSocketFd = accept(listenAcceptedSocketFd, (struct sockaddr *)&clientAddress, (socklen_t *)&clientAddressSize);

    socketStructure *acceptedSocket = new socketStructure;

    // assigning the data of new client to the structure
    acceptedSocket->acceptedSocketFd = clientSocketFd;
    acceptedSocket->address = clientAddress;
    acceptedSocket->acceptedCheck = clientSocketFd > 0;

    if(!acceptedSocket->acceptedCheck)
        acceptedSocket->error = clientSocketFd;

    return acceptedSocket;
}

int main()
{
    // create socket
    int serverSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocketFd == -1) {
        cout << "Failed to create socket." << endl;
        return 1;
    }

    // the IP address and port number of the server to connect to
    const char* ip = "";
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(2000); 
    server.sin_addr.s_addr = INADDR_ANY;

    //bind
    int result = bind(serverSocketFd, (struct sockaddr *)&server, sizeof(server));
    if(result == 0)
        cout << "Socket bind successfully\n";

    //listen
    int listenResult = listen(serverSocketFd, 10);

    listenIncomingConnections(serverSocketFd);

    shutdown(serverSocketFd,SHUT_RDWR);

    return 0;
}