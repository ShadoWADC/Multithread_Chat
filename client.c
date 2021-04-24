#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>  // htons() and inet_addr()
#include <netinet/in.h> // struct sockaddr_in
#include <sys/socket.h>
#include <time.h>
#include "common.h"

struct login                           
{
    char username[30];
    char password[20];
};

struct login l;
int login (void);
void registration (void);

  
int main(int argc, char* argv[]) {
	int option;
	do{
		printf("Chat Service Menu:");
		printf("\n\t(1)Register\n\t(2)Login\n\t(0)Exit\n\n");
		printf("Select your option: ");
		scanf("%d",&option);
		while( getchar() != '\n' ){ /* flush to end of input line */ }

		
		if(option == 1)
		{
			system("clear");
			registration();
		}

		else if(option == 2)
			{
				system("clear");
				if(login() == 0)break;
			}
		else if (option == 0){
				exit(EXIT_SUCCESS);
		}	
		
	}while (option != 0);
	
	
    int ret,bytes_sent,recv_bytes;

    // variables for handling a socket
    int socket_desc;
    struct sockaddr_in server_addr = {0}; // some fields are required to be filled with 0

    // create a socket
    //socket_desc = socket(AF_INET, SOCK_DGRAM, 0); 
    socket_desc = socket(AF_INET, SOCK_STREAM, 0); 
    if(socket_desc < 0) handle_error("Could not create socket");

    // set up parameters for the connection
    server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = htons(SERVER_PORT); // don't forget about network byte order!

    // initiate a connection on the socket
    ret = connect(socket_desc, (struct sockaddr*) &server_addr, sizeof(struct sockaddr_in));
    if(ret) handle_error("Could not create connection");

    if (DEBUG) fprintf(stderr, "Connection established!\n");

    char buf[1024];
    size_t buf_len = sizeof(buf);
    int msg_len;
    memset(buf, 0, buf_len);
	
	//send username to server
	bytes_sent=0;
	ret = 0;
	
	memcpy(buf, l.username, buf_len); //Copying message into buffer
	msg_len = strlen(buf)+1;
    buf[strlen(buf)] = '\n'; // remove '\n' from the end of the message 
    printf("\n\nUsername selected <%s>\n\n",buf);
    
	while ( bytes_sent < msg_len) {
		ret = send(socket_desc, buf + bytes_sent, msg_len - bytes_sent, 0);
		if (ret == -1 && errno == EINTR) continue;
		if (ret == -1) handle_error("Cannot write to the socket");
		bytes_sent += ret;
	}
        
    memset(buf, 0, buf_len);
    recv_bytes = 0;

    // main loop
    while (1) {
        char* quit_command = SERVER_COMMAND;
        size_t quit_command_len = strlen(quit_command);
        do{
			system("clear");
			printf("Welcome to the Chat Client!\nPlease select an option:\n");
			printf("\t(1) Send a message\n");
			printf("\t(2) Check recieved messages\n");
			printf("\t(0) Exit\nSelect your option: ");
			scanf("%d",&option);
			while( getchar() != '\n' ){ /* flush to end of input line */ }
		}while(option<0||option>2);
		
		
		if (option == 0){
				exit(EXIT_SUCCESS);
		}
		else if(option == 1){ //SEND
			system("clear");
			printf("MESSAGE FORMAT = USERNAME_DEST:MESSAGE\n");
			printf("Insert your message: ");

			/* Read a line from stdin
			 *
			 * fgets() reads up to sizeof(buf)-1 bytes and on success
			 * returns the first argument passed to it. */
			if (fgets(buf, sizeof(buf), stdin) != (char*)buf) {
				fprintf(stderr, "Error while reading from stdin, exiting...\n");
				exit(EXIT_FAILURE);
			}
			
			//MESSAGE: USER_SOURCE:USER_DESTINATION:MESSAGE
			//Concatenating USER to the message Header
			char dest[1024];
			strcpy( dest, l.username );
			strcat( dest, ":");
			strcat( dest, buf);
			memcpy(buf, dest, sizeof(buf));

			msg_len = strlen(buf);
			buf[strlen(buf) - 1] = '\n'; // remove '\n' from the end of the message
			
			// send message to server
			bytes_sent=0;
			while ( bytes_sent < msg_len) {
				ret = send(socket_desc, buf + bytes_sent, msg_len - bytes_sent, 0);
				if (ret == -1 && errno == EINTR) continue;
				if (ret == -1) handle_error("Cannot write to the socket");
				bytes_sent += ret;
			}
		}
		else if(option == 2){ //RECIEVE
			
			int flags = fcntl(socket_desc, F_GETFL);
			fcntl(socket_desc, F_SETFL, flags | O_NONBLOCK);
		
			//Reading answer from SERVER
			//Message Delivered
			//USER NOT ONLINE
			// read message from server
			recv_bytes = 0;
			memset(buf, 0, buf_len);
			
			do {
				ret = recv(socket_desc, buf + recv_bytes, buf_len - recv_bytes, 0);
				if (ret == -1 && errno == EINTR) continue;
				if (ret == -1) {
						printf("Your Inbox is Empty, no incoming messages...\nPress a key to continue..\n.");
						getchar();
						flags = 999;
						break/*handle_error("Cannot read from the socket")*/;//Since is non blocking
					}
				if (ret == 0) break;
				recv_bytes += ret;				
			} while ( buf[recv_bytes-1] != '\n' );
			

			fcntl(socket_desc, F_SETFL, 0); //Setting it back to Blocking
			if(flags==999) continue; //No Inbox Case
			
			/*if(!timeout_flag)*/ printf("Server response: %s\n", buf); // no need to insert '\0'
			
			
			/* After a quit command we won't receive any more data from
			 * the server, thus we must exit the main loop. */
			if (msg_len == quit_command_len && !memcmp(buf, quit_command, quit_command_len)) break;
			
			//--------------
			//GET SENDER - USERNAME
			int len = strlen(buf);
			int semi = 0;
			int pos1 = 0, pos2 = 0;

			int start = 0; //Start of Message
			for(int i = 0; i<len; i++){
				if(buf[i]==':') {
					semi++;
					if(semi == 1){
						pos2 = i;
						
					}
				}	
			}
			
			const int size_dest = pos2-pos1+1;
			char sender[size_dest];
			int k = 0;
			for(int i = pos1; i<pos2; i++){
				sender[k] = buf[i];
				k++;
			}
			sender[size_dest-1] = '\0';


			//MESSAGE: USER_SOURCE:USER_DESTINATION:MESSAGE
			//Concatenating USER to the message Header
			if(semi>=2){
				char temp[1024];
				memcpy(temp, l.username, sizeof(temp));
				strcat( temp, ":"); 
				strcat( temp, sender);

				strcat( temp, ":"); 
				strcat( temp, "Message Read\n"); 
				memcpy(buf, temp, sizeof(buf));
				
				
				msg_len = strlen(buf);
				// send message to server
				bytes_sent=0;
				while ( bytes_sent < msg_len) {
					ret = send(socket_desc, buf + bytes_sent, msg_len - bytes_sent, 0);
					if (ret == -1 && errno == EINTR) continue;
					if (ret == -1) handle_error("Cannot write to the socket");
					bytes_sent += ret;
				}
			}

			//---------
			
			printf("Press any key to go back to the main MENU...");
			getchar();	
		}
 
    }

    // close the socket
    ret = close(socket_desc);
    if(ret) handle_error("Cannot close socket");

    if (DEBUG) fprintf(stderr, "Exiting...\n");

    exit(EXIT_SUCCESS);
}

