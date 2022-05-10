//Kolton Klette
//Professor Thoshita Gamage
//CS-447 - Network Communications
//February 23rd, 2021

//Include Libraries
#include "ServerSocket.h"
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
#include <sys/stat.h>
using namespace std;

//Function Declaration
string readConfFilePORT(string fileName);
string removeWhiteSpace(string str);
bool checkForNumber(const string& str); //used for ARTICLE; checks the validity of a number argument
string getCurrentWorkingDirectory();
vector<string> getAllNEWSGROUPNames(vector<string> temp, DIR* directory, struct dirent* entry, string dbDirectory);

//Main
int main(int argc, char* argv[])
{
	//Variable Declaration
	
		//Server Connection/Process Handling
		string serverPORT;
		string clientData;
		string homeDirc = getCurrentWorkingDirectory();                 //saves the current working directory when the server is called
		string dirc = getCurrentWorkingDirectory() + "/db/";            //saves the working directory to handle db information
		string serverResponse;                                          //saves the response that is gradually handled by the server
		vector<string> splitInput;                                      //saves and handles the client's response
		vector<string> newsGroupNames;                                  //saves the names of NEWSGROUPS in the database for future reference

		//Protocol-Specific
		string lowWaterMark;											//used for GROUP Protocol; saves the value of the lowWaterMark of a provided directory
		string highWaterMark;											//used for GROUP Protocol; saves the value of the highWaterMark of a provided directory
		vector<string> fileList;										//used for GROUP Protocol; saves the contents of a provided directory

		vector<int> sortedFileList;										//used for LISTGROUP Protocol; saves the contents of a provided directory w/o .txt extensions
		vector<int> rangeValues;										//used for LISTGROUP Protocol; saves the value of input when a range needs to be found

		vector<string> articleFileList;									//used for ARTICLE Protocol; saves the contents of a provided directory with file extensions
		vector<string> headerInformation;								//used for ARTICLE Protocol; saves individual lines of the HEADER for later organization
		string lowWaterMarkFile;										//used for ARTICLE Protocol; saves the file name of the low watermark in the current newsgroup
		string highWaterMarkFile;										//used for ARTICLE Protocol; saves the file name of the high watermark in the current newsgroup

		string currentlyIteratedArticle;                                //used for ARTICLE/NEXT Protocol; saves the file name of the currently iterated article
		
		int numberofNEXTs;                                              //used for NEXT Protocol; saves the amount of times NEXT has been called
		string NEXTmessage_ID;											//used for NEXT Protocol; saves the message_ID of the NEXT article iterated	
		string NEXTarticleNumber;                                       //used for NEXT Protocol; saves the NEXT article number iterated

		string currentlySelectedNewsGroup;								//used for various protocols; saves currently selected NEWSGROUP during runtime

		//Directory/Stat Information
		DIR* directory;
		struct dirent* entry;
		struct stat valid;

	//Fetch The Names Of All NEWSGROUPS In The db Directory
	newsGroupNames = getAllNEWSGROUPNames(newsGroupNames, directory, entry, dirc);

	//List of Implemented Instructions
	string functions[12] = { "GROUP", "LISTGROUP", "NEXT", "ARTICLE", "DATE", "HDR", "LIST HEADERS", "LIST ACTIVE", "LIST NEWSGROUP", "HELP", "CAPABILITIES", "QUIT" };

	//Extract NNTP_PORT from server.conf File
	serverPORT = readConfFilePORT("server.conf");

	//Server Start
	cout << "Awaiting a response from the client...\n";
	
	//Create server socket
	ServerSocket server(stoi(removeWhiteSpace(serverPORT)));

	while (true)
	{
		ServerSocket new_sock;
		server.accept(new_sock);
		cout << "A client has connected to the server properly." << "\n";
		try
		{
			if (!fork())
			{
				while (true)
				{
					//Declare a New Socket
					new_sock >> clientData; //information from the socket is received from a client

					//Split Response Into Vector Array For Easy Accessibility
					string temp; //temporarily saves data while splitting input
					istringstream splitInputStream(clientData);

					while (getline(splitInputStream, temp, ' '))
					{
						splitInput.push_back(removeWhiteSpace(temp)); //saves split input into a vector for later use, and cleans any whitespace if present
						temp = ""; //cleans the value of temp to be re-used by the loop.
					}

					//Test Case: Print Out The Values of the Vector
					for (int i = 0; i < splitInput.size(); i++)
					{
						cout << splitInput.at(i) << " ";
					}
					cout << "\n";

					//Response Received From The Client
					cout << "[C]: " << clientData << "\n";

					//Client Response Handling
					if (splitInput.at(0) == functions[0]) //GROUP Protocol
					{
						//Regardless of Case To Be Determined, clear sortedFileList
						sortedFileList.clear();

						//Variable Declaration
						string reqDirc;

						//Test If Valid Input Is Given
						if (splitInput.size() < 2)
						{
							serverResponse += "501 Additional arguments needed\n";
						}
						else
						{
							//Reset Values of Other Relevant Protocols
							numberofNEXTs = 0; //resets the value of NEXT Protocol calls when currentlySelectedNewsGroup is changed

							//Create Directory PATH to Requested newsgroup
							reqDirc = dirc + splitInput.at(1);

							//Test If Requested Directory Exists
							if (stat((reqDirc.c_str()), &valid) != 0)
							{
								serverResponse += "411 " + splitInput.at(1) + " is unknown\n";
							}
							else //Protocol Process
							{
								//Variable Declaration
								string nameOfFile;
								string dummyFile;
								size_t infoIndex; //used to remove .info file From listOfFiles
								size_t txtIndex; //used to remove .txt extensions from listofFiles
								int fileCount;

								//Save The Currently Selected NEWSGROUP
								currentlySelectedNewsGroup = splitInput.at(1); //saves DIRECTORY PATH of selected NEWSGROUP, if available.

								//Clear the File List
								fileList.clear();

								//Generate a List of Files
								if ((directory = opendir(reqDirc.c_str())) != nullptr)
								{
									while ((entry = readdir(directory)) != nullptr)
									{
										//Assignment File to Variable
										nameOfFile = entry->d_name;

										//Saves .info File to dummy value
										infoIndex = nameOfFile.find_last_of(".");
										dummyFile = nameOfFile.substr(0, infoIndex);

										if (dummyFile != "")
										{
											fileList.push_back(nameOfFile);
										}
									}
									closedir(directory);
								}

								//Erase Lingering Directory Navigation and .info File
								for (int i = 0; i < fileList.size(); i++)
								{
									if ((strcmp(removeWhiteSpace(fileList.at(i)).c_str(), ".")) == 0 ||
										(strcmp(removeWhiteSpace(fileList.at(i)).c_str(), "..")) == 0 ||
										(strcmp(removeWhiteSpace(fileList.at(i)).c_str(), ".info")) == 0)
									{
										fileList.erase(fileList.begin() + i);
									}
								}

								if (fileList.size() == 0)
								{
									serverResponse += "[S]: 211 0 0 0 " + splitInput.at(1) + "\n";
								}
								else
								{
									//Prep File Name Information - Strip .txt extensions
									for (int i = 0; i < fileList.size(); i++)
									{
										//checks to see if directory navigation items are the current item, and ignores them
										if ((strcmp(removeWhiteSpace(fileList.at(i)).c_str(), ".")) != 0 ||
											(strcmp(removeWhiteSpace(fileList.at(i)).c_str(), "..")) != 0)
										{
											size_t txtIndex = fileList.at(i).find_last_of(".");
											fileList.at(i) = fileList.at(i).substr(0, txtIndex);
										}
									}

									//Finding High Watermarks - Saving highWaterMark File Name
									highWaterMark = fileList.at(0);
									for (int i = 0; i < fileList.size(); i++)
									{
										if (stoi(removeWhiteSpace(highWaterMark)) < stoi(removeWhiteSpace(fileList.at(i))))
										{
											highWaterMark = fileList.at(i);
										}
									}
									highWaterMarkFile = highWaterMark + ".txt";

									//Finding Low Watermark - Saving lowWaterMark File Name
									lowWaterMark = fileList.at(0);
									for (int i = 0; i < fileList.size(); i++)
									{
										if (stoi(removeWhiteSpace(lowWaterMark)) > stoi(removeWhiteSpace(fileList.at(i))))
										{
											lowWaterMark = fileList.at(i);
										}
									}
									lowWaterMarkFile = lowWaterMark + ".txt";

									//Fill sortedFileList When NewsGroup is selected
									for (int i = 0; i < fileList.size(); i++)
									{
										sortedFileList.push_back(stoi(fileList.at(i)));
									}
									sort(sortedFileList.begin(), sortedFileList.end());

									//Set the currentlyIteratedArticle to the 2nd File In The List
									currentlyIteratedArticle = removeWhiteSpace(to_string(sortedFileList.at(0)));
									numberofNEXTs = 0;                                                             //save the position of the current article number

									//Process serverResponse - "[S]: 211 [#OfFiles] [lowWaterMark] [highWaterMark] [nameOfGroup]" 
									serverResponse += "[S]: 211 " + to_string(fileList.size()) + " " + lowWaterMark + " " + highWaterMark + " " + splitInput.at(1) + "\n";
								}
							}
						}
					}
					else if (splitInput.at(0) == functions[1]) //LISTGROUP Protocol
					{
						//Regardless of Case To Be Determined, clear sortedFileList
						sortedFileList.clear();
						rangeValues.clear();

						//Test Case - Print Out the Currently Saved News Group
						//cout << currentlySelectedNewsGroup << "\n";

						//Variable Declaration
						string reqDirc;

						//Test If Valid Input Is Given
						if (splitInput.size() > 3)
						{
							serverResponse += "501 Too many arguments; please provide a selected newgroup and a range of numbers (###-###).\n";
						}
						else if ((splitInput.size() == 1) && currentlySelectedNewsGroup == "") //Provided if currentlySelectedNewGroup has not been provided a value
						{
							serverResponse += "412 No newsgroup selected; please run the GROUP protocol to select a newsgroup.\n";
						}
						else
						{
							if (splitInput.size() == 3) //LISTGROUP Protocol w/ LISTGROUP, specified newsgroup, and range value
							{
								//Prep Range Input
								string rangeTemp;
								istringstream rangeStream(splitInput.at(2));
								while (getline(rangeStream, rangeTemp, '-'))
								{
									rangeValues.push_back(stoi(removeWhiteSpace(rangeTemp)));
									rangeTemp = "";
								}
								//NOTE: Element 0 of rangeValues is considered the minimum, and the Element 1 (if present) is considered the maximum

								//Test Case - Print Range Values
								for (int i = 0; i < rangeValues.size(); i++)
								{
									cout << rangeValues.at(i) << "\n";
								}

								//Create Directory PATH to Saved NEWSGROUP
								reqDirc = dirc + splitInput.at(1);

								//Test If Requested Directory Exists
								if (stat((reqDirc.c_str()), &valid) != 0)
								{
									serverResponse += "411 " + splitInput.at(1) + " is unknown\n";
								}
								else
								{
									//Variable Declaration
									string nameOfFile;
									string dummyFile;
									size_t infoIndex; //used to remove .info file From listOfFiles
									size_t txtIndex; //used to remove .txt extensions from listofFiles
									int fileCount;

									//Clear the File List And Sorted File List
									fileList.clear();
									sortedFileList.clear();

									//Generate a List of Files
									if ((directory = opendir(reqDirc.c_str())) != nullptr)
									{
										while ((entry = readdir(directory)) != nullptr)
										{
											//Assignment File to Variable
											nameOfFile = entry->d_name;

											//Saves .info File to dummy value
											infoIndex = nameOfFile.find_last_of(".");
											dummyFile = nameOfFile.substr(0, infoIndex);

											if (dummyFile != "")
											{
												fileList.push_back(nameOfFile);
											}
										}
										closedir(directory);
									}

									//Erase Lingering Directory Navigation and .info File
									for (int i = 0; i < fileList.size(); i++)
									{
										if ((strcmp(removeWhiteSpace(fileList.at(i)).c_str(), ".")) == 0 ||
											(strcmp(removeWhiteSpace(fileList.at(i)).c_str(), "..")) == 0 ||
											(strcmp(removeWhiteSpace(fileList.at(i)).c_str(), ".info")) == 0)
										{
											fileList.erase(fileList.begin() + i);
										}
									}

									if (fileList.size() == 0)
									{
										serverResponse += "[S]: 211 0 0 0 " + splitInput.at(1) + "\n";
									}
									else
									{
										//Prep File Name Information - Strip .txt extensions
										for (int i = 0; i < fileList.size(); i++)
										{
											//checks to see if directory navigation items are the current item, and ignores them
											if ((strcmp(removeWhiteSpace(fileList.at(i)).c_str(), ".")) != 0 ||
												(strcmp(removeWhiteSpace(fileList.at(i)).c_str(), "..")) != 0)
											{
												size_t txtIndex = fileList.at(i).find_last_of(".");
												fileList.at(i) = fileList.at(i).substr(0, txtIndex);
											}
										}

										//Finding High Watermarks
										highWaterMark = fileList.at(0);
										for (int i = 0; i < fileList.size(); i++)
										{
											if (stoi(removeWhiteSpace(highWaterMark)) < stoi(removeWhiteSpace(fileList.at(i))))
											{
												highWaterMark = fileList.at(i);
											}
										}

										//Finding Low Watermark
										lowWaterMark = fileList.at(0);
										for (int i = 0; i < fileList.size(); i++)
										{
											if (stoi(removeWhiteSpace(lowWaterMark)) > stoi(removeWhiteSpace(fileList.at(i))))
											{
												lowWaterMark = fileList.at(i);
											}
										}

										//Add Files from String fileList to Int sortedFileList
										for (int i = 0; i < fileList.size(); i++)
										{
											sortedFileList.push_back(stoi(fileList.at(i)));
										}
										sort(sortedFileList.begin(), sortedFileList.end());

										//Process serverResponse - "[S]: 211 [#OfFiles] [lowWaterMark] [highWaterMark] [nameOfGroup]" 
										serverResponse += "[S]: 211 " + to_string(fileList.size()) + " " + lowWaterMark + " " + highWaterMark + " " + splitInput.at(1) + " list follows:" + "\n";

										//Process serverResponse - File List Under Range Constraints
										if (rangeValues.size() == 1) //condition to print if empty range is given
										{
											//serverResponse += "[S]: .\n";
										}
										else //condition to print if both range values are given
										{
											for (int i = 0; i < sortedFileList.size(); i++)
											{
												if ((sortedFileList.at(i) >= rangeValues.at(0)) && (sortedFileList.at(i) <= rangeValues.at(1)))
												{
													serverResponse += "[S]: " + to_string(sortedFileList.at(i)) + "\n";
												}
											}
										}
									}
								}
							}
							else if (splitInput.size() == 2) //LISTGROUP Protocol w/ LISTGROUP and specified newsgroup
							{
								//Create Directory PATH to Saved NEWSGROUP
								reqDirc = dirc + splitInput.at(1);

								//Test If Requested Directory Exists
								if (stat((reqDirc.c_str()), &valid) != 0)
								{
									serverResponse += "411 " + splitInput.at(1) + " is unknown\n";
								}
								else
								{
									//Variable Declaration
									string nameOfFile;
									string dummyFile;
									size_t infoIndex; //used to remove .info file From listOfFiles
									size_t txtIndex; //used to remove .txt extensions from listofFiles
									int fileCount;

									//Clear the File List And Sorted File List
									fileList.clear();
									sortedFileList.clear();

									//Generate a List of Files
									if ((directory = opendir(reqDirc.c_str())) != nullptr)
									{
										while ((entry = readdir(directory)) != nullptr)
										{
											//Assignment File to Variable
											nameOfFile = entry->d_name;

											//Saves .info File to dummy value
											infoIndex = nameOfFile.find_last_of(".");
											dummyFile = nameOfFile.substr(0, infoIndex);

											if (dummyFile != "")
											{
												fileList.push_back(nameOfFile);
											}
										}
										closedir(directory);
									}

									//Erase Lingering Directory Navigation and .info File
									for (int i = 0; i < fileList.size(); i++)
									{
										if ((strcmp(removeWhiteSpace(fileList.at(i)).c_str(), ".")) == 0 ||
											(strcmp(removeWhiteSpace(fileList.at(i)).c_str(), "..")) == 0 ||
											(strcmp(removeWhiteSpace(fileList.at(i)).c_str(), ".info")) == 0)
										{
											fileList.erase(fileList.begin() + i);
										}
									}

									if (fileList.size() == 0)
									{
										serverResponse += "[S]: 211 0 0 0 " + splitInput.at(1) + "\n";
									}
									else
									{
										//Prep File Name Information - Strip .txt extensions
										for (int i = 0; i < fileList.size(); i++)
										{
											//checks to see if directory navigation items are the current item, and ignores them
											if ((strcmp(removeWhiteSpace(fileList.at(i)).c_str(), ".")) != 0 ||
												(strcmp(removeWhiteSpace(fileList.at(i)).c_str(), "..")) != 0)
											{
												size_t txtIndex = fileList.at(i).find_last_of(".");
												fileList.at(i) = fileList.at(i).substr(0, txtIndex);
											}
										}

										//Finding High Watermarks
										highWaterMark = fileList.at(0);
										for (int i = 0; i < fileList.size(); i++)
										{
											if (stoi(removeWhiteSpace(highWaterMark)) < stoi(removeWhiteSpace(fileList.at(i))))
											{
												highWaterMark = fileList.at(i);
											}
										}

										//Finding Low Watermark
										lowWaterMark = fileList.at(0);
										for (int i = 0; i < fileList.size(); i++)
										{
											if (stoi(removeWhiteSpace(lowWaterMark)) > stoi(removeWhiteSpace(fileList.at(i))))
											{
												lowWaterMark = fileList.at(i);
											}
										}

										//Add Files from String fileList to Int sortedFileList
										for (int i = 0; i < fileList.size(); i++)
										{
											sortedFileList.push_back(stoi(fileList.at(i)));
										}
										sort(sortedFileList.begin(), sortedFileList.end());

										//Process serverResponse - "[S]: 211 [#OfFiles] [lowWaterMark] [highWaterMark] [nameOfGroup]" 
										serverResponse += "[S]: 211 " + to_string(fileList.size()) + " " + lowWaterMark + " " + highWaterMark + " " + splitInput.at(1) + " list follows:" + "\n";

										//Process serverResponse - File List
										for (int i = 0; i < sortedFileList.size(); i++)
										{
											serverResponse += "[S]: " + to_string(sortedFileList.at(i)) + "\n";
										}
									}
								}
							}
							else //LISTGROUP Protocol w/ just LISTGROUP
							{
								//Test Case - Print Out Currently Selected News Group
								//cout << currentlySelectedNewsGroup << "\n";

								//Create Directory PATH to Saved NEWSGROUP
								reqDirc = dirc + currentlySelectedNewsGroup;

								//Test If Requested Directory Exists
								if (stat((reqDirc.c_str()), &valid) != 0)
								{
									serverResponse += "411 " + currentlySelectedNewsGroup + " is unknown\n";
								}
								else
								{
									//Variable Declaration
									string nameOfFile;
									string dummyFile;
									size_t infoIndex; //used to remove .info file From listOfFiles
									size_t txtIndex; //used to remove .txt extensions from listofFiles
									int fileCount;

									//Clear the File List And Sorted File List
									fileList.clear();
									sortedFileList.clear();

									//Generate a List of Files
									if ((directory = opendir(reqDirc.c_str())) != nullptr)
									{
										while ((entry = readdir(directory)) != nullptr)
										{
											//Assignment File to Variable
											nameOfFile = entry->d_name;

											//Saves .info File to dummy value
											infoIndex = nameOfFile.find_last_of(".");
											dummyFile = nameOfFile.substr(0, infoIndex);

											if (dummyFile != "")
											{
												fileList.push_back(nameOfFile);
											}
										}
										closedir(directory);
									}

									//Erase Lingering Directory Navigation and .info File
									for (int i = 0; i < fileList.size(); i++)
									{
										if ((strcmp(removeWhiteSpace(fileList.at(i)).c_str(), ".")) == 0 ||
											(strcmp(removeWhiteSpace(fileList.at(i)).c_str(), "..")) == 0 ||
											(strcmp(removeWhiteSpace(fileList.at(i)).c_str(), ".info")) == 0)
										{
											fileList.erase(fileList.begin() + i);
										}
									}

									if (fileList.size() == 0)
									{
										serverResponse += "[S]: 211 0 0 0 " + currentlySelectedNewsGroup + "\n";
									}
									else
									{
										//Prep File Name Information - Strip .txt extensions
										for (int i = 0; i < fileList.size(); i++)
										{
											//checks to see if directory navigation items are the current item, and ignores them
											if ((strcmp(removeWhiteSpace(fileList.at(i)).c_str(), ".")) != 0 ||
												(strcmp(removeWhiteSpace(fileList.at(i)).c_str(), "..")) != 0)
											{
												size_t txtIndex = fileList.at(i).find_last_of(".");
												fileList.at(i) = fileList.at(i).substr(0, txtIndex);
											}
										}

										//Finding High Watermarks
										highWaterMark = fileList.at(0);
										for (int i = 0; i < fileList.size(); i++)
										{
											if (stoi(removeWhiteSpace(highWaterMark)) < stoi(removeWhiteSpace(fileList.at(i))))
											{
												highWaterMark = fileList.at(i);
											}
										}

										//Finding Low Watermark
										lowWaterMark = fileList.at(0);
										for (int i = 0; i < fileList.size(); i++)
										{
											if (stoi(removeWhiteSpace(lowWaterMark)) > stoi(removeWhiteSpace(fileList.at(i))))
											{
												lowWaterMark = fileList.at(i);
											}
										}

										//Add Files from String fileList to Int sortedFileList
										for (int i = 0; i < fileList.size(); i++)
										{
											sortedFileList.push_back(stoi(fileList.at(i)));
										}
										sort(sortedFileList.begin(), sortedFileList.end());

										//Process serverResponse - "[S]: 211 [#OfFiles] [lowWaterMark] [highWaterMark] [nameOfGroup]" 
										serverResponse += "[S]: 211 " + to_string(fileList.size()) + " " + lowWaterMark + " " + highWaterMark + " " + currentlySelectedNewsGroup + " list follows:" + "\n";

										//Process serverResponse - File List
										for (int i = 0; i < sortedFileList.size(); i++)
										{
											serverResponse += "[S]: " + to_string(sortedFileList.at(i)) + "\n";
										}
									}
								}
							}
						}
					}
					else if (splitInput.at(0) == functions[2]) //NEXT Protocol
					{
						if (splitInput.size() > 1)
						{
							serverResponse += "501 Too many arguments; please only provide the NEXT keyword while an existing newsgroup is valid.\n";
						}
						else if (currentlySelectedNewsGroup == "")
						{
							serverResponse += "[S]: 412 No newsgroup selected; please run the GROUP protocol to select a newsgroup.\n";
						}
						else if (fileList.size() == 0)
						{
							serverResponse += "[S]: 420 No current article selected; the selected newsgroup is empty.\n";
						}
						else
						{
							//Variable Declaration
							string articleNEXT;
							//int articleNumberNEXT = numberofNEXTs; 
							vector<string> tempVectorNEXT;
							fstream tempFileNEXT;
							string tempFileName;
							string tempInputNEXT;
							string tempTrashNEXT;
							string tempLineInputNEXT;
							string tempMessageID;

							//NEXT Process
							if ((numberofNEXTs + 1) >= sortedFileList.size()) //The end of the newsgroup has been reached
							{
								serverResponse += "[S]: 421 No next article to retrieve; end of the current newsgroup.\n";
							}
							else
							{
								//Grab the Next File In the Current Directory
								articleNEXT = removeWhiteSpace(to_string(sortedFileList.at(numberofNEXTs + 1)));

								//File Handling

								//Construct File Path to open corresponding file
								tempFileName = dirc + currentlySelectedNewsGroup + "/" + articleNEXT + ".txt";

								//Open File
								tempFileNEXT.open(tempFileName);
								if (!tempFileNEXT)
								{
									cerr << "Unable to open " << tempFileName << "\n";
									exit(1); //forcibly ends the program if an error occurs
								}

								//Grab HEADER Information From NEXT Article
								while (getline(tempFileNEXT, tempInputNEXT, '\n'))
								{
									if (tempInputNEXT.empty() || tempInputNEXT == " ")
									{
										break;
									}
									tempVectorNEXT.push_back(tempInputNEXT); //saves HEADER information to a vector for later reference
								}

								//Close File
								tempFileNEXT.close();

								//Find Message-ID in the selected file
								for (int j = 0; j < tempVectorNEXT.size(); j++)
								{
									if (tempVectorNEXT.at(j).find("Message-ID:") != string::npos)
									{
										istringstream headerLine(tempVectorNEXT.at(j));
										getline(headerLine, tempTrashNEXT, ':');           //saves "trash" information to unused variable
										getline(headerLine, tempLineInputNEXT, '\n');      //saves wanted information to desired variable
										tempMessageID = tempLineInputNEXT;
									}
								}

								//Increment the Value of numberofNEXT
								numberofNEXTs++;

								//Prep serverResponse
								serverResponse += "[S]: 223 " + removeWhiteSpace(articleNEXT) + " " + removeWhiteSpace(tempMessageID) + " retrieved.\n";
							}
						}
					}
					else if (splitInput.at(0) == functions[3]) //ARTICLE Protocol
					{
						if (splitInput.size() > 2) //checks to see if too many arguments were provided by ARTICLE
						{
							serverResponse += "501 Too many arguments; please provide a legal message-ID or article number as an additional argument.\n";
						}
						else if (splitInput.size() == 2) //check what kind of input is provided for the argument
						{
							if (checkForNumber(splitInput.at(1))) //checks if second input is a number
							{
								if (currentlySelectedNewsGroup == "") //checks if currentlySelectedNewsGroup is invalid
								{
									serverResponse += "[S]: 412 No newsgroup selected; please run the GROUP protocol to select a newsgroup.\n";
								}
								else if (fileList.size() == 0) //checks if currentlySelectedNewsGroup is empty
								{
									serverResponse += "[S]: 420 No current article selected; the selected newsgroup is empty.\n";
								}
								else //ARTICLE Protocol call w/ article number
								{
									//Variable Declaration
									int articleNumber = stoi(splitInput.at(1));
									bool containsArticle = false;
									string articleNumberFile = splitInput.at(1) + ".txt";
									string changedDirectory;
									string emptylineInput;
									string tempInput;
									ifstream articleNumberStream;

									//Change currentlyIteratedArticle
									currentlyIteratedArticle = splitInput.at(1);

									//Check to see if provided article exists in the newsgroup
									for (int i = 0; i < sortedFileList.size(); i++)
									{
										if (sortedFileList.at(i) == articleNumber)
										{
											containsArticle = true;
											numberofNEXTs = i; //save the position of the article if it is found
										}
									}

									if (containsArticle == true)
									{
										//Dip Into currentlySelectedNewsGroup to read lowWaterMarkFile Information
										changedDirectory = dirc + currentlySelectedNewsGroup; //changes the directory to read fileNames

										//Change the Directory to the currentlySelectedNewsGroup Directory
										chdir(changedDirectory.c_str());

										//File Handling
										articleNumberStream.open(articleNumberFile);

										if (!articleNumberStream)
										{
											cerr << "Unable to open " << articleNumberFile << "\n";
											exit(1); //forcibly ends the program if an error occurs
										}

										//Read articleNumberFile

										//Save Header Information
										while (getline(articleNumberStream, tempInput, '\n'))
										{
											if (tempInput.empty() || tempInput == " ")
											{
												serverResponse += "[S]: \n";
												break;
											}
											headerInformation.push_back(tempInput); //saves HEADER information to a vector for later reference
											serverResponse += "[S]: " + tempInput + "\n";
										}

										//Read Rest Of The File
										while (getline(articleNumberStream, tempInput, '\n'))
										{
											serverResponse += "[S]: " + tempInput + "\n";
										}

										//File Close
										articleNumberStream.close();

										//Change the Directory back to the "Home" Directory where 
										chdir(homeDirc.c_str());
									}
									else
									{
										serverResponse += "[S]: 423 No article with that number; please use LISTGROUP to see legal article numbers.\n";
									}
								}
							}
							else //ARTICLE Protocol call w/ <message-ID>
							{
								//Variable Declaration
								bool foundMatch = false;
								string foundFile;
								int count = 0;

								string currentDirectory;
								string nameOfFile;
								string dummyFile;
								string tempFile;
								string tempInput;
								string tempTrash;
								string tempLineInput;
								string articleInfo;
								string directoryForCurrentFile;

								ifstream tempFileStream;
								size_t infoIndex;
								vector<string> tempVector;

								//Find A Matching Message ID
								while (count < newsGroupNames.size())
								{
									//Change Directory to an Existing NEWSGROUP
									currentDirectory = dirc + newsGroupNames.at(count);

									//Generate a List of Files
									if ((directory = opendir(currentDirectory.c_str())) != nullptr)
									{
										while ((entry = readdir(directory)) != nullptr)
										{
											//Assignment File to Variable
											nameOfFile = entry->d_name;

											//Saves .info File to dummy value
											infoIndex = nameOfFile.find_last_of(".");
											dummyFile = nameOfFile.substr(0, infoIndex);

											if (dummyFile != "")
											{
												articleFileList.push_back(nameOfFile);
											}
										}
										closedir(directory);
									}

									//Erase Lingering Directory Navigation
									for (int i = 0; i < articleFileList.size(); i++)
									{
										if ((strcmp(removeWhiteSpace(articleFileList.at(i)).c_str(), ".")) == 0)
										{
											articleFileList.erase(articleFileList.begin() + i);
										}
										if ((strcmp(removeWhiteSpace(articleFileList.at(i)).c_str(), "..")) == 0)
										{
											articleFileList.erase(articleFileList.begin() + i);
										}
									}

									//Read HEADER Information from Files In Current Directory
									for (int i = 0; i < articleFileList.size(); i++)
									{
										tempFile = removeWhiteSpace(articleFileList.at(i));

										//File Handling
										directoryForCurrentFile = currentDirectory + "/" + tempFile;
										tempFileStream.open(directoryForCurrentFile);

										if (!tempFileStream)
										{
											cerr << "Unable to open " << tempFile << "\n";
											exit(1); //forcibly ends the program if an error occurs
										}

										//Save Header Information
										while (getline(tempFileStream, tempInput, '\n'))
										{
											if (tempInput.empty() || tempInput == " ")
											{
												break;
											}
											tempVector.push_back(tempInput); //saves HEADER information to a vector for later reference
										}

										tempFileStream.close();

										//Find Message-ID Value From the HEADER Vector, then compare to input 
										for (int j = 0; j < tempVector.size(); j++)
										{
											if (tempVector.at(j).find("Message-ID:") != string::npos)
											{
												istringstream headerLine(tempVector.at(j));
												getline(headerLine, tempTrash, ':'); //saves "trash" information to unused variable
												getline(headerLine, tempLineInput, '\n'); //saves wanted information to desired variable

												if (removeWhiteSpace(tempLineInput) == removeWhiteSpace(splitInput.at(1)))
												{
													foundMatch = true;
													foundFile = directoryForCurrentFile;
													headerInformation = tempVector;
												}
											}
										}

										//Clear Header Information
										tempVector.clear();
									}

									//Clear Information When After Each Iteration
									articleFileList.clear();

									//If Matching Message-ID has not been found
									count++;
								}

								if (foundMatch == false) //if the message-ID is still not found after iterating through the db
								{
									serverResponse += "[S]: 430 No such article found; message-id was not found in the database.\n";
								}
								else //if the message-id is found
								{
									//File Reading
									tempFileStream.open(foundFile);

									while (getline(tempFileStream, articleInfo, '\n'))
									{
										serverResponse += "[S]: " + articleInfo + "\n";
									}

									tempFileStream.close();
								}
							}
						}
						else //ARTICLE Protocol call w/ no arguments
						{
							if (currentlySelectedNewsGroup == "") //checks if currentlySelectedNewsGroup is invalid
							{
								serverResponse += "[S]: 412 No newsgroup selected; please run the GROUP protocol to select a newsgroup.\n";
							}
							else if (fileList.size() == 0) //checks if currentlySelectedNewsGroup is empty
							{
								serverResponse += "[S]: 420 No current article selected; the selected newsgroup is empty.\n";
							}
							else
							{
								//Variable Declaration
								string changedDirectory;
								string emptylineInput;
								string tempInput;
								ifstream lowWaterMarkStream;

								//Dip Into currentlySelectedNewsGroup to read lowWaterMarkFile Information
								changedDirectory = dirc + currentlySelectedNewsGroup; //changes the directory to read fileNames

								//Change the Directory to the currentlySelectedNewsGroup Directory
								chdir(changedDirectory.c_str());

								//File Handling
								lowWaterMarkStream.open(lowWaterMarkFile);

								if (!lowWaterMarkStream)
								{
									cerr << "Unable to open " << lowWaterMarkFile << "\n";
									exit(1); //forcibly ends the program if an error occurs
								}

								//Read lowWaterMarkFile

								//Save Header Information
								while (getline(lowWaterMarkStream, tempInput, '\n'))
								{
									if (tempInput.empty() || tempInput == " ")
									{
										serverResponse += "[S]: \n";
										break;
									}
									headerInformation.push_back(tempInput); //saves HEADER information to a vector for later reference
									serverResponse += "[S]: " + tempInput + "\n";
								}

								//Read Rest Of The File
								while (getline(lowWaterMarkStream, tempInput, '\n'))
								{
									serverResponse += "[S]: " + tempInput + "\n";
								}

								//File Close
								lowWaterMarkStream.close();

								//Change the Directory back to the "Home" Directory where 
								chdir(homeDirc.c_str());
							}
						}

					}
					else if (splitInput.at(0) == functions[4]) //DATE Protocol
					{
						//Variable Declaration
						time_t tempTime;
						tm* timeStamp;
						char infoBuffer[80];

						//<ctime> Library Functions To Get The Current TimeStamp
						time(&tempTime);
						timeStamp = localtime(&tempTime);

						strftime(infoBuffer, 80, "%Y%m%d%H%M%S", timeStamp);
						puts(infoBuffer);

						//Convert Char Buffer Into String to Send Over serverResponse
						string infoBufferString = "";
						for (int i = 0; i < (sizeof(infoBuffer) / sizeof(infoBuffer[0])); i++)
						{
							infoBufferString = infoBufferString + infoBuffer[i];
						}

						//Test Case - Print Value of infoBufferString
						cout << infoBufferString << "\n";

						//Setting serverResponse
						serverResponse += "[S]: 111 " + infoBufferString + "\n";

					}
					else if (splitInput.at(0) == functions[5]) //HDR (First-Form) Protocol
					{
						if (splitInput.size() < 3) //Temporary condition; only first-form
						{
							serverResponse += "501 Additional arguments needed.\n";
						}
						else //HDR Protocol call w/ <message-ID>
						{
							//Variable Declaration
							bool foundMatch = false;
							string foundFile;
							int count = 0;

							string currentDirectory;
							string nameOfFile;
							string dummyFile;
							string tempFile;
							string tempInput;
							string tempTrash;
							string tempLineInput;
							string articleInfo;
							string directoryForCurrentFile;
							string fieldLine;

							ifstream tempFileStream;
							size_t infoIndex;
							vector<string> tempVector;
							vector<string> HDRFileList;

							//Find A Matching Message ID
							while (count < newsGroupNames.size())
							{
								//Change Directory to an Existing NEWSGROUP
								currentDirectory = dirc + newsGroupNames.at(count);

								//Generate a List of Files
								if ((directory = opendir(currentDirectory.c_str())) != nullptr)
								{
									while ((entry = readdir(directory)) != nullptr)
									{
										//Assignment File to Variable
										nameOfFile = entry->d_name;

										//Saves .info File to dummy value
										infoIndex = nameOfFile.find_last_of(".");
										dummyFile = nameOfFile.substr(0, infoIndex);

										if (dummyFile != "")
										{
											HDRFileList.push_back(nameOfFile);
										}
									}
									closedir(directory);
								}

								//Erase Lingering Directory Navigation
								for (int i = 0; i < HDRFileList.size(); i++)
								{
									if ((strcmp(removeWhiteSpace(HDRFileList.at(i)).c_str(), ".")) == 0)
									{
										HDRFileList.erase(HDRFileList.begin() + i);
									}
									if ((strcmp(removeWhiteSpace(HDRFileList.at(i)).c_str(), "..")) == 0)
									{
										HDRFileList.erase(HDRFileList.begin() + i);
									}
								}

								//Read HEADER Information from Files In Current Directory
								for (int i = 0; i < HDRFileList.size(); i++)
								{
									tempFile = removeWhiteSpace(HDRFileList.at(i));

									//File Handling
									directoryForCurrentFile = currentDirectory + "/" + tempFile;
									tempFileStream.open(directoryForCurrentFile);

									if (!tempFileStream)
									{
										cerr << "Unable to open " << tempFile << "\n";
										exit(1); //forcibly ends the program if an error occurs
									}

									//Save Header Information
									while (getline(tempFileStream, tempInput, '\n'))
									{
										if (tempInput.empty() || tempInput == " ")
										{
											break;
										}
										tempVector.push_back(tempInput); //saves HEADER information to a vector for later reference
									}

									tempFileStream.close();

									//Find Message-ID Value From the HEADER Vector, then compare to input 
									for (int j = 0; j < tempVector.size(); j++)
									{
										if (tempVector.at(j).find("Message-ID:") != string::npos)
										{
											istringstream headerLine(tempVector.at(j));
											getline(headerLine, tempTrash, ':'); //saves "trash" information to unused variable
											getline(headerLine, tempLineInput, '\n'); //saves wanted information to desired variable

											if (removeWhiteSpace(tempLineInput) == removeWhiteSpace(splitInput.at(2)))
											{
												foundMatch = true;
												foundFile = directoryForCurrentFile;
												headerInformation = tempVector;
											}
										}
									}

									//Clear Header Information
									tempVector.clear();
								}

								//Clear Information When After Each Iteration
								HDRFileList.clear();

								//If Matching Message-ID has not been found
								count++;
							}

							if (foundMatch == false) //if the message-ID is still not found after iterating through the db
							{
								serverResponse += "[S]: 430 No such article found; message-id was not found in the database.\n";
							}
							else //if the message-id is found
							{
								//Iterate Through Header Information
								for (int k = 0; k < headerInformation.size(); k++)
								{
									cout << headerInformation.at(k) << "\n";
									if (headerInformation.at(k).find(splitInput.at(1)) != string::npos)
									{
										fieldLine = headerInformation.at(k);
									}
								}

								//Process serverResponse
								serverResponse += "[S]: 225 Header information follows\n";
								serverResponse += "[S]: 0 " + fieldLine + "\n";
							}
						}
					}
					else if ((splitInput.size() == 2) && (splitInput.at(0) + " " + splitInput.at(1) == functions[6])) //LIST HEADERS Protocol
					{
						if (splitInput.size() > 2)
						{
							serverResponse += "501 Too many argument; please do not provide additional parameters\n";
						}
						else
						{
							//Variable Declaration
							string headerLine;
							string HEADERFile;
							string HEADERnameOfFile;
							string HEADERdummyFile;
							string HEADERDirectory;
							string iterationString;
							size_t infoIndex;
							fstream headerStream;
							string headerInput;
							string headerCompare;
							string foundHEADER;
							bool headerExists = false;

							vector<string> LIST_HEADERfileList;
							vector<string> tempHEADER;
							vector<string> listOfHEADERS;

							//First Statement is Always Defaulted
							serverResponse += "[S]: 215 headers:\n";

							//Search Through Each NewsGroup, and detect header information
							for (int i = 0; i < newsGroupNames.size(); i++)
							{
								//Dip Into currentlySelectedNewsGroup to read lowWaterMarkFile Information
								HEADERDirectory = dirc + newsGroupNames.at(i); //changes the directory to read fileNames

								//Generate a List of Files
								if ((directory = opendir(HEADERDirectory.c_str())) != nullptr)
								{
									while ((entry = readdir(directory)) != nullptr)
									{
										//Assignment File to Variable
										HEADERnameOfFile = entry->d_name;

										//Saves .info File to dummy value
										infoIndex = HEADERnameOfFile.find_last_of(".");
										HEADERdummyFile = HEADERnameOfFile.substr(0, infoIndex);

										if (HEADERdummyFile != "")
										{
											LIST_HEADERfileList.push_back(HEADERnameOfFile);
										}
									}
									closedir(directory);
								}

								//Erase Lingering Directory Navigation and .info File
								for (int i = 0; i < LIST_HEADERfileList.size(); i++)
								{
									if ((strcmp(removeWhiteSpace(LIST_HEADERfileList.at(i)).c_str(), ".")) == 0 ||
										(strcmp(removeWhiteSpace(LIST_HEADERfileList.at(i)).c_str(), "..")) == 0 ||
										(strcmp(removeWhiteSpace(LIST_HEADERfileList.at(i)).c_str(), ".info")) == 0)
									{
										LIST_HEADERfileList.erase(LIST_HEADERfileList.begin() + i);
									}
								}

								//Change the Directory to the currentlySelectedNewsGroup Directory
								chdir(HEADERDirectory.c_str());

								for (int j = 0; j < LIST_HEADERfileList.size(); j++)
								{
									//Change The File To Be Iterated Through
									HEADERFile = HEADERDirectory + "/" + LIST_HEADERfileList.at(j);

									//File Handling
									headerStream.open(HEADERFile);

									if (!headerStream)
									{
										cerr << "Unable to open " << HEADERFile << "\n";
										exit(1); //forcibly ends the program if an error occurs
									}

									//Save Header Information
									while (getline(headerStream, headerInput, '\n'))
									{
										if (headerInput.empty() || headerInput == " ")
										{
											break;
										}
										tempHEADER.push_back(headerInput); //saves HEADER information to a vector for later reference
									}

									//File Close
									headerStream.close();

									//Change the Directory back to the "Home" Directory where 
									chdir(homeDirc.c_str());

									//Organize Header Information
									for (int k = 0; k < tempHEADER.size(); k++)
									{
										//Find Unique Headers in the Given Header
										iterationString = tempHEADER.at(k);
										for (int l = 0; l < iterationString.size(); l++)
										{
											if (iterationString.find(':') == string::npos)
											{
												foundHEADER = "";
												continue;
											}
											else if (iterationString[l] == ':')
											{
												break;
											}
											foundHEADER += iterationString[l];
										}

										//Check To See If The Found Header Already Exists In The Generated List Of Headers
										if (listOfHEADERS.size() == 0)
										{
											listOfHEADERS.push_back(foundHEADER);
										}
										else
										{
											for (int m = 0; m < listOfHEADERS.size(); m++)
											{
												if (listOfHEADERS.at(m) == foundHEADER)
												{
													headerExists = true;
												}
											}
											if (headerExists != true && !(foundHEADER.empty()))
											{
												listOfHEADERS.push_back(foundHEADER);
												headerExists = false;
											}
										}

										//After Run With One Line, clean foundHEADER
										foundHEADER = "";
										headerExists = false;
									}
								}

								//After Each Pass Through A Single NewsGroup, clear the LIST_HEADERfileList
								LIST_HEADERfileList.clear();

							}

							//After Collecting Unique HEADERS, Fill serverResponse
							for (int n = 0; n < listOfHEADERS.size(); n++)
							{
								serverResponse += "[S]: " + listOfHEADERS.at(n) + "\n";
							}
						}
					}
					else if ((splitInput.size() == 2) && (splitInput.at(0) + " " + splitInput.at(1) == functions[7])) //LIST ACTIVE Protocol
					{
						if (splitInput.size() > 2)
						{
							serverResponse += "501 Too many argument; please do not provide additional parameters\n";
						}
						else
						{
							//Variable Declaration
							string infoLine;
							string infoDesc;
							string infoFile;
							string tempHighWaterMark;
							string tempLowWaterMark;
							string nameOfFile;
							string dummyFile;
							size_t infoIndex;
							fstream infoStream;

							vector<string> LIST_ACTIVEfileList;

							//First Statement is Always Defaulted
							serverResponse += "[S]: 215 list of newsgroups follows:\n";

							//Search Through Each NewsGroup, and add the .info information to the serverResponse
							for (int i = 0; i < newsGroupNames.size(); i++)
							{
								//Prep The Name of the File To Be Opened
								infoFile = dirc + newsGroupNames.at(i);

								//Generate a List of Files
								if ((directory = opendir(infoFile.c_str())) != nullptr)
								{
									while ((entry = readdir(directory)) != nullptr)
									{
										//Assignment File to Variable
										nameOfFile = entry->d_name;

										//Saves .info File to dummy value
										infoIndex = nameOfFile.find_last_of(".");
										dummyFile = nameOfFile.substr(0, infoIndex);

										if (dummyFile != "")
										{
											LIST_ACTIVEfileList.push_back(nameOfFile);
										}
									}
									closedir(directory);
								}

								//Erase Lingering Directory Navigation and .info File
								for (int i = 0; i < LIST_ACTIVEfileList.size(); i++)
								{
									if ((strcmp(removeWhiteSpace(LIST_ACTIVEfileList.at(i)).c_str(), ".")) == 0 ||
										(strcmp(removeWhiteSpace(LIST_ACTIVEfileList.at(i)).c_str(), "..")) == 0 ||
										(strcmp(removeWhiteSpace(LIST_ACTIVEfileList.at(i)).c_str(), ".info")) == 0)
									{
										LIST_ACTIVEfileList.erase(LIST_ACTIVEfileList.begin() + i);
									}
								}

								if (LIST_ACTIVEfileList.size() == 0)
								{
									serverResponse += "[S]: " + newsGroupNames.at(i) + " 0 0 n" + "\n";
								}
								else
								{
									//Prep File Name Information - Strip .txt extensions
									for (int i = 0; i < LIST_ACTIVEfileList.size(); i++)
									{
										//checks to see if directory navigation items are the current item, and ignores them
										if ((strcmp(removeWhiteSpace(LIST_ACTIVEfileList.at(i)).c_str(), ".")) != 0 ||
											(strcmp(removeWhiteSpace(LIST_ACTIVEfileList.at(i)).c_str(), "..")) != 0)
										{
											size_t txtIndex = LIST_ACTIVEfileList.at(i).find_last_of(".");
											LIST_ACTIVEfileList.at(i) = LIST_ACTIVEfileList.at(i).substr(0, txtIndex);
										}
									}

									//Finding High Watermarks
									tempHighWaterMark = LIST_ACTIVEfileList.at(0);
									for (int i = 0; i < LIST_ACTIVEfileList.size(); i++)
									{
										if (stoi(removeWhiteSpace(tempHighWaterMark)) < stoi(removeWhiteSpace(LIST_ACTIVEfileList.at(i))))
										{
											tempHighWaterMark = LIST_ACTIVEfileList.at(i);
										}
									}

									//Finding Low Watermark
									tempLowWaterMark = LIST_ACTIVEfileList.at(0);
									for (int i = 0; i < LIST_ACTIVEfileList.size(); i++)
									{
										if (stoi(removeWhiteSpace(tempLowWaterMark)) > stoi(removeWhiteSpace(LIST_ACTIVEfileList.at(i))))
										{
											tempLowWaterMark = LIST_ACTIVEfileList.at(i);
										}
									}

									//Add Line To serverResponse
									serverResponse += "[S]: " + newsGroupNames.at(i) + " " + tempHighWaterMark + " " + tempLowWaterMark + " n" + "\n";

									//Clear the File List For The Next Iterations
									LIST_ACTIVEfileList.clear();
								}
							}
						}
					}
					else if ((splitInput.size() == 2) && (splitInput.at(0) + " " + splitInput.at(1) == functions[8])) //LIST NEWSGROUP Protocol
					{
						if (splitInput.size() > 2)
						{
							serverResponse += "501 Too many argument; please do not provide additional parameters\n";
						}
						else
						{
							//Variable Declaration
							string infoLine;
							string infoDesc;
							string infoFile;
							fstream infoStream;

							//First Statement is Always Defaulted
							serverResponse += "[S]: 215 information follows:\n";

							//Search Through Each NewsGroup, and add the .info information to the serverResponse
							for (int i = 0; i < newsGroupNames.size(); i++)
							{
								//Prep The Name of the File To Be Opened
								infoFile = dirc + newsGroupNames.at(i) + "/.info";
								cout << infoFile << "\n";

								//Open The Desired File
								infoStream.open(infoFile);
								if (!infoStream) //checks to make sure that the directory contains a .info file
								{
									serverResponse += "[S]: " + newsGroupNames.at(i) + " - " + "No Available Description; Directory Is Empty\n";
									infoStream.close();
								}
								else
								{
									//Read The Contents of the File - It Should Always Be A Single Line
									while (getline(infoStream, infoLine, '\n'))
									{
										infoDesc = infoLine;
									}

									//Close The .info File
									infoStream.close();

									//Add Line To serverResponse
									serverResponse += "[S]: " + newsGroupNames.at(i) + " - " + infoDesc + "\n";
								}
							}
						}
					}
					else if (splitInput.at(0) == functions[9]) //HELP Protocol
					{
						//Variable Declaration
						string tempString;

						//HELP Response Confirms
						serverResponse += "[S]: 100 Help text as follows\n";

						//Server Response
						for (int i = 0; i < (sizeof(functions) / sizeof(functions[0])); i++)
						{
							tempString = functions[i];
							if (tempString == functions[0]) //GROUP Protocol Description
							{
								serverResponse += "[S]: " + tempString + " - returns current newsgroup.\n";
							}
							if (tempString == functions[1]) //LISTGROUP Protocol Description
							{
								serverResponse += "[S]: " + tempString + " - returns newsgroup + list of articles.\n";
							}
							if (tempString == functions[2]) //NEXT Protocol Description
							{
								serverResponse += "[S]: " + tempString + " - sets newgroup to next available article.\n";
							}
							if (tempString == functions[3]) //ARTICLE Protocol Description
							{
								serverResponse += "[S]: " + tempString + " - returns specified article in newsgroup.\n";
							}
							if (tempString == functions[4]) //DATE Protocol Description
							{
								serverResponse += "[S]: " + tempString + " - returns timestamp of server.\n";
							}
							if (tempString == functions[5]) //HDR Protocol Description
							{
								serverResponse += "[S]: " + tempString + " - returns fields of the header of an article.\n";
							}
							if (tempString == functions[6]) //LIST HEADERS Protocol Description
							{
								serverResponse += "[S]: " + tempString + " - returns fields available in header of articles.\n";
							}
							if (tempString == functions[7]) //LIST ACTIVE Protocol Description
							{
								serverResponse += "[S]: " + tempString + " - returns list of newsgroups + posting permissions.\n";
							}
							if (tempString == functions[8]) //NEWSGROUP Protocol Description
							{
								serverResponse += "[S]: " + tempString + " - returns list of newsgroups with descriptions.\n";
							}
							if (tempString == functions[9]) //HELP Protocol Description
							{
								serverResponse += "[S]: " + tempString + " - returns description of commands.\n";
							}
							if (tempString == functions[10]) //CAPABILITIES Protocol Description
							{
								serverResponse += "[S]: " + tempString + " - returns list of commands user can run.\n";
							}
							if (tempString == functions[11]) //QUIT Protocol Description
							{
								serverResponse += "[S]: " + tempString + " - allows client to disconnect from server.\n";
							}
						}
					}
					else if (splitInput.at(0) == functions[10]) //CAPABILITIES Protocol
					{
						//Protocol Process
						serverResponse += "[S]: 101 Capability list: \n"; //response code line
						serverResponse += "[S]: VERSION 2 \n";
						serverResponse += "[S]: READER\n";
						for (int i = 0; i < (sizeof(functions) / sizeof(functions[0])); i++)
						{
							//cout << i;
							serverResponse += "[S]: " + functions[i] + "\n";
						}

					}
					else if (splitInput.at(0) == functions[11]) //QUIT Protocol
					{
						//Server Response
						serverResponse = "[S]: 205 closing connection \n";
					}
					else //some form of input is not a legal command
					{
						//Test Case - Basic Echo
						serverResponse = "[S]: 500 unknown command; please use HELP or CAPABILITIES for a list of implemented protocols.\n";
					}

					//Send The Information that is Prepped by the Server
					serverResponse += "[S]: .\n"; //terminating octet
					new_sock << serverResponse;

					//Regardless of Outcome, Clean String/Input Information
					splitInput.clear();
					clientData = "";
					serverResponse = "";
				}
			}
		}
		catch (SocketException&) {}
	}
		
	//End of Main
	return 0;
}

