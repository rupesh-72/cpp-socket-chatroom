#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <thread>

using namespace std;

void createNewThreadToListenMessage(int socketFd);
void printIncomingMessage(int socketFd);
void readMessageAndSendToServer(int socketFd);

// creating new thread to listen incoming messages
void createNewThreadToListenMessage(int socketFd)
{
    thread t(printIncomingMessage, socketFd);
    t.detach();
}

// printing the received messages
void printIncomingMessage(int socketFd)
{
    char receivedMessage[1024];
    while(true)
    {
        ssize_t amountReceived = recv(socketFd, receivedMessage, 1024, 0);
        if(amountReceived == 0)
        {
            break;
        }
        receivedMessage[amountReceived] = 0; // clear the previous message from buffer
        cout << receivedMessage << endl;
    }
    close(socketFd);
}

// reading your message and sending it to the server
void readMessageAndSendToServer(int socketFd)
{   
    string name;
    cout << "Enter your Name : ";
    getline(cin, name);

    string message;
    while (true) {
        cout << "You: ";
        getline(cin, message);

        if (message == "exit") {
            break;
        }
        send(socketFd, message.c_str(), message.length() * sizeof(char), 0);
    }
}

int main() 
{   
    // create socket
    int sockFd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockFd == -1) {
        cout << "Failed to create socket." << endl;
        return 1;
    }

    // the IP address and port number of the server to connect to
    const char* ip = "127.0.0.1";
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(2000); 
    inet_pton(AF_INET, ip, &server.sin_addr.s_addr);

    // Connect to the server
    int result = connect(sockFd, (struct sockaddr *)&server, sizeof(server));
    if(result == 0)
        cout << "Connection Succesful\n";
    else
        cout << "Failed to connect to the server\n";


    createNewThreadToListenMessage(sockFd);
    readMessageAndSendToServer(sockFd);

    close(sockFd);
    shutdown(sockFd, SHUT_RDWR);

    return 0; 
}
