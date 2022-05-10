//Kolton Klette
//Professor Thoshita Gamage
//CS-447 - Network Communications
//February 23rd, 2021

//NOTE: Resource from Linux Gazette is cited in supplementary report.

#include "ClientSocket.h"
#include "SocketException.h"
using namespace std;


ClientSocket::ClientSocket(string host, int port)
{
    if (!Socket::create())
    {
        throw SocketException("Could not create client socket.");
    }

    if (!Socket::connect(host, port))
    {
        throw SocketException("Could not bind to port.");
    }

}


const ClientSocket& ClientSocket::operator << (const string& s) const
{
    if (!Socket::send(s))
    {
        throw SocketException("Could not write to socket.");
    }

    return *this;

}


const ClientSocket& ClientSocket::operator >> (string& s) const
{
    if (!Socket::recv(s))
    {
        throw SocketException("Could not read from socket.");
    }

    return *this;
}