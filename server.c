#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>  // htons()
#include <netinet/in.h> // struct sockaddr_in
#include <sys/socket.h>
#include <semaphore.h>
#include "common.h"
#include <pthread.h>

/*
#include <sys/wait.h>
#include <fcntl.h>
#include <semaphore.h>


#define CRITICAL_SECTION "/critical_section"
sem_t *critical_section = NULL;
*/

typedef struct handler_args_s
{
    /** SOLUTION
     *
     * Specify fields for the arguments that will be populated in the
     * main thread and then accessed in connection_handler(void* arg).
     **/
    int socket_desc;
    struct sockaddr_in* client_addr;
} handler_args_t;

struct users_online_t                           
{
	int socket_desc[MAX_USERS]; //SOCKET DESCRIPTORS
    char list[MAX_USERS][30]; //USERNAME
    int online; //NUMBER OF USERS ONLINE
    char client_ip[MAX_USERS][INET_ADDRSTRLEN]; //IP ADDRESS 
	uint16_t client_port[MAX_USERS];	//PORT
};
struct users_online_t users;

void printOnlineUsers(int id){
	system("clear");
	printf("Welcome to the Chat Service 1.0\n");
	printf("Online Users List(%d/%d)\n", users.online, MAX_USERS);
	
	for(int i = 0; i<users.online ; i++){
		printf("ID:%d\tUsername:%s\tClient:%s\tPort:%hu\tDescriptor: %d\n", i, users.list[i], users.client_ip[i], users.client_port[i], users.socket_desc[i]);	
	}
}

int getSenderDescriptor(char * msg){
	int l = strlen(msg);
	
	//Check Format
	int semi = 0;
	int pos1 = 0, pos2 = 0; //Pos of Destinatario 
	int start = 0; //Start of Message
	for(int i = 0; i<l; i++){
		if(msg[i]==':') {
			semi++;
			if(semi == 1){
				pos2 = i;
				break;
			}
		}	
	}
		
	const int size_dest = pos2-pos1/*+1*/;
	char sender [size_dest];
	int k = 0;
	//printf("\nDESTINATARIO: ");
	for(int i = pos1; i<pos2; i++){
		sender[k] = msg[i];
		//printf("%c", destinatario[k]);
		k++;
	}
	
	for(int i = 0; i<users.online; i++){
		if(!memcmp(users.list[i], sender, sizeof(sender))) return users.socket_desc[i];
	}
	
	return -3; //USER NOT ONLINE
}

int isDeliverable(char * msg){
	int l = strlen(msg);
	if(l<7) return -1; //Test lunghezza minima package
	
	//Check Format
	int semi = 0;
	int pos1 = 0, pos2 = 0; //Pos of Destinatario 
	int start = 0; //Start of Message
	for(int i = 0; i<l; i++){
		if(msg[i]==':') {
			semi++;
			if(semi == 1)pos1 = i+1;
			if(semi==2){ 
				start = i+1;
				pos2 = i;
			}
		}	
	}
	if(semi<2) return -2;
	

	
	const int size_dest = pos2-pos1;
	char destinatario [size_dest];
	int k = 0;
	for(int i = pos1; i<pos2; i++){
		destinatario[k] = msg[i];
		k++;
	}
	
	const int size_data = l-pos2;
	char data[size_data];
	k = 0;
	
	//printf("\nRIGAPERRIGA:\n");
	for(int i = pos2+1; i<l; i++){
		data[k] = msg[i];
		//printf("#%d<%c>\n",k,data[k]);
		k++;
	}
	data[size_data-1] = '\0';
		
	
	size_t temp = strlen("Message Read\n");
	int descriptor = 0;
	/*printf("\nIL MESSAGGIO ESTRATTO E' <%s>",data);
	printf("\nStrlen:%ld\nSizeof:%zu\n",strlen(data),sizeof(data));
	printf("\nVS\n");
	printf("\MESSAGE:<%s>","Message Read\n");
	printf("\nStrlen:%ld\nSizeof:%zu\n",strlen("Message Read\n"),sizeof("Message Read\n"));*/
	
	if (strlen(data) == temp && !memcmp(data, "Message Read\n", temp)) {
		//printf("\nSIAMO NELL IF DEL DELIVERED\n");
		descriptor = 1000;	
	}

	//Search in connected USERS:
	if (users.online==1) return -3; //Online da solo -> USER NOT ONLINE
	
	for(int i = 0; i<users.online; i++){
		if(!memcmp(users.list[i], destinatario, sizeof(destinatario))) return descriptor+users.socket_desc[i];
	}
	
	return -3; //USER NOT ONLINE
}

