#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#define maxlines 50
#define PORT 5069
#define PPORT 5096
#define MAX_BUFFER_SIZE 4096
#define BUFFER_SIZE 100
#define IPADDR "127.0.0.1"

int checkemailformat(char *mailid)
{
	int n=strlen(mailid);int count=0;
	for(int i=0;i<n;i++)
	{
		if(mailid[i]=='@'){count++;if(count>1)return 0;if(i==n-1 || i==0)return 0;}
	}return count;
}

int readfromuser(char *from,char *to,char *subject,char **body,int *lines)
{
	int i;
	printf("From: ");scanf("%[^\n]s",from);getchar();
	printf("To: ");scanf("%[^\n]s",to);getchar();
	printf("Subject: ");scanf("%[^\n]s",subject);getchar();
	for(i=0;i<maxlines;i++)
	{
		body[i]=(char *)malloc(80*sizeof(char));
		scanf("%[^\n]s",body[i]);getchar();if(i== maxlines-1)return 0;
		if(strcmp(body[i],".")==0){*lines = i+1;break;}
	}
	return checkemailformat(from)&&checkemailformat(to);
}

void managePop(int sockfd,char username[],char password[]);
void handleMail(int sockfd);

int main(int argc, char *argv[])
{
	char username[BUFFER_SIZE];
	char password[BUFFER_SIZE];
	printf("username: ");scanf("%[^\n]s",username);getchar();
	printf("password: ");scanf("%[^\n]s",password);getchar();

	int	smptfd,popfd;
	struct sockaddr_in	serv_addr;

	int i,n,lines,code;
	char buf[100],message[100],message1[80];

	char *from,*to,*subject,**body;
	from = (char *)malloc(sizeof(char)*20);
	to = (char *)malloc(sizeof(char)*20);
	subject = (char *)malloc(sizeof(char)*50);
	body = (char **)malloc(80*sizeof(char *));

	while(1)
	{
		printf("\n1. Manage Mail : Shows the stored mails of the logged in user only\n");
		printf("2. Send Mail : Allows the user to send a mail\n");
		printf("3. Quit : Quits the program.\n");
		printf("enter choice >> ");
		if(!scanf("%d",&n))
		{
			while(getchar()!='\n');printf("enter valid choice\n");continue;
		}while(getchar()!='\n');

		if(n==1){
			managePop(popfd,username,password);
			continue;
		}
		if(n==2)
		{
			if(!readfromuser(from,to,subject,body,&lines))
			{
				printf("invalid format.\n");continue;
			}
			/*for(int i=0;i<lines;i++)
			{
				printf("%d.%s\n",i+1,body[i]);
			}*/
			
			if ((smptfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
				perror("Unable to create socket\n");
				exit(0);
			}
			serv_addr.sin_family = AF_INET;
			inet_aton("127.0.0.1", &serv_addr.sin_addr);
			serv_addr.sin_port	= htons(PORT);

			if ((connect(smptfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) < 0) {
				perror("Unable to connect to server\n");
				exit(0);
			}

			memset(buf,'\0',100);
			memset(message,'\0',100);
			memset(message1,'\0',80);

			recv(smptfd, buf, 100, 0);
			// sscanf(buf,"%d %[^\n]s",&code,message);
			// sscanf(message,"%s",message1);
			printf("%s\n",buf);

			memset(buf,'\0',100);
			sprintf(buf,"HELO %s",message1);printf("%s\n",buf);
			send(smptfd,buf,strlen(buf)+1,0);

			memset(buf,'\0',100);
			recv(smptfd, buf, 100, 0);
			printf("%s\n",buf);

			memset(buf,'\0',100);
			sprintf(buf,"MAIL FROM:<%s>",from);
            printf("%s\n",buf);
			send(smptfd,buf,strlen(buf)+1,0);

			memset(buf,'\0',100);
			recv(smptfd, buf, 100, 0);
            printf("%s\n",buf);

			memset(buf,'\0',100);
			sprintf(buf,"RCPT TO:<%s>",to);printf("%s\n",buf);
			send(smptfd,buf,strlen(buf)+1,0);


			memset(buf,'\0',100);
			recv(smptfd, buf, 100, 0);printf("%s\n",buf);
			sscanf(buf,"%d",&code);
			if(code==550){printf("Error in sending mail: <%s>\n",buf);close(smptfd);continue;}

			memset(buf,'\0',100);
			sprintf(buf,"DATA");printf("%s\n",buf);
			send(smptfd,buf,strlen(buf)+1,0);

			memset(buf,'\0',100);
			recv(smptfd, buf, 100, 0);printf("%s\n",buf);
			sscanf(buf,"%d",&code);

			if(code==354)
			{
				memset(buf,'\0',100);
				sprintf(buf,"From:<%s>",from);printf("%s\n",buf);
				send(smptfd,buf,100,0);

				memset(buf,'\0',100);
				sprintf(buf,"To:<%s>",to);printf("%s\n",buf);
				send(smptfd,buf,100,0);
				//recv(smptfd, buf, 100, 0);
				memset(buf,'\0',100);
				printf("%d\n",sprintf(buf,"Subject: %s",subject));printf("%s\n",buf);
				send(smptfd,buf,100,0);memset(buf,'\0',100);

				for(int i=0;i<lines;i++)
				{
					memset(buf,'\0',100);
					strcpy(buf,body[i]);printf("%s\n",buf);
					send(smptfd,buf,100,0);
                    // printf("%d\n",i);
				}				
			}
            // printf("receiving....\n");
			memset(buf,'\0',100);
			recv(smptfd, buf, 100, 0);printf("%s\n",buf);

			memset(buf,'\0',100);
			sprintf(buf,"QUIT");
			send(smptfd,buf,5,0);

			memset(buf,'\0',100);
			recv(smptfd, buf, 100, 0);printf("%s\n",buf);
			sscanf(buf,"%d",&code);
			if(code==221)
			{
				close(smptfd);continue;
			}

			close(smptfd);
			continue;
		}
		if(n==3)exit(0);
	}

	return 0;
}

void managePop(int sockfd,char username[],char password[]){
	struct sockaddr_in serv_addr;
    char buf[BUFFER_SIZE];

	if((sockfd=socket(AF_INET,SOCK_STREAM,0))<0){
        perror("Cannot create socket\n");
        exit(0);
    }

	serv_addr.sin_family = AF_INET;
    inet_aton(IPADDR, &serv_addr.sin_addr);
    serv_addr.sin_port=htons(PPORT);

	if(connect(sockfd,(struct sockaddr*)&serv_addr,sizeof(serv_addr))<0){
        printf("Connection failed\n");
        exit(0);
    }

	//OK message from server
    recv(sockfd,buf,sizeof(buf),0);
    printf("Server: %s\n",buf);

	strcpy(buf,username);
    send(sockfd,buf,sizeof(buf),0);

	memset(buf,0,sizeof(buf));
    recv(sockfd,buf,sizeof(buf),0);

	if(strncmp(buf,"+OK",3)==0){
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
        }
    }else{
        printf("Server: %s\n",buf);
    }

	close(sockfd);
}

void handleMail(int sockfd){
    char buf[BUFFER_SIZE];
    char buf2[4*BUFFER_SIZE];
    char mailInfo[4096];
    int mailNo,num_emails;
    char del;
    int queryNo;

    memset(buf,0,sizeof(buf));
    strcpy(buf,"STAT send number of mails");
    send(sockfd,buf,sizeof(buf),0);

    memset(buf,0,sizeof(buf));
    recv(sockfd,buf,sizeof(buf),0);
    num_emails=atoi(buf);

    while(1){
        printf("## OPTIONS ##\n");
        printf("1. Display number of mails in mail box\n");
        printf("2. List all mails in mail box\n");
        printf("3. Mail retrival\n");
        printf("4. Reset deleted mails\n");
        printf("5. Quit\n");
        printf("Enter the operation number to be performed:\n");
        scanf("%d",&queryNo);

        if(queryNo==1){
            memset(buf,0,sizeof(buf));
            strcpy(buf,"STAT");
            send(sockfd,buf,sizeof(buf),0);

            memset(buf,0,sizeof(buf));
            recv(sockfd,buf,sizeof(buf),0);
            printf("Number of mails in MailBox: %d\n",atoi(buf));
        }else if(queryNo==2){
            memset(buf,0,sizeof(buf));
            strcpy(buf,"LIST");
            send(sockfd,buf,sizeof(buf),0);

            printf("S.No || Sender EmailID || Received Time || Subject\n");
            for(int i=0;i<num_emails;i++){
                memset(buf2,0,sizeof(buf2));
                recv(sockfd,buf2,sizeof(buf2),0);
                if(strncmp(buf2,"-ERR",4)!=0){
                    printf("%s\n",buf2);
                }
            }
        }else if(queryNo==3){
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

        }else if(queryNo==4){
            memset(buf,0,sizeof(buf));
            strcpy(buf,"RSET");
            send(sockfd,buf,sizeof(buf),0);
            printf("Deleted mails are reset\n");
        }else if(queryNo==5){
            memset(buf,0,sizeof(buf));
            strcpy(buf,"QUIT");
            send(sockfd,buf,sizeof(buf),0);
            break;
        }

    }
}