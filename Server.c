#include <sys/socket.h>	//socket(), bind(), listen(), accept()
#include <stdlib.h>	//exit(), rand(), srand()
#include <unistd.h>	//send(), close(), read(), write()
#include <arpa/inet.h>	//htons()
#include <string.h>	//memset()
#include <stdio.h>	//printf()
#include <time.h>	//time()


/* This example has been modified to run accept() in a loop so that it can
   accept clients multiple times.
*/


//MAIN: server
int main (void) 
{
	int listen_fd, client_fd, err, opt, port;
	ssize_t wcount;
	socklen_t addrlen;
	struct sockaddr_in addr;
	char *msg = "Server says: connection established!";

	//Initialize random generator
	srand(time(NULL));

	//Get a random port from 10000 to 65535
	port = 10000 + (rand() % (65535 - 10000 + 1));	
	
	//Print the port (so the client will know which port to connect to)
	printf("\nListening on Port: %d\n\n", port);

	//Create a new Internet domain stream socket (TCP/IP socket)
	listen_fd = socket(AF_INET, SOCK_STREAM, 0);

	//Exit on socket() error	
	if (listen_fd == -1) {
		fprintf(stderr, "\nsocket(): exiting\n");
		exit(1);
	}

	//Set socket options (covered in textbook 61.9 and 61.10)
	//(not otherwise covered in class yet)
	//This option lets the server quickly rebind to a partly closed port
	opt = 1;
	err = setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	//Set addr struct to 0 bytes (to zero out fields we're not using)
	memset(&addr, 0, sizeof(struct sockaddr_in));

	//Set 3 addr structure members
	addr.sin_family = AF_INET;		//addr type is Internet (IPv4)
	addr.sin_port = htons(port);		//convert port to network byte ordering
	addr.sin_addr.s_addr = INADDR_ANY;	//bind to any local address

	//Store the structure size
	addrlen = sizeof(addr);

	//Bind socket
	err = bind(listen_fd, (struct sockaddr *)&addr, addrlen);

	//Exit on bind() error
	if (err == -1) {
		close(listen_fd);
		fprintf(stderr, "\nbind(): exiting\n");
		exit(2);
	}

	//Begin listening for connections with a max backlog of 1
	//(backlog is very low to demonstrate dropped connection attempts)
	err = listen(listen_fd, 32);

	//Exit on listen() error
	if (err == -1) {
		close(listen_fd);
		fprintf(stderr, "\nlisten(): exiting\n");
		exit(3);
	}

	printf("\nWaiting for client to connect ...\n\n");

	while(1)
	{
		//Accept one client connection
		client_fd = accept(listen_fd, NULL, NULL);

		//Print a notice if an error occurs
		if (client_fd <= 0)
			fprintf(stderr, "\naccept(): exiting\n");
	

		//Send a message to the client immediately after connecting
		wcount = write(client_fd, msg, strlen(msg));

		//Report if a write error occured
		if (wcount < (int)strlen(msg)){
			fprintf(stderr, "\nwrite(): exiting\n");
			exit(5);
		}

		close(client_fd);
		printf("Client connected; message sent; closed socket; continue to listen\n\n");
	}

	//Exit normally by closing sockets
	printf("Exiting\n\n");
	close(listen_fd);
	exit(EXIT_SUCCESS);
}
