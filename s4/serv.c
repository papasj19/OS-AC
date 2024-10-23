#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define SERVER_IP2 "192.168.1.4" 
#define SERVER_IP "172.16.205.4" 
#define PORT 9655


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>



int main(int argc, char *argv[]) {
    //check args given 
    if (argc != 2) {
        customWrite("Usage: ./g <file>\n");
        return 1;
    }

//make the socket 
    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sockfd < 0){
        perror("socket");
        return 1;
    }


    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(9656);
    


        if(bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
            perror("bind");
            return 1;
        }


        listen(sockfd, 5);


    while(1){  

        int sock2 = accept(sockfd, NULL, NULL);
        if(sock2 < 0){
            perror("accept");
            return 1;
        }




        
    }
    

    


    return 0; 
}