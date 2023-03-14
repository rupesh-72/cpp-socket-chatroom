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
        cout << "\r" << receivedMessage << endl << "You: " << flush; // \r is used to move the cursor to beginning of current line to print the received message before you: 
    }
    close(socketFd);
}

// reading your message and sending it to the server
void readMessageAndSendToServer(int socketFd, string name)
{   
    string connectedMessage = name + " is now connected.";
    string disconnectedMessage = name + " is now disconnected.";

    send(socketFd, connectedMessage.c_str(), connectedMessage.length() * sizeof(char), 0);

    string message;
    while (true) {
        cout << "You: " << flush;
        getline(cin, message);

        if (message == "exit") {
            send(socketFd, disconnectedMessage.c_str(), disconnectedMessage.length() * sizeof(char), 0);
            cout << "You are now disconnected.\n";
            break;
        }
        string sendMessage = name + ": " + message;
        sendMessage.push_back('\0'); // added null character to eliminate printing of message multiples times on the client side
        send(socketFd, sendMessage.c_str(), sendMessage.length() * sizeof(char), 0);
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
    string name;
    cout << "Enter your Name : ";
    getline(cin, name);
    int result = connect(sockFd, (struct sockaddr *)&server, sizeof(server));
    if(result == 0)
        cout << "You are now connected.\n";
    else
        cout << "Failed to connect to the server\n";


    createNewThreadToListenMessage(sockFd);
    readMessageAndSendToServer(sockFd, name);

    close(sockFd);
    shutdown(sockFd, SHUT_RDWR);

    return 0; 
}
