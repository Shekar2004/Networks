#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 100
#define MAX_BUFFER_SIZE 3600
#define PORT 5096
#define MAX_CLIENTS 5
#define MAX_USERS 100
#define MAX_EMAILS 100

struct User{
    char username[BUFFER_SIZE];
    char password[BUFFER_SIZE];
};

struct Email
{
    char from[BUFFER_SIZE];
    char to[BUFFER_SIZE];
    char received[BUFFER_SIZE];
    char subject[BUFFER_SIZE];
    char body[MAX_BUFFER_SIZE];
};

void mailManager(int newsockfd,char username[]);

void trimNewline(char *str) {
    int len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0'; // Replace newline with null terminator
    }
}

int main(int argc, char* argv[]){
    //declarations
    int sockfd,newsockfd;
    int clilen;
    struct sockaddr_in cli_addr,serv_addr;

    char buf[BUFFER_SIZE];
    char username[BUFFER_SIZE];
    char password[BUFFER_SIZE];

    int idx,num_users=0;
    struct User users[MAX_USERS];

    //opening user file to read the number of users
    FILE *file=fopen("user.txt","r");
    if(file==NULL){
        perror("Error in opening user.txt file");
        exit(EXIT_FAILURE);
    }

    //copying username and passwords from file
    while(fscanf(file,"%s  %s",users[num_users].username,users[num_users].password)==2){
        num_users++;
    }

    fclose(file);

    //socket creation
    if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0){
        printf("Cannot create socket\n");
        exit(0);
    }

    //server details
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=INADDR_ANY;
    serv_addr.sin_port=htons(PORT);

    //socket binding
    if(bind(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr))<0){
        printf("Unable to bind to server address");
        exit(0);
    }

    //listenting
    listen(sockfd,5);

    printf("Server listening at port %d...\n",PORT);


    while(1){
        //accept for client
        clilen=(sizeof(cli_addr));
        newsockfd=accept(sockfd,(struct sockaddr*)&cli_addr,&clilen);

        if(newsockfd<0){
            printf("Accepting Client Failed!!\n");
            exit(0);
        }

        //client into child process, parent continues to listen for new clients
        if(fork()==0){
            close(sockfd);

            //OK message
            strcpy(buf,"+OK POP3 Server Ready");
            send(newsockfd,buf,strlen(buf)+1,0);

            //recv username
            memset(buf,0,sizeof(buf));
            recv(newsockfd,buf,sizeof(buf),0);
            strcpy(username,buf);
            printf("username: %s\n",username);

            //checking if user exist
            int usrExist=0;
            int k=0;
            for(k=0;k<num_users;k++){
                if(strcmp(users[k].username,username)==0){
                    usrExist=1;
                    break;
                }
            }

            //checking for passwrod if exist other wise error message
            if(usrExist==1){
                strcpy(buf,"+OK user exists");
                send(newsockfd,buf,sizeof(buf),0);
                
                //recv password
                memset(buf,0,sizeof(buf));
                recv(newsockfd,buf,sizeof(buf),0);
                strcpy(password,buf);
                printf("password: %s\n",password);
                
                memset(buf,0,sizeof(buf));
                //matching password else error message
                if(strcmp(users[k].password,password)==0){
                    strcpy(buf,"+OK password match");
                    send(newsockfd,buf,sizeof(buf),0);
                    mailManager(newsockfd,username);
                }else{
                    strcpy(buf,"-ERR password mismatch");
                    send(newsockfd,buf,sizeof(buf),0);
                }

            }else{
                //user does not exist
                memset(buf,0,sizeof(buf));
                strcpy(buf,"-ERR user does not exist");
                send(newsockfd,buf,sizeof(buf),0);
            }

            close(newsockfd);
            exit(0);
        }

        close(newsockfd);
    }

    return 0;
}

void mailManager(int newsockfd,char username[]){
    char buf[BUFFER_SIZE];
    char buf2[4*BUFFER_SIZE];
    char accumulator[4096];
    char mailBoxPath[BUFFER_SIZE];
    char temp[BUFFER_SIZE];
    int idx;

    //mail box path
    snprintf(mailBoxPath,sizeof(mailBoxPath),"./%s/mymailbox",username);
    
    //file opening
    FILE *file=fopen(mailBoxPath,"r");
    if(file == NULL){
        perror("Error opening mail file");
        exit(EXIT_FAILURE);
    }

    struct Email emails[MAX_EMAILS];
    int num_emails=0;
    int del_count=0;
    int *dlt;
    char line[BUFFER_SIZE];

    //reading mails
    while(fgets(line,sizeof(line),file)!=NULL){
        if(strcmp(line,".\n")==0){
            num_emails++;
            continue;
        }

        if(strncmp(line,"From:",5)==0){
            trimNewline(line);
            strcpy(emails[num_emails].from,line+6);
        }else if(strncmp(line,"To:",3)==0){
            trimNewline(line);
            strcpy(emails[num_emails].to,line+4);
        }else if(strncmp(line,"Received:",9)==0){
            trimNewline(line);
            strcpy(emails[num_emails].received,line+10);
        }else if(strncmp(line,"Subject:",8)==0){
            trimNewline(line);
            strcpy(emails[num_emails].subject,line+9);
        }else{
            strcat(emails[num_emails].body,line);
        }

    }

    fclose(file);

    dlt=(int *)malloc(num_emails*sizeof(int));
    for(int i=0;i<num_emails;i++){
        dlt[i]=0;
    }

    while(1){
        memset(buf,0,sizeof(buf));
        recv(newsockfd,buf,sizeof(buf),0);
        printf("Client: %s\n",buf);
        if(strncmp(buf,"STAT",4)==0){//stat for number of mails
            sprintf(buf,"%d",num_emails);
            send(newsockfd,buf,sizeof(buf),0);

            memset(buf,0,sizeof(buf));
            recv(newsockfd,buf,sizeof(buf),0);
            printf("Client: %s\n",buf);
            if(strncmp(buf,"LIST",4)==0){//display menu
                for(int i=0;i<num_emails;i++){
                    memset(buf,0,sizeof(buf));
                    sprintf(buf2,"%d || %s || %s || %s",i+1,emails[i].from,emails[i].received,emails[i].subject);
                    if(dlt[i]==1){
                        strcpy(buf2,"-ERR");
                    }
                    send(newsockfd,buf2,sizeof(buf2),0);
                }

                memset(buf,0,sizeof(buf));
                recv(newsockfd,buf,sizeof(buf),0);
                printf("Client: %s\n",buf);
                if(strncmp(buf,"RETR",4)==0){//return particular mail
                    memset(buf,0,sizeof(buf));
                    recv(newsockfd,buf,sizeof(buf),0);
                    idx=atoi(buf);
                    printf("mail number asked: %d\n",idx);
                    if(idx<0){
                        break;
                    }

                    memset(accumulator,0,sizeof(accumulator));
                    sprintf(accumulator,"From:%s\nTo:%s\nReceived:%s\nSubject:%s%s",emails[idx-1].from,emails[idx-1].to,emails[idx-1].received,emails[idx-1].subject,emails[idx-1].body);
                    send(newsockfd,accumulator,sizeof(accumulator),0);
                    memset(buf,0,sizeof(buf));
                    recv(newsockfd,buf,sizeof(buf),0);
                    printf("Client: %s\n",buf);
                    if(strncmp(buf,"DELE",4)==0){//delete mail
                        dlt[idx-1]=1;
                        del_count++;
                    }
                }
            }
        }
    }

    for(int i=0;i<num_emails;i++){
        if(dlt[i]==1){
            printf("%d ",i+1);
        }
    }
}