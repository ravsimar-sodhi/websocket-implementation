// Client side C/C++ program to demonstrate Socket programming
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#define PORT 8880

int main(int argc, char
    const * argv[]) {
    struct sockaddr_in address;
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    // char *hello = "Hello from client";
    char buffer[1024] = {
        0
    }, fileList[1024] = {
        0
    };
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n [client]: Socket creation error \n");
        return -1;
    }

    memset( & serv_addr, '0', sizeof(serv_addr)); // to make sure the struct is empty. Essentially sets sin_zero as 0
    // which is meant to be, and rest is defined below

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Converts an IP address in numbers-and-dots notation into either a 
    // struct in_addr or a struct in6_addr depending on whether you specify AF_INET or AF_INET6.
    if (inet_pton(AF_INET, "127.0.0.1", & serv_addr.sin_addr) <= 0) {
        printf("\n[client]: Invalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr * ) & serv_addr, sizeof(serv_addr)) < 0) // connect to the server address
    {
        printf("\n[client]: Connection Failed \n");
        return -1;
    }
    char * name = malloc(sizeof(char) * 1024);
    valread = read(sock, fileList, 1024);
    while (1) {
        printf("%s\n", fileList);
        bzero(buffer, 1024);
        char fileBuffer[1024] = {
            0
        };
        name = malloc(sizeof(char) * 1024);
        int dot_count = 0;
        scanf("%s", name);
        if (strcmp(name, "-1") == 0) {
            puts("Connection Closed");
            break;
        }
        puts(name);
        send(sock, name, strlen(name), 0);
        printf("[client]: Sending request to server to download %s \n", name);

        bzero(buffer, 1024);
        valread = read(sock, buffer, 1024);
        if (buffer[0] == 'O' && buffer[1] == 'K') {
            printf("[client]: server acknowledges file availability.\n");
            printf("[client]: Initiating file transfer of file %s.\n", name);
            printf("[client]: Receiving file.");
            fflush(stdout);
            FILE * fp;
            fp = fopen(name, "w");
            while ((valread = recv(sock, fileBuffer, 1023, 0)) > 0) {
                if (dot_count % 6000 == 0) {
                    fputs(".", stdout);
                    fflush(stdout);
                }
                char temp = fileBuffer[strlen(fileBuffer) - 1];
                dot_count++;
                if (temp == '#') {
                    fwrite(fileBuffer, sizeof(char), valread - 1, fp);
                    break;
                } else
                fwrite(fileBuffer, sizeof(char), valread, fp);
                bzero(fileBuffer, 1024);
            }
            puts("\n[client]: File received.\n");
            close(fp);
        } //close if
        else
            puts("[client]: server responded with error.\n\t Check whether filename was entered correctly.\n");
        free(name);
    }
    close(sock);
    return 0;
}