Simple Multiprocess Server </br>
-------------------------------


This is a quick and small implementation to how Interprocess communication takes place. The project contains a client and a server where a server once started is always up and running to accept requests. The client can send a request to the server which gets fulfilled and the server returns a response. Multiple clients can access the server at once.

<h4>What does it do ?</h4> </br>
The server and clients communicate through POSIX message queues. The server consists of resources (files) which are requested by clients. The server boots up by opening a server queue and keeps listening to requests. A client requesting for a resource boots up and creates his own client queue. It sends its request to the server which includes the client_queues name (unique name), filename to be accessed and keyword. The server scans through the file and sends/writes back all matching lines (to the keyword) to the client queue. For each client request the server would fork a new process to handle the request. The child process is killed once its job is one.

<h4>Directions to run:</h4></br>
1. Clone the repo (Only Linux machine as it uses few Linux header files)
2. Since make files is already available straight away run the following on a Linux terminal (cd into the project root directory ... /Simple-Multiprocessor)
    
    --> ./server_mf /sq
    --> ./client_mf /sq f1 the

Note: For creating/compiling the makefiles yourself

--> gcc server.c -o server_mf -lrt</br>
--> gcc client.c -o client_mf -lrt
    
References: 
<a href="http://www.cs.bilkent.edu.tr/~korpe/sites/cs342spring2014/lib/exe/fetch.php?media=internal:project1.pdf">Link</a>
