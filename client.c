#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <netinet/in.h>
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


int main(int argc, char *argv[]) 
{
    int clientSocket, seg_no = 0,seg_size = 0, seq_sent = 0, time = 0, starting_seq = 0;
    struct sockaddr_in serverAddr, clientAddr;
    struct DataPacket Packet;
    socklen_t addrSize = sizeof(clientAddr);
    ssize_t bytes_received = 0, bytes_sent = 0;

    // Creating the socket :

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == -1) 
    {
        perror("Error in creating socket.");
        exit(EXIT_FAILURE);
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(argv[1]);
    serverAddr.sin_port = htons(PORT);

    // Connecting to the receiver : 

    connect(clientSocket,(struct sockaddr *)&serverAddr,sizeof(serverAddr));
    printf("Connected to the receiver...\n");

    // Sending and Receiving the HandShake packet for Connection establishment for Data Transfer : 
  
    printf("\nEnter the Segment size : ");
    scanf("%d",&Packet.MSS);
    seg_size = Packet.MSS;
    printf("Enter the sequence number of handshaking : ");
    scanf("%d", &Packet.seq_no);
    seq_sent = Packet.seq_no;
    Packet.type = 'E';
    bytes_sent = write(clientSocket, &Packet, sizeof(Packet));					// Reqesting for data transfer
    printf("\nSequence number : %d\n",seq_sent);
    Packet.connected = 0;

    /*--------------------------------------------------- CONNECTION ESTABLISHMENT -----------------------------------------------------------*/

    while (!Packet.connected)
    {
		struct pollfd poll_fd;
		poll_fd.fd = clientSocket;
		poll_fd.events = POLLIN;
		time = TIME_OUT * 1000;
		int poll_result = poll(&poll_fd, 1, time); 

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
			    alarm(0);
			}
		}	
		    
		 if (Packet.type == 'E' && Packet.ack_no == seq_sent + 1)		// Received acknowlegment for the request for transfer
		    {							
				printf("Acknowlegment received : %d\n", Packet.ack_no);
				Packet.ack_no = Packet.seq_no + 1;
				Packet.seq_no = seq_sent + 1;
				bytes_sent = write(clientSocket, &Packet, sizeof(Packet));			// Sending the acknowlegment for receiver's packet
				printf("Acknowlegment sent : %d\n",Packet.ack_no);
				
				struct pollfd poll_fd;
				poll_fd.fd = clientSocket;
			    	poll_fd.events = POLLIN;

				int poll_result = poll(&poll_fd, 1, TIME_OUT * 1000); 

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
						if (Packet.connected == 1) 
						{
							printf("\nConnection Established for Data Transfer....\n");	
							break;
						}
					}
			    	}
			}	
    }
	
   /*----------------------------------------------------------- DATA TRANSFER ----------------------------------------------------------------------------*/

   // Displaying the segment number after getting packet number from user :

   printf("\nEnter the starting packet's sequence number : ");
   scanf("%d",&starting_seq);
   
   Packet.seq_no = starting_seq;
   seq_sent = Packet.seq_no;
   Packet.type = 'D';
   seg_no = ((Packet.seq_no - starting_seq)/seg_size)+1;
   printf("\nSequence number : %d.\n",Packet.seq_no);
   printf("Segment Number : %d.\n",seg_no);
   bytes_sent = write(clientSocket, &Packet, sizeof(Packet));			// Sending a data Packet
   sleep(2);

    while (1) 
    {
	    struct pollfd poll_fd;
	    poll_fd.fd = clientSocket;
	    poll_fd.events = POLLIN;
	    time = TIME_OUT * 1000;
	    int poll_result = poll(&poll_fd, 1, time); 

	    if (poll_result == -1)
	    {
		perror("Poll error");
		exit(EXIT_FAILURE);
	    }
	    else if (poll_result == 0)
	    {
		printf("\nTimeout occurred. Resending the packet.\n");
		printf("Sequence number : %d\n", seq_sent);
		bytes_sent = write(clientSocket, &Packet, sizeof(Packet));
		continue;
	    }
	    else
	    {
		if (poll_fd.revents & POLLIN)
		{
			bytes_received = read(clientSocket, &Packet, sizeof(Packet));
			alarm(0);
			if (bytes_received <= 0) 
			{
			    printf("Connection closed by receiver.\n");
			    break;
			}

			if (Packet.type != 'D')
			{
				printf("Connection terminated by receiver.\n");
				exit(-1);
			}
			
			if (Packet.ack_no == seq_sent + seg_size)			// Acknowlegment received
			{
			   	Packet.type = 'D';
				printf("\nAcknowlegment number : %d.\n",Packet.ack_no);
				printf("\nEnter the data to be sent : ");
				scanf("%d",&Packet.seq_no);
				seq_sent = Packet.seq_no;
				seg_no = ((Packet.seq_no - starting_seq)/seg_size)+1;
			   	printf("\nSequence number : %d.\n",Packet.seq_no);
			   	printf("Segment Number : %d.\n",seg_no);
				bytes_sent = write(clientSocket, &Packet, sizeof(Packet));		// Sending the data packet
			}
			else									// Send requested packet
			{
				Packet.type = 'D';
				printf("\nAcknowlegment number : %d.\n",Packet.ack_no);
				seq_sent = Packet.ack_no;
				Packet.seq_no = seq_sent;
				seg_no = ((Packet.seq_no - starting_seq)/seg_size)+1;
			   	printf("\nSequence number : %d.\n",Packet.seq_no);
			   	printf("Segment Number : %d.\n",seg_no);
				bytes_sent = write(clientSocket, &Packet, sizeof(Packet));
			}
		}
	    }

    	}

    // Closing the socket :

    close(clientSocket);

    return 0;
}
