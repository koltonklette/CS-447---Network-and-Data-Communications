//Kolton Klette
//Professor Thoshita Gamage
//CS-447 - Network Communications
//February 23rd, 2021

//NOTE: Resource from Linux Gazette is cited in supplementary report.


#ifndef SocketException_class
#define SocketException_class

#include <string>
using namespace std;

class SocketException
{
public:
	SocketException(string s) : m_s(s) {};
	~SocketException() {};

	string description() { return m_s; }

private:

	string m_s;

};

#endif
