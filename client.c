#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 5070
#define MAX_BUFFER_SIZE 100

void send_command(int sockfd, const char* command);
void send_email(int sockfd, const char* email);

int main(int argc, char* argv[]) {
    int sockfd;
    struct sockaddr_in serv_addr;
    char buf[MAX_BUFFER_SIZE];
    char user_input[MAX_BUFFER_SIZE];

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Unable to create socket\n");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    inet_aton("127.0.0.1", &serv_addr.sin_addr);
    serv_addr.sin_port = htons(PORT);

    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Unable to connect to server\n");
        exit(EXIT_FAILURE);
    }

    // Receive the service ready message
    memset(buf, 0, sizeof(buf));
    recv(sockfd, buf, sizeof(buf), 0);
    printf("Server says:%s\n", buf);

    while (1) {
        printf("Enter command (or 'quit' to exit): ");
        fgets(user_input, sizeof(user_input), stdin);

        // Remove newline character
        user_input[strcspn(user_input, "\n")] = '\0';

        // Send user input as a command to the server
        send_command(sockfd, user_input);

        // Receive and print the server's response
        memset(buf, 0, sizeof(buf));
        recv(sockfd, buf, sizeof(buf), 0);
        printf("Server says: %s\n", buf);

        // Check if the user entered 'quit' to exit the loop
        if (strcmp(user_input, "QUIT") == 0) {
            break;
        }

        // If the user entered the 'DATA' command, send the sample email
        if (strcmp(user_input, "DATA") == 0) {
            const char* sample_email = "From: user1@<domain_name>\n"
                                       "To: user2@<domain_name>\n"
                                       "Subject: sample subject\n"
                                       "Received: <time at which received, in date: hour : minute>\n"
                                       "sample line1\n"
                                       ".";
            send_email(sockfd, sample_email);

            // Wait for the server's response after sending the email
            memset(buf, 0, sizeof(buf));
            recv(sockfd, buf, sizeof(buf), 0);
            printf("Server says: %s\n", buf);
        }
    }

    close(sockfd);

    return 0;
}

void send_command(int sockfd, const char* command) {
    char buf[MAX_BUFFER_SIZE];
    memset(buf, 0, sizeof(buf));

    // Send the command
    strcpy(buf, command);
    send(sockfd, buf, strlen(buf) + 1, 0);
}

void send_email(int sockfd, const char* email) {
    size_t email_length = strlen(email);
    size_t sent_length = 0;

    while (sent_length < email_length) {
        // Calculate the remaining length to send
        size_t remaining_length = email_length - sent_length;

        // Determine the length to send in this iteration (up to 100 characters)
        size_t send_length = remaining_length > MAX_BUFFER_SIZE ? MAX_BUFFER_SIZE : remaining_length;

        // Create a buffer to hold the current part of the email
        char email_part[MAX_BUFFER_SIZE + 1];
        strncpy(email_part, email + sent_length, send_length);
        email_part[send_length] = '\0';

        // Send the current part of the email
        send_command(sockfd, email_part);

        // Update the length of the sent email
        sent_length += send_length;
    }
}
