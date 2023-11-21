#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <signal.h>
#include <poll.h>

#define TIME_OUT 10
#define PORT 8080
#define MAX_BUFFER_SIZE 1024


struct DataPacket
{
	int MSS;			// Maximum segment size
	int seq_no;
	int ack_no;
	int connected;
	char type;
};


int main() 
{
    int serverSocket, clientSocket, seq_no = 0, ack_no = 0,seg_size = 0, ack_sent = 0, time = 0, connected = 0;
    struct sockaddr_in serverAddr, clientAddr;
    struct DataPacket Packet;
    socklen_t addrSize = sizeof(clientAddr);
    ssize_t bytes_received = 0, bytes_sent = 0;

    // Creating the socket :

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) 
    {
        perror("Error in creating socket.");
        exit(EXIT_FAILURE);
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    // Binding of the socket :

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) 
    {
        perror("Error in binding the socket.");
        exit(-1);
    }

    // Listen for incoming connections :

    if (listen(serverSocket, 5) == -1) 
    {
        perror("Error in listening for senders.");
        exit(-1);
    }

    printf("Server listening on port %d for the senders ...\n", PORT);
    
    // Accept the connection from the client :

    clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &addrSize);
    if (clientSocket == -1) 
    {
        perror("Error in accepting connection.");
        exit(EXIT_FAILURE);
    }

    printf("Connection accepted.\n");

    // Receiving and Sending the HandShake packet for Connection establishment for Data Transfer : 
  
    bytes_received = read(clientSocket, &Packet, sizeof(Packet));			
    if (bytes_received <= 0) 
    {
	printf("Connection closed by sender.\n");
        exit(-1);
    }
    
    Packet.connected = 0;

    /*--------------------------------------------------- CONNECTION ESTABLISHMENT -----------------------------------------------------------*/

    while (!Packet.connected)
    {
	    if (Packet.type == 'E')								// Received request for transfer
	    {
		printf("Sequence number received : %d\n", Packet.seq_no);
		Packet.ack_no = Packet.seq_no + 1;
		ack_sent = Packet.ack_no;
		printf("Enter a sequence number for handshaking : ");
		scanf("%d",&Packet.seq_no);
		seg_size = Packet.MSS;
		bytes_sent = write(clientSocket, &Packet, sizeof(Packet));			// Sending acknowlegment for transfer
		printf("Acknowlegement sent : %d\n", ack_sent);
		struct pollfd poll_fd;
	    	poll_fd.fd = clientSocket;
	    	poll_fd.events = POLLIN;
		time = TIME_OUT * 1000;
    		int poll_result = poll(&poll_fd, 1, time); // TIME_OUT is in seconds, poll expects milliseconds

    		if (poll_result == -1)
    		{
		        perror("Poll error");
		        exit(EXIT_FAILURE);
		}
	        else if (poll_result == 0)
    		{
		        printf("\nTimeout occurred. NO acknowlegment Received.\n");
			exit(-1);
    		}
    		else
    		{
		        if (poll_fd.revents & POLLIN)
        		{
				bytes_received = read(clientSocket, &Packet, sizeof(Packet));
				time = TIME_OUT * 1000;
				if (bytes_received <= 0) 
			    	{
					printf("Connection closed by sender.\n");
					exit(-1);
			    	}

			    	if (Packet.type == 'E')								// Checking if acknowleged ?
			    	{
					if (Packet.seq_no == ack_sent)							// YES
					{
						printf("\nAcknowlegement received : %d\n", Packet.ack_no);
						printf("\nConnection Established for Data Transfer....\n");
						Packet.connected = 1;
						bytes_sent = write(clientSocket, &Packet, sizeof(Packet));
						sleep(5);
						break;
					}
				}
			}
		}		
	}
	
    }
	
	
   /*----------------------------------------------------------- DATA TRANSFER ----------------------------------------------------------------------------*/

   // Displaying the acknowlegement number after receiving the data : 

    struct pollfd poll_fd;
    poll_fd.fd = clientSocket;
    poll_fd.events = POLLIN;
    time = TIME_OUT * 1000;
    int poll_result = poll(&poll_fd, 1, time); // TIME_OUT is in seconds, poll expects milliseconds

    if (poll_result == -1)
    {
        perror("Poll error");
        exit(EXIT_FAILURE);
    }
    else if (poll_result == 0)
    {
        printf("Timeout occurred.\n");
	exit(-1);
    }
    else
    {
        if (poll_fd.revents & POLLIN)
        {
		bytes_received = read(clientSocket, &Packet, sizeof(Packet));
		if (bytes_received <= 0) 
	    	{
			printf("Connection closed by sender.\n");
			exit(-1);
	    	}

	    	Packet.ack_no = Packet.seq_no + seg_size;
		ack_sent = Packet.ack_no; 
	    	printf("\nSequence number : %d\n",Packet.seq_no);
	    	printf("Acknowlegment sent : %d\n",Packet.ack_no);
	    	bytes_sent = write(clientSocket, &Packet, sizeof(Packet));
		sleep(2);					// Waiting for the sender to send the data 
	}
    }
	 
    while (1) 
    {
	struct pollfd poll_fd;
    poll_fd.fd = clientSocket;
    poll_fd.events = POLLIN;
    time = TIME_OUT * 1000;

    int poll_result = poll(&poll_fd, 1, time ); // TIME_OUT is in seconds, poll expects milliseconds

    if (poll_result == -1)
    {
        perror("Poll error");
        exit(EXIT_FAILURE);
    }
    else if (poll_result == 0)
    {
        printf("\nTimeout occurred. Resending the Acknowlegment.\n");
	printf("Acknowlegment sent : %d\n", ack_sent);
	bytes_sent = write(clientSocket, &Packet, sizeof(Packet));
    }
    else
    {
        if (poll_fd.revents & POLLIN)
        {
		bytes_received = read(clientSocket, &Packet, sizeof(Packet));
		alarm(0);
		if (bytes_received <= 0) 
		{
		    printf("Connection closed by sender.\n");
		    break;
		}

		if (Packet.type != 'D')
		{
			printf("Connection terminated by sender.\n");
			exit(-1);
		}

		if (Packet.seq_no == ack_sent)			// Received the packet
		{
			Packet.ack_no = Packet.seq_no + seg_size;
			ack_sent = Packet.ack_no;
			Packet.type = 'D';
			printf("\nSequence number : %d\n",Packet.seq_no);
			printf("Acknowlegment sent : %d\n", Packet.ack_no);
			bytes_sent = write(clientSocket, &Packet, sizeof(Packet));
		}
	}
    }

		
    }

    // Closing the sockets :

    close(clientSocket);
    close(serverSocket);

    return 0;
}
