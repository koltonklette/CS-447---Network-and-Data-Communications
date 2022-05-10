//Kolton Klette
//Professor Thoshita Gamage
//CS-447 - Network Communications
//February 23rd, 2021

//NOTE: Resource from Linux Gazette is cited in supplementary report.

#ifndef Socket_class
#define Socket_class


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string>
#include <arpa/inet.h>
using namespace std;


const int MAXHOSTNAME = 200;
const int MAXCONNECTIONS = 5;
const int MAXRECV = 99999;

class Socket
{
public:
	Socket();
	virtual ~Socket();

	// Server initialization
	bool create();
	bool bind(const int port);
	bool listen() const;
	bool accept(Socket&) const;

	// Client initialization
	bool connect(const string host, const int port);

	// Data Transimission
	bool send(const string) const;
	int recv(string&) const;


	void set_non_blocking(const bool);

	bool is_valid() const { return m_sock != -1; }

private:

	int m_sock;
	sockaddr_in m_addr;


};


#endif