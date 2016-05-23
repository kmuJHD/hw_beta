//temp code

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "types.h"


main()
{
    int c_socket, s_socket;
    struct sockaddr_in s_addr, c_addr;
    int len;
    char rcvBuffer[BUFSIZ];
    int n;
    
    s_socket = socket(PF_INET, SOCK_STREAM, 0);
    
    memset(&s_addr, 0, sizeof(s_addr));
    s_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(8000);
    
    
    if(bind(s_socket, (struct sockaddr*) &s_addr, sizeof(s_addr)) == -1)
    {
        printf("Can not Bind\n");
        return -1;        
    }
    
    
    if(listen(s_socket, 5) == -1)
    {
        printf("Listen Faile\n");
        return -1;
    }
    
    
    while(1)
    {
        // 매 패킷 전송이 끝나면 소켓을 닫고 새로 대기
        len = sizeof(c_addr);
        c_socket = accept(s_socket, (struct sockaddr*)&c_addr, &len);
        
        if(c_socket == -1)
        {
            printf("Accept Error\n");
            return -1;
        }
                
        ssize_t numBytesRcvd = recv(c_socket, rcvBuffer, BUFSIZE, 0);
        if(numBytesRcvd == -1)
        {
            printf("Recv Error\n");
            return -1;
        }
        
        rcvBuffer[numBytesRcvd] = '\0';

        printf("Server : %s\n", rcvBuffer);
        
        
        close(c_socket);
    }
    close(s_socket);
}