int login (void)
{
    char username[30],password[20];
	int match = -1;
    FILE *log;

    log = fopen("login.txt","r");
    if (log == NULL)
    {
        fputs("Error at opening File!", stderr);
        exit(1);
    }

    printf("\nPlease Enter your login credentials below\n\n");
    printf("Username:  ");
    scanf("%s",username);
    while( getchar() != '\n' ){ /* flush to end of input line */ }
    printf("\nPassword: ");
    scanf("%s",password);
    while( getchar() != '\n' ){ /* flush to end of input line */ }
	
    while(fread(&l,sizeof(l),1,log))
        {
        if(strcmp(username,l.username)==0 && strcmp(password,l.password)==0)
            {   
                printf("\nLogin Successful!\n");
                match = 0;
                break;
            }
        else 
            {
                printf("\nIncorrect credentials!\nPlease enter the correct credentials\n");
            }
        }

    fclose(log);
    return match;
}


void registration(void)
{
    FILE *log;

    log=fopen("login.txt","a");
    if (log == NULL)
    {
        fputs("Error at opening File!", stderr);
        exit(1);
    }

    struct login l;

    printf("\nWelcome to the chat service, please proceed with the registration of your account.\n\n");

    printf("\nEnter Username:\n");
    scanf("%s",l.username);
    while( getchar() != '\n' ){ /* flush to end of input line */ }
    printf("\nEnter Password:\n");
    scanf("%s",l.password);
    while( getchar() != '\n' ){ /* flush to end of input line */ }


    fwrite(&l,sizeof(l),1,log);
    fclose(log);
	
	printf("\nRegistration Successful!\n");
    printf("\nWelcome, %s!\n\n",l.username);

    printf("Press any key to continue...");
    getchar();
    system("clear");
}
