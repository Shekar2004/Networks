#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_BUFFER_SIZE 1024
#define BUFFER_SIZE 100
#define PORT 5096
#define IPADDR "127.0.0.1"

void handleMail(int sockfd);

int main(int argc, char *argv[]){
    //declartions
    int sockfd;
    struct sockaddr_in serv_addr;
    char buf[BUFFER_SIZE];
    char username[BUFFER_SIZE];
    char password[BUFFER_SIZE];

    //socket creation
    if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0){
        perror("Cannot create socket\n");
        exit(0);
    }

    //server details
    serv_addr.sin_family = AF_INET;
    inet_aton(IPADDR, &serv_addr.sin_addr);
    serv_addr.sin_port=htons(PORT);

    //connection
    if(connect(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr))<0){
        printf("Connection failed\n");
        exit(0);
    }


    //OK message from server
    recv(sockfd,buf,sizeof(buf),0);
    printf("Server: %s\n",buf);

    //username
    printf("Enter Username: ");
    scanf("%[^\n]s",username);
    getchar();

    strcpy(buf,username);
    send(sockfd,buf,sizeof(buf),0);

    memset(buf,0,sizeof(buf));
    recv(sockfd,buf,sizeof(buf),0);
    if(strncmp(buf,"+OK",3)==0){
        //password
        printf("Enter Password: ");
        scanf("%[^\n]s",password);
        getchar();

        //send password
        memset(buf,0,sizeof(buf));
        strcpy(buf,password);
        send(sockfd,buf,sizeof(buf),0);

        //recv response message
        memset(buf,0,sizeof(buf));
        recv(sockfd,buf,sizeof(buf),0);
        printf("Server: %s\n",buf);
        if(strncmp(buf,"+OK",3)==0){
            handleMail(sockfd);
            memset(buf,0,sizeof(buf));
            // strcpy(buf,"STAT send number of mails");
            // send(sockfd,buf,sizeof(buf),0);

            // memset(buf,0,sizeof(buf));
        }
    }else{
        printf("Server: %s\n",buf);
    }

    

    // strcpy(buf,"Hello from client");
    // send(sockfd,buf,strlen(buf)+1,0);

    close(sockfd);
    return 0;
}

void handleMail(int sockfd){
    char buf[BUFFER_SIZE];
    char buf2[4*BUFFER_SIZE];
    char mailInfo[4096];
    int mailNo,num_emails;
    char del;

    while(1){
        memset(buf,0,sizeof(buf));
        strcpy(buf,"STAT send number of mails");
        send(sockfd,buf,sizeof(buf),0);

        memset(buf,0,sizeof(buf));
        recv(sockfd,buf,sizeof(buf),0);
        num_emails=atoi(buf);

        // printf("number of mails: %d",);

        memset(buf,0,sizeof(buf));
        strcpy(buf,"LIST display mail menu");
        send(sockfd,buf,sizeof(buf),0);

        printf("S.No || Sender EmailID || Received Time || Subject\n");
        for(int i=0;i<num_emails;i++){
            memset(buf2,0,sizeof(buf2));
            recv(sockfd,buf2,sizeof(buf2),0);
            if(strncmp(buf2,"-ERR",4)!=0){
                printf("%s\n",buf2);
            }
        }

        do{
            printf("Enter mail no. to see:");
            scanf("%d",&mailNo);
            if(mailNo>num_emails){
                printf("Mail no. out of range, give again\n");
            }
        }while(mailNo>num_emails);

        memset(buf,0,sizeof(buf));
        strcpy(buf,"RETR");
        send(sockfd,buf,sizeof(buf),0);

        memset(buf,0,sizeof(buf));
        sprintf(buf,"%d",mailNo);
        send(sockfd,buf,sizeof(buf),0);

        if(mailNo<0){
            break;
        }

        memset(mailInfo,0,sizeof(mailInfo));
        recv(sockfd,mailInfo,sizeof(mailInfo),0);
        printf("Mail Asked for:\n %s",mailInfo);
        getchar();
        del=getchar();

        if(del=='d'){
            memset(buf,0,sizeof(buf));
            strcpy(buf,"DELE");
            send(sockfd,buf,sizeof(buf),0);
        }else{
            memset(buf,0,sizeof(buf));
            strcpy(buf,"some random");
            send(sockfd,buf,sizeof(buf),0);
        }
    }

}