void connection_handler(int socket_desc, struct sockaddr_in* client_addr) {
    int ret, recv_bytes;
	int id;
	
    char buf[1024];
    size_t buf_len = sizeof(buf);
    int msg_len;

    char* quit_command = SERVER_COMMAND;
    size_t quit_command_len = strlen(quit_command);

    // parse client IP address and port
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client_addr->sin_addr), client_ip, INET_ADDRSTRLEN);
    uint16_t client_port = ntohs(client_addr->sin_port); // port number is an unsigned short
	
	//recieve username
	// read message from client
	memset(buf, 0, buf_len);
	recv_bytes = 0;
	do {
		ret = recv(socket_desc, buf + recv_bytes, 1, 0);
		if (ret == -1 && errno == EINTR) continue;
		if (ret == -1) handle_error("Cannot read from the socket");
		if (ret == 0) break;
	} while ( buf[recv_bytes++] != '\n' );
	
	buf[strlen(buf) - 1] = '\0';
	
	/*
	// enter critical section
    ret = sem_wait(critical_section);
    if(ret) {
        handle_error("sem_wait failed");
	}
    */
    
	//Critical Section
	memcpy(users.list[users.online], buf, sizeof(users.list[users.online])); //Copy message recieved to username list
	memcpy(users.client_ip[users.online], client_ip, sizeof(users.client_ip[users.online])); 
	users.client_port[users.online] = client_port;
	id = users.online;
	users.online++;
	//Critical Section
	printOnlineUsers(id);
	
	/*
	// exit critical section
    ret = sem_post(critical_section);
	if(ret) {
	    handle_error("sem_post failed");
	}
	*/
	
	//WELCOME
	
	buf_len = sizeof(buf);
	memset(buf, 0, buf_len); //TO CHECK 
    // send custom welcome message - and print user connected on the server thread
    /*sprintf(buf, "Hi! I'm an echo server. You are %s talking on port %hu.\nI will send you back whatever"
            " you send me. I will stop if you send me %s :-)\n", client_ip, client_port, quit_command);
    msg_len = strlen(buf);
    int bytes_sent = 0;
	while ( bytes_sent < msg_len) {
        ret = send(socket_desc, buf + bytes_sent, msg_len - bytes_sent, 0);
        if (ret == -1 && errno == EINTR) continue;
        if (ret == -1) handle_error("Cannot write to the socket");
        bytes_sent += ret;
    }*/
	
	int bytes_sent = 0;
	//First value does not arrive
    // echo loop
    while (1) {
        // read message from client
        memset(buf, 0, buf_len);
        recv_bytes = 0;
        do {
            ret = recv(socket_desc, buf + recv_bytes, 1, 0);
            if (ret == -1 && errno == EINTR) continue;
            if (ret == -1) handle_error("Cannot read from the socket");
            if (ret == 0) break;
		} while ( buf[recv_bytes++] != '\n' );
		
	
        // check whether I have just been told to quit...
        if (recv_bytes == 0) break;
        if (recv_bytes == quit_command_len && !memcmp(buf, quit_command, quit_command_len)) break;
		printf("%s->%s", users.list[id], buf);
        // ... or if I have to send the message back
        
        
        
        //Delivering message to reciever
        //Searching for User
        int test = isDeliverable(buf); //if > returns SOCKET Descriptor of Dest, otherwise error
        //printf("\nResult of test = %d\n",test);
        
        if(test > 0 && test <1000){ //MSG is Fine and USER is Online - Sendint to DESTINATARIO
			bytes_sent=0;
			while ( bytes_sent < recv_bytes) {
				ret = send(test, buf + bytes_sent, recv_bytes - bytes_sent, 0);
				if (ret == -1 && errno == EINTR) continue;
				if (ret == -1) handle_error("Cannot write to the socket");
				bytes_sent += ret;
			}
			sprintf(buf, "Message Delivered\n");
		} 
        else if(test == -3) sprintf(buf, "User not ONLINE\n");
        else sprintf(buf, "Format of the MESSAGE Incorrect\n");
    
		
        //Answer to Sender "DELIVERED or USER NOT CONNECTED"
        msg_len = strlen(buf);
		bytes_sent = 0;
		while ( bytes_sent < msg_len) {
			ret = send(socket_desc, buf + bytes_sent, msg_len - bytes_sent, 0);
			if (ret == -1 && errno == EINTR) continue;
			if (ret == -1) handle_error("Cannot write to the socket");
			bytes_sent += ret;
		}
        
        
        //ORIGINAL
        /*bytes_sent=0;
        while ( bytes_sent < recv_bytes) {
            ret = send(socket_desc, buf + bytes_sent, recv_bytes - bytes_sent, 0);
            if (ret == -1 && errno == EINTR) continue;
            if (ret == -1) handle_error("Cannot write to the socket");
            bytes_sent += ret;
        }*/
       
        
    }
    // close socket
    ret = close(socket_desc);

    if (ret) handle_error("Cannot close socket for incoming connection");
}

