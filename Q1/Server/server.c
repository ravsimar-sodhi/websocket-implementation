#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <dirent.h>
#define PORT 8880

void getFileList(char** fileList)
{
    struct dirent **namelist;
    int n;
    
    n = scandir("./Data", &namelist, NULL, alphasort);
    if (n == -1)
    {
        perror("scandir");
        return ;
    }

    while (n--)
    {
        // No hidden files are displayed.
        if((namelist[n]->d_name)[0] == '.')
            continue;
        else 
        {
            strcat(*fileList,namelist[n]->d_name);
            strcat(*fileList, "\n");
        }
    }
    return ;
}

int main(int argc, char const *argv[])
{
    int server_fd, new_socket, valread;
    struct sockaddr_in address;  // sockaddr_in - references elements of the socket address. "in" for internet
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    
    char* fileList = malloc(sizeof(char)*120);
    getFileList(&fileList);
    
    char* headPrompt = "[server]:Please choose file for downloading:\n";
    
    // Function call to get file list in current directory where server is run.

    // Creating socket file descriptor
    
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)  // creates socket, SOCK_STREAM is for TCP. SOCK_DGRAM for UDP
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // This is to lose the pesky "Address already in use" error message
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) // SOL_SOCKET is the socket layer itself
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;  // Address family. For IPv6, it's AF_INET6. 29 others exist like AF_UNIX etc. 
    address.sin_addr.s_addr = INADDR_ANY;  // Accept connections from any IP address - listens from all interfaces.
    address.sin_port = htons( PORT );    // Server port to open. Htons converts to Big Endian - Left to Right. RTL is Little Endian

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Port bind is done. You want to wait for incoming connections and handle them in some way.
    // The process is two step: first you listen(), then you accept()
    if (listen(server_fd, 3) < 0) // 3 is the maximum size of queue - connections you haven't accepted
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // returns a brand new socket file descriptor to use for this single accepted connection. Once done, use send and recv
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    
    send(new_socket, headPrompt, strlen(headPrompt), 0); 
    send(new_socket ,fileList, strlen(fileList) , 0);  
    printf("[server]: Sending file list to client.\n");
    
    bzero((char *)&buffer, sizeof(buffer));

    valread = read( new_socket , buffer, 1024);
    char fileBuffer[1024] = {0},fileName[1024] = {0};
    int dot_count=0;
    printf("[server]:Request received to download file: %s\n",buffer);
    strcpy(fileName,"Data/");
    strcat(fileName,buffer);
    FILE *f;
    f = fopen(fileName, "r");
    if(f  == NULL)
    {
        char* error = "[server]: Client requesting non-existent file.";
        printf("%s\n",error);
        send(new_socket,error,strlen(error),0);
    }
    else
    {
        char* ok = "OK";
        send(new_socket, ok, sizeof(ok), 0);
        printf("[server]: File is available for transfer. Preparing to send.\n");
        printf("[server]: Sending File.");
        fflush(stdout);
        while ((valread = fread(fileBuffer, sizeof(char), 1024, f)) > 0)
        {
            if(dot_count%6000==0)
            {
                printf(".");
                fflush(stdout);
            }
            if (send(new_socket, fileBuffer, valread, 0) < 0)
            {
                printf("[server]:ERROR: Failed to send file %s.\n", buffer);
                break;
            }
            bzero(fileBuffer, 1024);
            dot_count++;
        }
    }
    printf("\n[server]: File sent. Closing connection.\n");
    close(new_socket);
    close(server_fd);
    return 0;
}
