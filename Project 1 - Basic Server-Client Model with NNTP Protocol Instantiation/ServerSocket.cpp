//Kolton Klette
//Professor Thoshita Gamage
//CS-447 - Network Communications
//February 23rd, 2021

//NOTE: Resource from Linux Gazette is cited in supplementary report.

#include "ServerSocket.h"
#include "SocketException.h"
#include <iostream>
using namespace std;


ServerSocket::ServerSocket(int port)
{
    cout << "PORT: " << port << "\n";
    if (!Socket::create())
    {
        throw SocketException("Could not create server socket.");
    }

    if (!Socket::bind(port))
    {
        throw SocketException("Could not bind to port.");
    }

    if (!Socket::listen())
    {
        throw SocketException("Could not listen to socket.");
    }

}

ServerSocket::~ServerSocket()
{
}


const ServerSocket& ServerSocket::operator << (const string& s) const
{
    if (!Socket::send(s))
    {
        throw SocketException("Could not write to socket.");
    }

    return *this;

}


const ServerSocket& ServerSocket::operator >> (string& s) const
{
    if (!Socket::recv(s))
    {
        throw SocketException("Could not read from socket.");
    }

    return *this;
}

void ServerSocket::accept(ServerSocket& sock)
{
    if (!Socket::accept(sock))
    {
        throw SocketException("Could not accept socket.");
    }
}