// Wrapper function that take as input handler_args_t struct and then call 
// connection_handler.
void *thread_connection_handler(void *arg) {
    handler_args_t *args = (handler_args_t *)arg;
    int socket_desc = args->socket_desc;
    struct sockaddr_in *client_addr = args->client_addr;
    connection_handler(socket_desc,client_addr);
    // do not forget to free client_addr! by design it belongs to the
    // thread spawned for handling the incoming connection
    free(args->client_addr);
    free(args);
    pthread_exit(NULL);
}

void mthreadServer(int server_desc) {
    int ret = 0;
	printf("Server ready and waiting to accept connections!\n");
	
    // loop to manage incoming connections spawning handler threads
    int sockaddr_len = sizeof(struct sockaddr_in);
    while (1) {
        // we dynamically allocate a fresh client_addr object at each
        // loop iteration and we initialize it to zero
        struct sockaddr_in* client_addr = calloc(1, sizeof(struct sockaddr_in));
        
        // accept incoming connection
        int client_desc = accept(server_desc, (struct sockaddr *) client_addr, (socklen_t *)&sockaddr_len);
        
        if (client_desc == -1 && errno == EINTR) continue; // check for interruption by signals
        if (client_desc < 0) handle_error("Cannot open socket for incoming connection");
        
        if (DEBUG) fprintf(stderr, "Incoming connection accepted...\n");

        /** SOLUTION
         *
         * Suggestions:
         * - connection_handler(void* arg) is the method to execute in
         *   the thread we are going to spawn
         * - allocate a handler_args_t data structure and populate it
         *   with the arguments that will be passed to the thread
         * - print a debug message to inform the user that a new thread
         *   has been created to handle the request
         * - since we don't need to pthread_join(), release libpthread's
         *   internal resources after pthread_create()
         * - to accept a new connection while a thread is executing, be
         *   careful: resetting fields in client_addr won't work!
         **/

        pthread_t thread;

        // prepare arguments for the new thread
        handler_args_t *thread_args = malloc(sizeof(handler_args_t));
        thread_args->socket_desc = client_desc;
        thread_args->client_addr = client_addr;
        users.socket_desc[users.online] = client_desc; //EDIT
        ret = pthread_create(&thread, NULL, thread_connection_handler, (void *)thread_args);
        if (ret) handle_error_en(ret, "Could not create a new thread");
        
        if (DEBUG) fprintf(stderr, "New thread created to handle the request!\n");
        
        ret = pthread_detach(thread); // I won't phtread_join() on this thread
        if (ret) handle_error_en(ret, "Could not detach the thread");
    }
}

int main(int argc, char* argv[]) {
    int ret;
	
	/*
	// critical section named semaphore
	sem_unlink(CRITICAL_SECTION);
    critical_section = sem_open(CRITICAL_SECTION, O_CREAT|O_EXCL, 0600, 1);
	if (critical_section == SEM_FAILED) {
		handle_error("Error creating named semaphore");
	}
	*/	
    
    
    int socket_desc;

    // some fields are required to be filled with 0
    struct sockaddr_in server_addr = {0};

     // we will reuse it for accept()

    // initialize socket for listening
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc < 0) handle_error("Could not create socket");

    server_addr.sin_addr.s_addr = INADDR_ANY; // we want to accept connections from any interface
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = htons(SERVER_PORT); // don't forget about network byte order!

    /* We enable SO_REUSEADDR to quickly restart our server after a crash:
     * for more details, read about the TIME_WAIT state in the TCP protocol */
    int reuseaddr_opt = 1;
    ret = setsockopt(socket_desc, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_opt, sizeof(reuseaddr_opt));
    if (ret) handle_error("Cannot set SO_REUSEADDR option");

    // bind address to socket
    ret = bind(socket_desc, (struct sockaddr*) &server_addr, sizeof(struct sockaddr_in));
    if (ret) handle_error("Cannot bind address to socket");

    // start listening
    ret = listen(socket_desc, MAX_CONN_QUEUE);
    if (ret) handle_error("Cannot listen on socket");

    mthreadServer(socket_desc);

    exit(EXIT_SUCCESS); // this will never be executed
}
