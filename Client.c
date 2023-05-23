#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define LENGTH 2048
#define USERBUFFER 35
#define TEMP_PORT 200001

void stringOverwriteStdout(void);
void trimString (char* arr, int length);
void catchExitHandler(int sig);
void sendMsgHandler(void);
void recvMsgHandler(void);
int escapeCharHandler(const char message[]);


// Global variables
volatile sig_atomic_t flag = 0;
int listen_fd = 0;
char username[32];

//flush output buffer and indicate newline in terminal
void stringOverwriteStdout(void) 
{
  printf("%s", "> ");
  fflush(stdout);
}

//trim the string to remove \n from input
void trimString (char* arr, int length) 
{
  int i;
  for (i = 0; i < length; i++) 
  { // trim \n
    if (arr[i] == '\n') {
		arr[i] = '\0';
		break;
    }
  }
}

//set flag on any kind of exit
void catchExitHandler(int sig) 
{
	flag = 1;
	printf("\nBye\n");
	close(listen_fd);
	exit(EXIT_SUCCESS);
}

//Handles the client messages to send and checks for escape character \ at the beggining
void sendMsgHandler(void) 
{
	char message[LENGTH] = {};
	char buffer[LENGTH + USERBUFFER] = {};

	//infinite loop for client to send new messages
  	while(1) {
  		stringOverwriteStdout();
    	fgets(message, LENGTH, stdin);
    	trimString(message, LENGTH);

		//checks for escape character at begging of input
		if (strcspn(message, "\\") == 0) 
		{
			//calls escapeCharHandler to get a int to see what special command to do
			if(escapeCharHandler(message) == 1)
			{
				catchExitHandler(2);
			}
			else
			{
				//to check if input was empty or not so it doesn't give a false error to client
				if(strlen(message) != 0)
				{
					//ignores everthing after the invalid command
					printf("%s: is not a valid command\n", strtok(message, " "));
				}
			}
		} 
		else 
		{
			sprintf(buffer, "<%s> %s\n", username, message);
			send(listen_fd, buffer, strlen(buffer), 0);
		}

		bzero(message, LENGTH);
		bzero(buffer, LENGTH + USERBUFFER);
	}
}

//Handles the client messages being recieved
void recvMsgHandler(void) 
{
	char message[LENGTH] = {};
	
	//infinite loop to recieve new messages
	while (1) 
	{
		int receive = recv(listen_fd, message, LENGTH, 0);
		if (receive > 0) 
		{
			printf("%s", message);
			stringOverwriteStdout();
		} else if (receive == 0) 
		{
			//catchExitHandler(2);
			break;
		}
		memset(message, 0, sizeof(message));
	}
}

//Handler for identify any escape sequences for special commands
int escapeCharHandler(const char message[])
{
	//printf(message);
	//printf(message[1]);
	if(message[1] == 'q')
	{
		return 1;
	}
	return 0;
}

int main(int argc, char **argv)
{
    int port = TEMP_PORT;
	int err;
	struct sockaddr_in server_addr;
	pthread_t send_msg_thread;
	pthread_t recv_msg_thread;
	

	signal(SIGINT, catchExitHandler);

	printf("Please enter your username: ");
    fgets(username, 32, stdin);
    trimString(username, strlen(username));

	//makes sure username it the correct length
	if (strlen(username) > 32 || strlen(username) < 2)
	{
		printf("username must be less than 30 and more than 2 characters.\n");
		return EXIT_FAILURE;
	}

	// Socket settings
	listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(port);


  // Connect to Server
	err = connect(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if (err == -1) 
	{
		printf("ERROR: connect\n");
		return EXIT_FAILURE;
	}

	// Send username
	send(listen_fd, username, 32, 0);

	//create the client's sending thread 
	if(pthread_create(&send_msg_thread, NULL, (void *) sendMsgHandler, NULL) != 0)
	{
		printf("ERROR: pthread\n");
		return EXIT_FAILURE;
	}

	//create the client's recieve thread 
	if(pthread_create(&recv_msg_thread, NULL, (void *) recvMsgHandler, NULL) != 0)
	{
		printf("ERROR: pthread\n");
		return EXIT_FAILURE;
	}

	printf("=== WELCOME TO THE CHATROOM ===\n");

	while (1)
	{
		//check if flag is set to close out
		if(flag)
		{
			break;
    	}
	}
}