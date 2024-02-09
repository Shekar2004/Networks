#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 5069
#define MAX_BUFFER_SIZE 4096
#define MAX_MAILBOX_PATH 256
#define BUFFER_SIZE 100
#define MAX_CLIENTS 5

int my_port=PORT;

void handle_client(int client_socket);
void modifyEmail(char* buf, const char* received_time);

int main(int argc, char* argv[]){
    if(argc>=2){
        my_port=atoi(argv[1]);
    }

    printf("SMTP running at port %d...\n",my_port);

    int sockfd,newsockfd;
    int clilen;
    struct sockaddr_in cli_addr,serv_addr;

    char buf[MAX_BUFFER_SIZE];

    if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0){
        perror("Cannot create socket");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(my_port);

    if(bind(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr))<0){
        perror("Unable to bind local address");
        exit(EXIT_FAILURE);
    }

    listen(sockfd,MAX_CLIENTS);

    while(1){
        clilen=sizeof(cli_addr);
        newsockfd = accept(sockfd,(struct sockaddr*)&cli_addr,&clilen);

        if(newsockfd<0){
            perror("Accepting client FAILED!!");
            exit(EXIT_FAILURE);
        }

        if(fork()==0){
            close(sockfd);

            handle_client(newsockfd);

            close(newsockfd);
            exit(0);
        }
        close(newsockfd);
    }

    close(sockfd);

    return 0;
}

void handle_client(int client_socket){
    char buf[BUFFER_SIZE];
    char accumulator[MAX_BUFFER_SIZE];

    FILE* mailbox;
    char to_username[BUFFER_SIZE];
    char received_time[MAX_BUFFER_SIZE];
    time_t current_time;
    struct tm* time_info;

    char mailbox_path[MAX_MAILBOX_PATH];

    strcpy(buf,"220 Service Ready");
    send(client_socket,buf,strlen(buf)+1,0);

    while(1){
        memset(buf,0,sizeof(buf));
        recv(client_socket,buf,sizeof(buf),0);
        printf("Client: %s\n",buf);

        if(strncmp(buf,"HELO",4)==0){
            strcpy(buf,"250 OK Helo");
            send(client_socket,buf,strlen(buf)+1,0);

            memset(buf,0,sizeof(buf));
            recv(client_socket,buf,sizeof(buf),0);
            printf("Client: %s\n",buf);

            if(strncmp(buf,"MAIL FROM",9)==0){
                strcpy(buf,"250 Sender ok");
                send(client_socket,buf,strlen(buf)+1,0);

                memset(buf,0,sizeof(buf));
                recv(client_socket,buf,sizeof(buf),0);
                printf("Client: %s\n",buf);

                if(strncmp(buf,"RCPT TO",7)==0){
                    strcpy(buf,"250 Recipient ok");
                    send(client_socket,buf,strlen(buf)+1,0);

                    memset(buf,0,sizeof(buf));
                    recv(client_socket,buf, sizeof(buf),0);
                    printf("Client: %s\n",buf);

                    if(strncmp(buf,"DATA",4)==0){
                        strcpy(buf,"354 Enter mail end with \".\" on a line by itself");
                        send(client_socket,buf,strlen(buf)+1,0);

                        while(1){
                            memset(buf,0,sizeof(buf));
                            recv(client_socket,buf,sizeof(buf),0);
                            printf("Client: %s\n",buf);

                            if(strcmp(buf,".") == 0){
                                strcat(accumulator,buf);strcat(accumulator,"\n");
                                // printf(" breaking loop\n");
                                break;
                            }else{
                                strcat(accumulator,buf);strcat(accumulator,"\n");

                                if(strncmp(buf,"To:",3)==0){
                                    //sscanf(buf,"%*[^\n]\nTo:< %[^@]", to_username);
                                    sscanf(buf,"To:<%s",to_username);
                                    to_username[strlen(to_username)-1]='\0';
                                    for(int i=0;i>=0;i++)
                                    {
                                        if(to_username[i]=='@'){to_username[i]='\0';break;}
                                    }
                                    // printf("username: %s\n",to_username);
                                    strcat(accumulator,"Received: <time at which received, in date: hour : minute>\n");
                                    // printf("reading user name\n");
                                }
                            }
                        }
                            // strcpy(buf,"250 OK message accepted for delivery");
                            // send(client_socket,buf,strlen(buf)+1,0);
                            snprintf(mailbox_path,sizeof(mailbox_path),"./%s/mymailbox",to_username);

                            // printf("path: %s\n",mailbox_path);

                            time(&current_time);
                            time_info = localtime(&current_time);
                            strftime(received_time,sizeof(received_time),"%Y-%m-%d %H:%M", time_info);

                            modifyEmail(accumulator, received_time);

                            mailbox=fopen(mailbox_path,"a");

                            if(mailbox==NULL){
                                perror("Error opening mailbox file");
                                exit(EXIT_FAILURE);
                            }

                            // printf("%s\n",buf);

                            if (fprintf(mailbox, "%s\n",accumulator) < 0) {
                            perror("Error writing to mailbox file");
                            fclose(mailbox);
                            exit(EXIT_FAILURE);
                            } else {
                            printf("Message successfully written to mailbox\n");
                            }

                            fclose(mailbox);

                            strcpy(buf,"250 OK message accepted for delivery");
                            send(client_socket,buf,strlen(buf)+1,0);
                    }
                }
            }
        }

        if(strncmp(buf,"QUIT",4)==0){
            strcpy(buf,"221 closing connection...");
            send(client_socket,buf,strlen(buf)+1,0);
            break;
        }
    }
}

void modifyEmail(char* buf, const char* received_time) {
    const char* placeholder = "<time at which received, in date: hour : minute>";

    char* time_placeholder = strstr(buf, placeholder);

    if (time_placeholder != NULL) {
        size_t prefix_length = time_placeholder - buf;
        size_t suffix_length = strlen(buf) - (prefix_length + strlen(placeholder));

        char temp[1024];
        strncpy(temp, buf, prefix_length);
        temp[prefix_length] = '\0';
        strcat(temp, received_time);
        strcat(temp, time_placeholder + strlen(placeholder));

        strcpy(buf, temp);
    }
}
