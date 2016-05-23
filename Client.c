/*
*   Client.c
*   홍현표 김상원 조심재
*   Warning
*   Linux 기반이므로 window 환경에서는 실행이 되지 않습니다.
*   PPT 에코 클라이언트 참고
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "types.h"

/*
types.h에 정의됨

typedef struct CP_Question{
    byte type;
    byte detail;
    byte grade;
    byte *data;
}CP_Question;

*Client 에서 보내는 패킷 type 
#define CP_QUESTION 0
#define CP_RENEW 1

*상세수준
#define LOW 0
#define MID 1
#define HIG 2

*등급
#define FIR 0
#define SEC 1
#define THI 2
*/

main()
{
    int c_socket;
    struct sockaddr_in c_addr;
    int len;
    char sndBuffer[BUFSIZ], rcvBuffer[BUFSIZ];
    CP_Question question;
    CP_Renew renew;
   
    
    memset(&c_addr, 0, sizeof(c_addr));
    c_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    c_addr.sin_family = AF_INET;
    c_addr.sin_port = htons(8000);
    
    
    while(1)
    {
        // 매 패킷 전송마다 새로 연결 생성
        c_socket = socket(PF_INET, SOCK_STREAM, 0);
        if(connect(c_socket, (struct sockaddr*)&c_addr, sizeof(c_addr)) == -1)
        {
            printf("Can not connect\n");
            close(c_socket);
            return -1;
        }
        
        
        //질문을 입력받아 sndBuffer에 입력
        printf("질문 : ");
        fgets(sndBuffer, sizeof(sndBuffer), stdin); 
        
        //fgets로 가져온 문자열을 끝에 줄바꿈문자가 들어가기 때문에 줄바굼 문자를 제거
        sndBuffer[strlen(sndBuffer) - 1] = '\0';    
        

        //------보내고자 하는 패킷을 구성------//
        question.type = '0';
        question.detail = '0';
        question.grade = '0';
        strcpy(question.data, sndBuffer);
        
        
        //testing
        int strlength = 0;
        
        strlength += sizeof(question.type);
        strlength += sizeof(question.detail);
        strlength += sizeof(question.grade);
        strlength += strlen(question.data);
                
        char *sndString;
        
        sndString = (char *)malloc(strlength);
        
        sndString[0] = question.type;
        sndString[1] = question.detail;
        sndString[2] = question.grade;
        strcat(sndString, question.data);        
        
        printf("%s %d\n", sndString, strlength);
        /*
        strcat(sndString, question.type);
        strcat(sndString, question.detail);
        strcat(sndString, question.data);
        strcat(sndString, question.grade);
        */
        
        //-----------CP_QUESTION----------//
        
        //------보내고자 하는 패킷을 구성------//
        renew.type = '1';
        //-------------CP_RENEW-----------//
        
        
        /*  폐기안 - 사용하지 않음
        //패킷을 보낼때 필요한 버퍼의 크기를 구하고 send함수를 통해 question구조체를 전송
        size_t bufferLen = sizeof(question);
        ssize_t numBytesSent = send(c_socket, (char*)&question, bufferLen, 0);
    
        //numBytesSent에는 send한 패킷의 크기가 반환되며 실패시 -1이 반환
        if(numBytesSent == -1)
        {
             printf("Send Error\n");
             close(c_socket);
             return -1;
        }
        */
        
        /*  개선안      
        *   기존에 구조체를 전송하게 되면 낭비되는 byte가 많아서 동적 할당을 통해 문자열을 생성하고
        *   패킷의 크기에 맞게 전송하도록 개선
        */   
        /*  패킷을 보낼때 send함수를 통해 동적 할당한 sndString 문자열을 전송 strlengths는
        *   위에서 계산한 sndString의 크기임
        */
        ssize_t numBytesSent = send(c_socket, sndString, strlength, 0);
   
        //numBytesSent에는 send한 패킷의 크기가 반환되며 실패시 -1이 반환
        if(numBytesSent == -1)
        {
             printf("Send Error\n");
             close(c_socket);
             return -1;
        }
        
        //RENEW 구조체에 대한 패킷 전송 / 실제 1byte 크기
        size_t bufferLen = sizeof(renew);
        numBytesSent = send(c_socket, (char*)&renew, bufferLen, 0);
        
        if(numBytesSent == -1)
        {
             printf("Send Error\n");
             close(c_socket);
             return -1;
        }
        close(c_socket);
    }
}
