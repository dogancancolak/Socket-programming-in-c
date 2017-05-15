#include<stdio.h>
#include<string.h>    // for strlen
#include<sys/socket.h>
#include<arpa/inet.h> // for inet_addr
#define localhost "127.0.0.1"
#define port_no 5575

int socket_desc;
struct sockaddr_in server;
char message[2000], server_reply[2000];
char name[200];

void *sender(){
    while (1) {
        printf("Input > ");
        fgets(message,2000,stdin);
        if (send(socket_desc, message, strlen(message)+1, 0) < 0) {
            puts("Send failed");
            return 1;
        }
        puts("Data sent...");
    }
}

void *receiver(){
    int read_size;
    while ((read_size = recv(socket_desc, server_reply, 2000, 0)) >0) {
        puts(server_reply);
        printf("Input > ");
    }
    if(read_size == 0)
    {
        puts("Server disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }
}

void main() {


    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1) {
        puts("Could not create socket");
        return 1;
    }
    server.sin_addr.s_addr = inet_addr(localhost);
    server.sin_family = AF_INET;
    server.sin_port = htons(port_no);


    if (connect(socket_desc, (struct sockaddr *) &server, sizeof(server)) < 0) {
        puts("Connection error");
        return 1;
    }
    puts("Connected to localhost...");

    pthread_t receiver_thread;
    pthread_t sender_thread;

    pthread_create(&receiver_thread, NULL, receiver, NULL);
    pthread_create(&sender_thread, NULL, sender, NULL);

    pthread_join(receiver_thread,NULL);
    pthread_join(sender_thread,NULL);

}

