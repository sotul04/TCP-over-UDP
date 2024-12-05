#include <iostream>
#include <string>
#include <regex>
#include <cstdlib>
#include <cstdint>
#include <vector>
#include "nodes/Client.hpp"
#include "nodes/Server.hpp"
#include "file/FileTransfer.hpp"
using namespace std;

bool isValidIPAddress(const std::string &ip)
{
    const std::regex ipRegex(
        "^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\."
        "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\."
        "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\."
        "(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$");
    return std::regex_match(ip, ipRegex);
}

void senderAction();
void receiverAction();

string ip = "127.0.0.1";
uint16_t port = 8080;

int main(int argc, char *argv[])
{
    if (argc > 1)
    {
        if (argc == 2)
        {
            int portArg = atoi(argv[1]);
            if (portArg < 1 || portArg > 65535)
            {
                cerr << "Invalid port number. Must be between 1 and 65535" << endl;
                return 1;
            }
            port = static_cast<uint16_t>(portArg);
        }
        else
        {
            string ipArg(argv[1]);
            if (!isValidIPAddress(ipArg) && ipArg != "localhost")
            {
                cerr << "Invalid IP Address." << endl;
                return 1;
            }
            if (ipArg == "localhost")
                ipArg = "127.0.0.1";
            ip = ipArg;
            int portArg = atoi(argv[2]);
            if (portArg < 1 || portArg > 65535)
            {
                cerr << "Invalid port number. Must be between 1 and 65535" << endl;
                return 1;
            }
            port = static_cast<uint16_t>(portArg);
        }
    }

    int menuSelected = 0;
    cout << OUT << "Node will started at " << ip << ":" << port << endl;

    while (!menuSelected || menuSelected < 1 || menuSelected > 2)
    {
        cout << INPUT << "Please choose the operating mode: " << endl;
        cout << INPUT << "1. Sender" << endl;
        cout << INPUT << "2. Receiver" << endl;
        cout << INPUT << "Input: ";
        cin >> menuSelected;
    }

    if (menuSelected == 1)
        senderAction();
    else
        receiverAction();

    return 0;
}

void senderAction()
{
    cout << OUT << "Node is now a sender" << endl;
    int sendingMode = 0;

    while (!sendingMode || sendingMode < 1 || sendingMode > 2)
    {
        cout << INPUT << "Please choose the sending mode: " << endl;
        cout << INPUT << "1. User input" << endl;
        cout << INPUT << "2. File input" << endl;
        cout << INPUT << "Input: ";
        cin >> sendingMode;
    }

    cin.ignore(numeric_limits<streamsize>::max(), '\n'); 

    if (sendingMode == 1) // user input
    {
        cout << INPUT << "Input mode chosen, please enter your input:" << endl;
        string userInput;
        getline(cin, userInput);
        cout << OUT << "User input has been successfully received." << endl;
        vector<Segment> data = prepareStringSegments(userInput, 0);
        Server server(port);
        server.setData(data);
        server.run();
    }
    else // file input
    {
        cout << INPUT << "File mode chosen, please enter the file path: ";
        string pathInput;
        getline(cin, pathInput);
        vector<Segment> data = sendFile(pathInput,0);
        if (data.empty()) {
            cerr << ERROR << "Error: cannot open file " << pathInput << endl;
            return;
        }

        cout << OUT << "File has beed successfully read." << endl;
        Server server(port);
        server.setData(data);
        server.run();

    }
}

void receiverAction()
{
    cout << OUT << "Node is now a receiver" << endl;
    int portInput = 0;
    while (!portInput || portInput < 1 || portInput > 65535)
    {
        cout << INPUT << "Input the servers program's port: ";
        cin >> portInput;
    }
    uint16_t serverPort = static_cast<uint16_t>(portInput);
    Client client(ip, port);
    client.setServerPort(serverPort);
    client.run();
}