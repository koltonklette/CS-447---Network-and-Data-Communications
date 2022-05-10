//Kolton Klette
//Thoshita Gamage
//CS-447 - Network Communications
//Februrary 22nd, 2022

Compilation Instructions:
Once the directory has been accessed in a working Linux environment (zone.cs.siue.edu container or otherwise), the 'Makefile.txt' can be called (while in the same directory as it) using the command 'make -f Makefile.txt.', which
will create the 'server' and 'client' executables that are run for the project. Each executable should be placed in their own container environment to facilitate a proper socket connection, and each executable should have their
respective .conf file in the same directory as they are being run (e.g server executable should be put in the same directory as the server.conf file and the client.conf file should be put in the same directory as the client executable).
The .conf files provided can be changed to include any available PORT number, but the IP value for the client should match that of the container's IP that the server executable is running on (considering testing is done specifically
in the zone.cs.siue.edu server).

Execution:
Once set up and compilation have been completed, a list of commands as illustrated in the report can be run on the client side once the server has been executed (./server suffices), followed by the execution of any number of concurrent
./client calls in a separate container. From there, the client can be interacted with by calling any of the instantiated protocol calls as outlined in the provided report.
