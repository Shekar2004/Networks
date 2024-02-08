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

int main(int argc, char *argv[])
{
	char username[40],password[40];
	printf("username: ");scanf("%[^\n]s",username);
	printf("password: ");scanf("%[^\n]s",password);

	int	sockfd ;
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

		if(n==1){printf("not yet implemented pop3.\n");continue;}
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
			
			if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
				perror("Unable to create socket\n");
				exit(0);
			}
			serv_addr.sin_family = AF_INET;
			inet_aton(argv[1], &serv_addr.sin_addr);
			serv_addr.sin_port	= htons(atoi(argv[2]));

			if ((connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) < 0) {
				perror("Unable to connect to server\n");
				exit(0);
			}

			memset(buf,'\0',100);
			memset(message,'\0',100);
			memset(message1,'\0',80);

			recv(sockfd, buf, 100, 0);
			// sscanf(buf,"%d %[^\n]s",&code,message);
			// sscanf(message,"%s",message1);
			printf("%s\n",buf);

			memset(buf,'\0',100);
			sprintf(buf,"HELO %s",message1);printf("%s\n",buf);
			send(sockfd,buf,strlen(buf)+1,0);

			memset(buf,'\0',100);
			recv(sockfd, buf, 100, 0);
			printf("%s\n",buf);

			memset(buf,'\0',100);
			sprintf(buf,"MAIL FROM:<%s>",from);
            printf("%s\n",buf);
			send(sockfd,buf,strlen(buf)+1,0);

			memset(buf,'\0',100);
			recv(sockfd, buf, 100, 0);
            printf("%s\n",buf);

			memset(buf,'\0',100);
			sprintf(buf,"RCPT TO:<%s>",to);printf("%s\n",buf);
			send(sockfd,buf,strlen(buf)+1,0);


			memset(buf,'\0',100);
			recv(sockfd, buf, 100, 0);printf("%s\n",buf);
			sscanf(buf,"%d",&code);
			if(code==550){printf("Error in sending mail: <%s>\n",buf);close(sockfd);continue;}

			memset(buf,'\0',100);
			sprintf(buf,"DATA");printf("%s\n",buf);
			send(sockfd,buf,strlen(buf)+1,0);

			memset(buf,'\0',100);
			recv(sockfd, buf, 100, 0);printf("%s\n",buf);
			sscanf(buf,"%d",&code);

			if(code==354)
			{
				memset(buf,'\0',100);
				sprintf(buf,"From:<%s>",from);printf("%s\n",buf);
				send(sockfd,buf,100,0);

				memset(buf,'\0',100);
				sprintf(buf,"To:<%s>",to);printf("%s\n",buf);
				send(sockfd,buf,100,0);
				//recv(sockfd, buf, 100, 0);
				memset(buf,'\0',100);
				printf("%d\n",sprintf(buf,"Subject: %s",subject));printf("%s\n",buf);
				send(sockfd,buf,100,0);memset(buf,'\0',100);

				for(int i=0;i<lines;i++)
				{
					memset(buf,'\0',100);
					strcpy(buf,body[i]);printf("%s\n",buf);
					send(sockfd,buf,100,0);
                    // printf("%d\n",i);
				}				
			}
            // printf("receiving....\n");
			memset(buf,'\0',100);
			recv(sockfd, buf, 100, 0);printf("%s\n",buf);

			memset(buf,'\0',100);
			sprintf(buf,"QUIT");
			send(sockfd,buf,5,0);

			memset(buf,'\0',100);
			recv(sockfd, buf, 100, 0);printf("%s\n",buf);
			sscanf(buf,"%d",&code);
			if(code==221)
			{
				close(sockfd);continue;
			}

			close(sockfd);
			continue;
		}
		if(n==3)exit(0);
	}

	return 0;
}
