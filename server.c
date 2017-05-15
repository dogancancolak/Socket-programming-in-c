#include<stdio.h>
#include<string.h>    // for strlen
#include<stdlib.h>
#include<sys/socket.h>
#include<arpa/inet.h> // for inet_addr
#include<unistd.h>    // for write
#include<pthread.h>   // for threading, link with lpthread

struct Person {
    char name[15];
    int socket;
};
struct group {
    char group_name[20];
    char* members[15];
    int count;
};
int flag = 0;

struct Person Accounts[100];
struct group Groups[20];
int group_counter = 0;
int account_counter = 0;

int parse(char **parsedInput,char *string, char *parseType){//can be changed, in order of OUTPUT, INPUT, PARSING TYPE
    int i = 0;     //index of output
    char *temp = string;
    char *token = strtok(temp, "\n");  //output words from input
    token = strtok(temp, parseType);

    while(token != NULL){
        parsedInput[i] = malloc(strlen(token)+1);//Ensure a memory for storing
        strcpy(parsedInput[i], token);   //indexing
        token = strtok(NULL, parseType);  //splitting
        i++; //next index
    }
    return i;
}

void *connection_handler(void *socket_desc)
{
    char* inputs[20];

    int sock = *((int*)socket_desc);
    char sMessage[2000];
    char rMessage[2000];


    int read_size;
    while ((read_size = recv(sock, rMessage, 2000, 0)) >0) {
        printf("Command comes from User : %d\n",sock);
        printf("Command : %s\n",rMessage);
        flag = 0;
        int size = parse(inputs,rMessage," ");

        if(!strcmp(inputs[0],"login")){              ////////////////////////////////// LOGIN
            struct Person ps;
            strcpy(ps.name, inputs[1]);
            ps.socket = sock;
            Accounts[account_counter] = ps;
            account_counter++;
            printf("User Logged in as : %s. User ID is : %d\n",ps.name,ps.socket);
        }                                           //////////////////////////////////////////////////////
        else if(!strcmp(inputs[0],"getusers")){     /////////////////////////////////// GET USERS
            strcpy(sMessage,"");
            for (int i = 0; i < account_counter; ++i) {
                strcat(sMessage,Accounts[i].name);
                strcat(sMessage,",");
            }
            printf("Response is : %s\n",sMessage);
            if (send(sock, sMessage, strlen(sMessage)+1, 0) < 0) {
                puts("Send failed");
                return 1;
            }
            puts("Data sent...");
        }                                           //////////////////////////////////////////////////////
        else if(!strcmp(inputs[0],"alias")){        //////////////////////////////////// ALIAS
            struct group gr;
            strcpy(gr.group_name,inputs[1]);
            char* persons[15];
            int memberCount = parse(persons,inputs[2],",");
            for (int i = 0; i < memberCount; ++i) {
                gr.members[i] = persons[i];
            }
            gr.count = memberCount;
            Groups[group_counter] = gr;
            group_counter++;
            printf("Group Created");
        }                                           /////////////////////////////////////////////////////
        else{                                       //////////////////////////////////// MESSAGE
            char user[15];          /////// Personal Messages
            struct Person key;
            for (int i = 0; i < account_counter; ++i) {
                if(!strcmp(inputs[0],Accounts[i].name)){
                    key = Accounts[i];
                    flag = 1;
                    break;
                }
            }
            if(flag){
                for (int i = 0; i < account_counter; ++i) {
                    if(sock == Accounts[i].socket){
                        strcpy(user,Accounts[i].name);
                        break;
                    }
                }
                strcpy(sMessage,"\n");
                strcat(sMessage,user);
                strcat(sMessage," : ");
                for (int i = 1; i < size; ++i) {
                    strcat(sMessage,inputs[i]);
                    strcat(sMessage," ");
                }
                if (send(key.socket, sMessage, strlen(sMessage)+1, 0) < 0) {
                    puts("Send failed");
                    return 1;
                }
                puts("Data sent...");
            }
            else{                     //// Group Messages
                struct group group_key;
                for (int i = 0; i < group_counter; ++i) {
                    if(!strcmp(inputs[0],Groups[i].group_name)){
                        group_key = Groups[i];
                        flag = 1;
                        break;
                    }
                }
                if(flag){
                    for (int i = 0; i < account_counter; ++i) {
                        if(sock == Accounts[i].socket){
                            strcpy(user,Accounts[i].name);
                            break;
                        }
                    }
                    for (int j = 0; j < group_key.count; ++j) {
                        printf("%d",j);
                        for (int i = 0; i < account_counter; ++i) {
                            if(!strcmp(group_key.members[j],Accounts[i].name)){
                                key = Accounts[i];
                                break;
                            }
                        }
                        strcpy(sMessage,"\n");
                        strcat(sMessage,user);
                        strcat(sMessage," : ");
                        for (int i = 1; i < size; ++i) {
                            strcat(sMessage,inputs[i]);
                            strcat(sMessage," ");
                        }
                        if (send(key.socket, sMessage, strlen(sMessage)+1, 0) < 0) {
                            puts("Send failed");
                            return 1;
                        }
                    }
                }
            }

        }                                           ///////////////////////////////////////////////////////
    }
    if(read_size == 0)
    {
        puts("Client disconnected");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }

}

int main(int argc, char *argv[])
{
    int socket_desc, new_socket, c, *new_sock;
    struct sockaddr_in server, client;

    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        puts("Could not create socket");
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(5575);

    if(bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        puts("Binding failed");
        return 1;
    }

    listen(socket_desc, 3);

    puts("Waiting for incoming connections...");
    c = sizeof(struct sockaddr_in);
    while((new_socket =
                   accept(socket_desc, (struct sockaddr *)&client, (socklen_t*) &c)) )
    {


        puts("Connection accepted");


        pthread_t sniffer_thread;
        new_sock = malloc(1);
        *new_sock = new_socket;

        if(pthread_create(&sniffer_thread, NULL, connection_handler,
                          (void*)new_sock) < 0)
        {
            puts("Could not create thread");
            return 1;
        }

        puts("Handler assigned");
    }

    return 0;
}
