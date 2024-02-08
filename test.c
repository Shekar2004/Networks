#include <stdio.h>
#include <string.h>

int main() {
    const char* sample_email_template = "From: user1@<domain_name>\n"
                                        "To: user3@<domain_name>\n"
                                        "Subject: sample subject\n"
                                        "Received: <time at which received, in date: hour : minute>\n"
                                        "sample line1\n"
                                        ".";

    // Create a copy of the template
    char buf[1024];
    strcpy(buf, sample_email_template);

    // Replace the time placeholder with a specific date and time
    const char* specific_time = "2006:13:01";
    const char* placeholder = "<time at which received, in date: hour : minute>";
    
    char* time_placeholder = strstr(buf, placeholder);
    
    if (time_placeholder != NULL) {
        size_t prefix_length = time_placeholder - buf;
        size_t suffix_length = strlen(buf) - (prefix_length + strlen(placeholder));

        char temp[1024];
        strncpy(temp, buf, prefix_length);
        temp[prefix_length] = '\0';
        strcat(temp, specific_time);
        strcat(temp, time_placeholder + strlen(placeholder));

        strcpy(buf, temp);
    }

    // Print the modified string
    printf("Modified string:\n%s\n", buf);

    return 0;
}

// void handle_client(int client_socket) {
//     char buf[MAX_BUFFER_SIZE];

//     strcpy(buf,"200 Service Ready");
//     send(client_socket,buf,strlen(buf)+1,0);

//     memset(buf,0,sizeof(buf));
//     recv(client_socket,buf,sizeof(buf),0);

//     FILE* mailbox;
//     char to_username[MAX_BUFFER_SIZE];
//     char received_time[MAX_BUFFER_SIZE];
//     time_t current_time;
//     struct tm* time_info;

//     sscanf(buf,"%*[^\n]\nTo: %[^@]", to_username);

//     time(&current_time);
//     time_info = localtime(&current_time);
//     strftime(received_time,sizeof(received_time),"%Y-%m-%d %H:%M", time_info);

//     modifyEmail(buf, received_time);

//     char mailbox_path[MAX_MAILBOX_PATH];
//     snprintf(mailbox_path,sizeof(mailbox_path),"./%s/mymailbox",to_username);

//     printf("path: %s\n",mailbox_path);

//     mailbox=fopen(mailbox_path,"a");

//     if(mailbox==NULL){
//         perror("Error opening mailbox file");
//         exit(EXIT_FAILURE);
//     }

//     printf("%s\n",buf);

//     if (fprintf(mailbox, "%s\n",buf) < 0) {
//     perror("Error writing to mailbox file");
//     fclose(mailbox);
//     exit(EXIT_FAILURE);
//     } else {
//     printf("Message successfully written to mailbox\n");
//     }

//     fclose(mailbox);

//     strcpy(buf,"250 OK message accpeted for delivery");
//     send(client_socket,buf,strlen(buf)+1,0);
// }