//temp code

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <time.h>
#include "types.h"

typedef struct Qdata {
	int size;
	char question[32];
	char answer[512];
	char detail[1024];
}Qdata;

char ranking[10][BUFSIZE];              // 랭킹 10위까지만 저장
struct Qdata qdata[4];
int user_grade;


void PacketManager(int c_socket, byte *buffer);
SP_Answer Search(byte detail, byte grade, char *keyword);
SP_RANK Rank_renew();



int main()
{
    pid_t pid;
    int c_socket, s_socket;
    struct sockaddr_in s_addr, c_addr;
    int len;
    char rcvBuffer[BUFSIZ];
    int n;
    
    //=================================================
    //  검색용 데이터 생성
    //=================================================
	sprintf(qdata[0].question, "%s", "wether");
	sprintf(qdata[0].answer, "%s", "sunny");
	sprintf(qdata[0].detail, "%s", "25 C, Humidity: 64%, Visibility: Very Good");

	sprintf(qdata[1].question, "%s", "");
	sprintf(qdata[1].answer, "%s", "");
	sprintf(qdata[1].detail, "%s", "");

	sprintf(qdata[2].question, "%s", "");
	sprintf(qdata[2].answer, "%s", "");
	sprintf(qdata[2].detail, "%s", "");

	sprintf(qdata[3].question, "%s", "");
	sprintf(qdata[3].answer, "%s", "");
	sprintf(qdata[3].detail, "%s", "");
    
    
    
    //=================================================
    // 소켓 전송 관련 초기화
    //=================================================
    s_socket = socket(PF_INET, SOCK_STREAM, 0);
    
    memset(&s_addr, 0, sizeof(s_addr));
    s_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(8000);
    
    
    //=================================================
    // 소켓 bind
    //=================================================
    if(bind(s_socket, (struct sockaddr*) &s_addr, sizeof(s_addr)) == -1)
    {
        printf("Can not Bind\n");
        return -1;        
    }
    
    //=================================================
    // listen 시작
    //=================================================
    if(listen(s_socket, 5) == -1)
    {
        printf("Listen Faile\n");
        return -1;
    }
    
    signal(SIGCHLD, SIG_IGN);                   // 좀비 프로세스 방지용 child process signal 무시
    while(1)
    {
        // 매 패킷 입력마다 새로운 연결 생성
        len = sizeof(c_addr);
        c_socket = accept(s_socket, (struct sockaddr*)&c_addr, &len);
        if(c_socket == -1)
        {
            printf("Accept Error\n");
            continue;
        }
        
        // fork 사용해 연결 독립적으로 생성
        if((pid = fork()) == -1){               // fork 함수 실패
            printf("fork error\n");
            close(c_socket);
            continue;
            
        }else if(pid > 0){                      // 부모 프로세스
            close(c_socket);
            continue;
            
        }else{                                  // 자식 프로세스 (실제 처리)
            close(s_socket);
            
            // 전송받은 데이터를 버퍼에 저장
            ssize_t numBytesRcvd = recv(c_socket, rcvBuffer, BUFSIZE, 0);
            if(numBytesRcvd == -1)
            {
                printf("Recv Error\n");
                return -1;
            }
            
            rcvBuffer[numBytesRcvd] = '\0';
            
            // 패킷 분류 함수 호출
            //printf("Server : %s\n", rcvBuffer);     // 디버깅용 패킷 표시
            PacketManager(c_socket, rcvBuffer);
            
            close(c_socket);
            exit(0);
        }
    }
    close(s_socket);
}

void PacketManager(int c_socket, byte *buffer){
    size_t packet_length = 0;           //전체 패킷 크기
    char *sndString;
    
    SP_Answer answer;
    SP_RANK rank;
    
    
    switch(buffer[0]){
        case CP_QUESTION:
            answer = Search(buffer[1], buffer[2], buffer+3);
            
            packet_length += sizeof(answer.type);
            packet_length += sizeof(answer.result);
            packet_length += sizeof(answer.detail);
            packet_length += strlen(answer.data);
            
            sndString = (char *)malloc(packet_length);
            
            sndString[0] = answer.type;
            sndString[1] = answer.result;
            sndString[2] = answer.detail;
            strcat(sndString, answer.data);
            
            break;
        case CP_RENEW:
            rank = Rank_renew();
            
            packet_length += sizeof(rank.type);
            packet_length += strlen(rank.renew_time);
            packet_length += strlen(rank.data);
            
            sndString = (char *)malloc(packet_length);
            
            sndString[0] = rank.type;
            strcat(sndString, rank.renew_time);
            strcat(sndString, rank.data);
            
            break;
        default:
            printf("잘못된 패킷 형태\n");
            return;
    }
    
    ssize_t numBytesSent = send(c_socket, sndString, packet_length, 0);
    //디버그용 메시지
    printf("\n-Send-\n packet:%s length:%ud Sentsize:%ud\n", sndString, (unsigned int)packet_length, (unsigned int)numBytesSent);

    //numBytesSent에는 send한 패킷의 크기가 반환되며 실패시 -1이 반환
    if(numBytesSent == -1)
    {
            printf("Send Error\n");
            return;
    }
}


SP_Answer Search(byte detail, byte grade, char *keyword){
    //디버그용 메시지
    printf("\n-Search-\n detail:%c grade:%c keyword:%s\n",detail, grade, keyword);
    
    SP_Answer answer;
    //=================================================
    // 입력 키워드로 검색어 순위(ranking[10][]) 갱신
    //=================================================
    
    
    
    
    //=================================================
    // 자료 검색 후 결과는 SP_Answer형으로 반환
    //=================================================
    answer.type = SP_ANSWER;
    answer.result = ANSWER_NOTFOUND;
    answer.detail = LOW;
    sprintf(answer.data, "NOTFOUND");
    
    return answer;
}


SP_RANK Rank_renew(){
    SP_RANK rank;

    //=================================================
    // ranking[10][] 읽어와서 SP_RANK형으로 반환
    //=================================================
    
    //ranking[10][BUFSIZE];
    return rank;
}
