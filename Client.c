/*
*   Client.c
*   홍현표 김상원 조심재
*   Warning
*   Linux 기반이므로 window 환경에서는 실행이 되지 않습니다.
*   
*   참고사항
*   1. 모든 패킷은 문자열 상태로 전송됨. 기본적으로 패킷을 받아서 문자열의 1byte는 type을 나타내기 때문에 이를 
*   구분하여 내구 동작을 작성하여야 함. 따로 함수를 만들어도 되고 main문에서 처리하여도 되나 코드를 읽기 쉽게
*   함수로 구현하는 쪽이 좋음. 대부분 구조체 형태는 types.h에 명시되어 있으니 참고. 어떤 형식으로 전송되는 지
*   알기 쉽게 이해하려면 서버와 클라이언트 둘다 실행시켜 보면 됨. 테스트 출력칸 확인.   
*
*   클라이언트에서 구현해야 하는 기능
*   1. 질문에 대한 응답을 받아 결과와 수준을 확인하여 그에 맞게 출력
*   2. 질문에 대해서 서버가 수정된 질문을 보내올 경우 그 패킷에 들어있는 데이터를 분리해서 정확도 순으로 출력
*   3. 갱신 패킷이 오면 2번과 마찮가지로 데이터를 추출해 순위순으로 출력
*
*   대략적인 순서(정확하지 않음)
*   질문, 갱신 패킷 전송 -> 응답 & 갱신 수락 패킷 수신 -> 응답 패킷 헤더 체크후 그에 맞게 출력 ->
*   갱신 패킷 체크 후 출력 -> 다음 질문 대기
*
*
*   참고(최신 갱신 코드) - https://github.com/kmuJHD/hw_beta.git
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include "types.h"

void SendQuestion();
void ReNew();

void Client_recvAnswer(int c_socket, char *rcvBuffer);
//void Client_recvReNew(int c_socket);

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


int c_socket;
struct sockaddr_in c_addr;
char sndBuffer[BUFSIZ], rcvBuffer[BUFSIZ];



main()
{
    int len;
        
    memset(&c_addr, 0, sizeof(c_addr));
    c_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    c_addr.sin_family = AF_INET;
    c_addr.sin_port = htons(8000);
    
    
    while(1)
    {     
        //질문을 입력받아 sndBuffer에 입력
        printf("질문 : ");
        fgets(sndBuffer, sizeof(sndBuffer), stdin); 
    
        //fgets로 가져온 문자열을 끝에 줄바꿈문자가 들어가기 때문에 줄바굼 문자를 제거
        sndBuffer[strlen(sndBuffer) - 1] = '\0';
                
        SendQuestion(c_addr);
  
    }
}

void SendQuestion()
{
    CP_Question question;
    
    // 매 패킷 전송마다 새로 연결 생성
    c_socket = socket(PF_INET, SOCK_STREAM, 0);
    if(connect(c_socket, (struct sockaddr*)&c_addr, sizeof(c_addr)) == -1)
    {
        printf("Can not connect\n");
        close(c_socket);
        return;
    }
   
    //------보내고자 하는 패킷을 구성(예제)------//
    question.type = '0';
    question.detail = '0';
    strcpy(question.data, sndBuffer);
    question.grade = '0';
    
        
    //어디까지나 예제로 변경될 수 있으나 기본적으로 문자열 형태로 변환하여 서버에 전송함
    int strlength = 0;
        
    strlength += sizeof(question.type);
    strlength += sizeof(question.detail);
    strlength += strlen(question.data);
    strlength += sizeof(question.grade);
               
    char *sndString;
        
    sndString = (char *)malloc(strlength);
        
    sndString[0] = question.type;
    sndString[1] = question.detail;
    sndString[2] = question.grade;
    strcat(sndString, question.data);        
     
    //test용 출력 - 동적 할당된 문자열
    printf("(Client)-Allocated String-\n%s %d\n", sndString, strlength);
    
    wait(1);
    
    ssize_t numBytesSent = send(c_socket, sndString, strlength, 0);
   
    //numBytesSent에는 send한 패킷의 크기가 반환되며 실패시 -1이 반환
    if(numBytesSent == -1)
    {
         printf("Send Error\n");
         close(c_socket);
         return;
    }
    
    ssize_t numBytesRcvd = recv(c_socket, rcvBuffer, BUFSIZE, 0);
    if(numBytesRcvd == -1)
    {
        printf("Recv Error\n");
    }else{
        rcvBuffer[numBytesRcvd] = '\0';
        
        // 디버깅용 패킷 표시   
        printf("\n(Client)-Response Packet-\nPacketData : %s\n", rcvBuffer);
    }
    /*
    while(1)
    {
        //recv 동작에 대해 응답 패킷 또는 수정 패킷에대한 분류 및 출력구현(while문 내부에 구현 - 함수로 만들어도 상관 없음)
      */
        Client_recvAnswer(c_socket, rcvBuffer);
      /*  
        
    }
    */
       
    close(c_socket);
}

void ReNew()
{
    CP_Renew renew;
    
    // 매 패킷 전송마다 새로 연결 생성
    c_socket = socket(PF_INET, SOCK_STREAM, 0);
    if(connect(c_socket, (struct sockaddr*)&c_addr, sizeof(c_addr)) == -1)
    {
        printf("Can not connect\n");
        close(c_socket);
        return;
    }
    
    //------보내고자 하는 패킷을 구성------//
    renew.type = '1';
    //-------------CP_RENEW-----------//
              
    
    //RENEW 구조체에 대한 패킷 전송 / 실제 1byte 크기
    size_t bufferLen = sizeof(renew);
    ssize_t numBytesSent = send(c_socket, (char*)&renew, bufferLen, 0);
        
    printf("%d\n", (int)numBytesSent);
        
    if(numBytesSent == -1)
    {
         printf("Send Error\n");
         close(c_socket);
         return;
    }
    
    /*
    while(1)
    {
        //recv 동작에 대해 갱신 패킷 data를 분리하여 순위대로 출력구현(while문 내부에 구현 - 함수로 만들어도 상관 없음)
        
    }
    */
    
    close(c_socket);
}
