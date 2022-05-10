//Kolton Klette
//Professor Thoshita Gamage
//CS-447 - Network Communications
//February 23rd, 2021

//Include Libraries
#include "ClientSocket.h"
#include "SocketException.h"
#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <bits/stdc++.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/types.h>
#include <strings.h>
#include <pthread.h>
#include <semaphore.h>
#include <sstream>
#include <cstring>
#include <vector>
#include <algorithm>
#include <ctime>
#include <cstdio>
#include <ctype.h>
#include <dirent.h>
using namespace std;

//Function Declaration
string readConfFileIP(string fileName);
string readConfFilePORT(string fileName);
string removeWhiteSpace(string str);

//Main
int main(int argc, char* argv[])
{
	//Variable Declaration
	string getInput;
	string reply;
	string clientIP;
	string clientPORT;

	//Extracts IP/PORT Information from .conf files
	clientIP = readConfFileIP("client.conf");
	clientPORT = readConfFilePORT("client.conf");
	
	//Initial Connection to the Server
	ClientSocket client_socket(removeWhiteSpace(clientIP), stoi(removeWhiteSpace(clientPORT)));
	cout << "Client has connected to the server.\n";
	
	//Client Instance
	cout << "Please enter the message you would like to send to the server: " << "\n";
	
		while (true)
		{
			try
			{
				//Grab User Input
				getline(cin, getInput);

				//Sends information over to the server
				client_socket << getInput;
				client_socket >> reply;

				//Sever Connection to the Server If The Sent Command is QUIT, Graceful Disconnect
				if (getInput == "QUIT")
				{
					cout << reply << "\n";
					cout << "You have disconnected from the server successfully." << "\n";
					break;
				}
			}
			catch (SocketException&) {}

			//Test to Receive Information From The Server
			cout << reply << "\n";
		}
	
	
	//End of Main
	return 0;
}

//Other Functions
string readConfFileIP(string fileName)
{
	//Variable Declaration
	ifstream inFile;
	string input;
	string sendIP;      //saves the final value of the IP address from the .conf file
	string stringTrash; //saves the value of string characters that are not needed
	string stringValue; //saves the value of string IP
	
	
	//Open client.conf file
	inFile.open(fileName);

	//Checks if the file was opened
	if (!inFile)
	{
		cerr << "Unable to open " << fileName << "\n";
		exit(1); //forcibly ends the program if an error occurs
	}

	//Read the file
	while (getline(inFile, input, '\n'))
	{
		//cout << input << "\n";
		if (input.find("IP") != string::npos) //looks for statement in the file that contains "IP" value
		{
			istringstream inputStream(input);
			getline(inputStream, stringTrash, '=');  //takes the "garbage" information from the file read in and stores it to a dummy variable
			getline(inputStream, stringValue, '\n'); //Takes the literal PORT value and saves it to a string value
			for (int i = 0; i < stringValue.length(); i++)
			{
				sendIP += stringValue[i]; //Saves all elements of stringValue to sendIP
			}
		}
	}

	//Close the actual
	inFile.close();

	//Test Case: Print the new string statement
	//cout << "The value saved to sendIP: " << sendIP << "\n";

	return sendIP;
}

string readConfFilePORT(string fileName)
{
	//Variable Declaration
	ifstream inFile;
	string input; 
	string sendPORT;    //saves the final value of the PORT number from the .conf file
	string stringTrash; //saves the value of string characters that are not needed
	string stringValue; //saves the value of string IP


	//Open client.conf file
	inFile.open("client.conf");

	//Checks if the file was opened
	if (!inFile)
	{
		cerr << "Unable to open client.conf";
		exit(1); //forcibly ends the program if an error occurs
	}

	//Read the file
	while (getline(inFile, input, '\n'))
	{
		//cout << input << "\n";
		if (input.find("PORT") != string::npos) //looks for statement in the file that contains "IP" value
		{
			istringstream inputStream(input);
			getline(inputStream, stringTrash, '=');  //takes the "garbage" information from the file read in and stores it to a dummy variable
			getline(inputStream, stringValue, '\n'); //Takes the literal PORT value and saves it to a string value
			for (int i = 0; i < stringValue.length(); i++)
			{
				sendPORT += stringValue[i]; //Saves all elements of stringValue to sendIP
			}
		}
	}

	//Close the actual
	inFile.close();

	//Test Case: Print the new string statement
	//cout << "The value saved to sendPORT: " << sendPORT << "\n";

	return sendPORT;
}

string removeWhiteSpace(string str)
{
	str.erase(remove_if(str.begin(), str.end(), ::isspace), str.end());
	return str;
}