//Other Functions
string readConfFilePORT(string fileName) //reads the provided .conf file and grabs PORT value
{
	//Variable Declaration
	ifstream inFile;
	string input;
	string sendPORT;    //saves the final value of the PORT number from the .conf file
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

string removeWhiteSpace(string str) //used to remove whitespace from string values
{
	str.erase(remove_if(str.begin(), str.end(), ::isspace), str.end());
	return str;
}

bool checkForNumber(const string& str) //checks whether a string is a legal number input
{
	for (char const& c : str)
	{
		if (isdigit(c) == 0)
		{
			return false;
		}
	}
	return true;
}

string getCurrentWorkingDirectory()
{
	//Variable Declaration
	char currentDirectory[256];
	string stringDirectory;

	//Find the Current Working Directory
	getcwd(currentDirectory, 256);

	//Convert Current Directory Information To String Value
	stringDirectory = currentDirectory;

	return stringDirectory;
}

vector<string> getAllNEWSGROUPNames(vector<string> temp, DIR* directory, struct dirent* entry, string dbDirectory)
{
	//Variable Declaration
	string tempString;

	//Open db Directory
	directory = opendir(dbDirectory.c_str());

	//Loop Structure To Fetch Names In The Directory
	entry = readdir(directory);
	while (entry != NULL)
	{
		if (entry->d_type == DT_DIR)
		{
			tempString = entry->d_name;
			temp.push_back(tempString);
		}
		entry = readdir(directory);
	}

	//Close db Directory
	closedir(directory);

	//Erase Lingering Directory Navigation
	for (int i = 0; i < temp.size(); i++)
	{
		if ((strcmp(removeWhiteSpace(temp.at(i)).c_str(), ".")) == 0)
		{
			temp.erase(temp.begin() + i);
		}
		if ((strcmp(removeWhiteSpace(temp.at(i)).c_str(), "..")) == 0)
		{
			temp.erase(temp.begin() + i);
		}
	}

	//Return the Vector
	return temp;
}