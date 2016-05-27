/*
*   Server.c
*   전현덕 김기윤 표지혜
*   
*   Warning
*   Linux 기반이므로 window 환경에서는 실행이 되지 않습니다.
*   
*   참고사항
*   1. 모든 패킷은 문자열 상태로 전송됨. 기본적으로 패킷을 받아서 문자열의 1byte는 type을 나타내기 때문에 이를 
*   구분하여 내구 동작을 작성하여야 함. 따로 함수를 만들어도 되고 main문에서 처리하여도 되나 코드를 읽기 쉽게
*   함수로 구현하는 쪽이 좋음. 대부분 구조체 형태는 types.h에 명시되어 있으니 참고. 어떤 형식으로 전송되는 지
*   알기 쉽게 이해하려면 서버와 클라이언트 둘다 실행시켜 보면 됨. 테스트 출력칸 확인.   
*
*   서버에서 구현해야 하는 기능 (Search(), Rank_renew() 함수 구현)
*   1. 질문받은 내용을 검색하여 그에 맞는 결과를 전송(성공/실패/등급부족, 상세도별로 전송내용 추가)
*   2. 만약 수정된 질문을 보내야 하면 정확도 순으로 합쳐서 전송 
*   3. 갱신 패킷이 오면 클라이언트로 미리 정의되어 있는 검색어 순위 전송
*
*   대략적인 순서(정확하지 않음)
*   질문, 갱신 패킷 수신 -> 해당 키워드로 검색어 순위 갱신 -> 갱신된 검색어 순위 전송 -> 
*   검색 결과 종류에 따라 패킷 전송 -> 다음 질문 대기
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
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include "types.h"

int ranking_mem;
Rank_data *ranking;              // 랭킹 RANKMAX(10위)까지만 저장
struct Qdata qdata[5];
int user_grade;

void getTime();
void PacketManager(int c_socket,char buffer[],int s_socket);
void Rank_calc(char *keyword);
void signalHandler(int signo);
SP_Answer Search(byte detail, byte grade, char *keyword,int c_socket,int
		s_socket);
SP_Alternative Modify(char *keyword);
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
    //  검색용 데이터, 검색 순위 초기화
    //=================================================
	sprintf(qdata[0].question, "%s", "weather");
	sprintf(qdata[0].answer_low, "%s", "sunny");
	sprintf(qdata[0].answer_mid, "%s", "25 C, Humidity: 64%, Visibility: Very Good");
	sprintf(qdata[0].answer_high,"%s","Today's weather is sunny, 25 C. Humidity: 65%, Visibility : Very Good");

	sprintf(qdata[1].question, "%s", "where");
	sprintf(qdata[1].answer_low, "%s", "Republic of Korea");
	sprintf(qdata[1].answer_mid, "%s", "Daegu, Republic of Korea");
	sprintf(qdata[1].answer_high,"%s", "Dalseogu, Daegu, Republic of Korea");

	sprintf(qdata[2].question, "%s", "seoul");
	sprintf(qdata[2].answer_low, "%s", "Capital of Korea");
	sprintf(qdata[2].answer_mid, "%s", "Capital of Korea, Population 10,000,000");
	sprintf(qdata[2].answer_high,"%s","Capital of Korea, Population 10,000,000, GDP : US$ 34,355");

	sprintf(qdata[3].question, "%s", "kmu");
	sprintf(qdata[3].answer_low, "%s", "Located in Dalseogu, Daegu, Republic of Korea");
	sprintf(qdata[3].answer_mid, "%s", "Located : Dalseogu,Daegu, Type :	Private");
	sprintf(qdata[3].answer_high,"%s","Located : Dalseogu,Daegu, Type : Private, Student 27,000");
    
    // =============================================================================
    // 공유메모리 초기화
    // =============================================================================
    if((ranking_mem = shmget((key_t)1234, (sizeof(Rank_data) * RANKMAX), IPC_CREAT|0666)) == -1) {
       perror("shmget failed");
       exit(1);
    }

    if((ranking = shmat(ranking_mem, (void *)0, 0)) == (void *)-1) {
       perror("shmat failed");
       exit(1);
    }

    memset((char *)ranking, 0, (sizeof(Rank_data) * RANKMAX));
    
    for(n = 0; n < RANKMAX; ++n){
        ranking[n].total = 0;
        sprintf(ranking[n].keyword, "결과 없음");
    }
    n = 0;
    
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
    }else{
		 getTime();
		 printf("Bind\n");
	 }
    
    //=================================================
    // listen 시작
    //=================================================
    if(listen(s_socket, 5) == -1)
    {
        printf("Listen Failed\n");
        return -1;
    }else{
		 getTime();
		 printf("Listen\n");
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
            }else{
					getTime();
					printf("Received\n");
				}
            
            rcvBuffer[numBytesRcvd] = '\0';
            // =============================================================================
            // 공유메모리 처리
            // =============================================================================
            if((ranking_mem=shmget((key_t)1234, (sizeof(Rank_data) * RANKMAX), IPC_CREAT|0666)) == -1) {
                perror("shmget failed");
                exit(1);
            }
            if(shmdt(ranking) == -1) {
                perror("shmdt failed");
                exit(1);
            }
            if((ranking=shmat(ranking_mem, (void *)0, 0)) == (void *)-1) {
                perror("shmat failed");
                exit(1);
            }
            
            // 패킷 분류 함수 호출
            printf("Server : %s\n", rcvBuffer);     // 디버깅용 패킷 표시
            PacketManager(c_socket, rcvBuffer,s_socket);
            
            // =============================================================================
            // 공유메모리 분리
            // =============================================================================
            if(shmdt(ranking) == -1) {
                perror("shmdt failed");
                exit(1);
            }
            
            
            close(c_socket);
            exit(0);
        }
    }
    close(s_socket);
    
    if(shmdt(ranking) == -1) {
       perror("shmdt failed");
       exit(1);
    }
    if(shmctl(ranking_mem, IPC_RMID, 0) == -1) {
       perror("shmctl failed");
       exit(1);
    }


}

void getTime(){
	time_t s_time;
	struct tm *day;
	
	time(&s_time);
	day=localtime(&s_time);
	printf("[%02d:%02d:%02d] Server : ",day->tm_hour,day->tm_min,day->tm_sec);
}

void PacketManager(int c_socket, char buffer[],int s_socket){
    size_t packet_length = 0;           //전체 패킷 크기
    char *sndString;
    
    SP_Answer answer;
    SP_Alternative modify;
    SP_RANK rank;
    
    //printf("Type : ");
    switch(buffer[0]){
        case CP_QUESTION: //질문
            Rank_calc(buffer+3);
				//printf("Question\n");
            answer = Search(buffer[1], buffer[2], buffer+3, c_socket,s_socket);
            
            packet_length += sizeof(answer.type);
            packet_length += sizeof(answer.result);
            packet_length += sizeof(answer.detail);
            packet_length += strlen(answer.data);
            
            sndString = (char *)malloc(packet_length);
            memset(sndString, 0, packet_length);
            
            sndString[0] = answer.type;
            sndString[1] = answer.result;
            sndString[2] = answer.detail;
            strcat(sndString, answer.data);
            break;
        case CP_RENEW:
            rank = Rank_renew();
				printf("Renew\n");
            
            packet_length += sizeof(rank.type);
            packet_length += strlen(rank.renew_time);
            packet_length += strlen(rank.data);
            
            sndString = (char *)malloc(packet_length);
            memset(sndString, 0, packet_length);
            
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
    //printf("\n(Server)-Send-\n packet:'%s' length:%d Sentsize:%d\n", sndString, (int)packet_length, (int)numBytesSent);

    //numBytesSent에는 send한 패킷의 크기가 반환되며 실패시 -1이 반환
    if(numBytesSent == -1){
		 printf("Send Error\n");
		 return;
    }
}

void Rank_calc(char *keyword){
    int exist = 0, i, j;
    
    
    // 이미 검색어 순위에 있는 경우
    for(i = 0; i < RANKMAX; ++i){
        if(strcmp(ranking[i].keyword, keyword) == 0){
            //printf("%s founded", ranking[i].keyword);
            ranking[i].total += 11;
            exist = 1;
            break;
        }
    }
    
    if(exist == 0){
        // 기존 검색어 비중 낮추기
        for(i = 0; i < RANKMAX; ++i){
            if(ranking[i].total > 0) ranking[i].total--;
        } 
        for(i = 0; i < RANKMAX; ++i){
            if(ranking[i].total == 0){
                ranking[i].total += 11;
                sprintf(ranking[i].keyword, "%s", keyword);
                // printf("%s added", ranking[i].keyword);
                break;
            }
        }
    }
    
    // 순위 정렬
    
    for(i = 0; i < RANKMAX; ++i){
        for(j = 0; j < RANKMAX - 1; ++j){
            if(ranking[j].total < ranking[j+1].total){
                Rank_data tmp = ranking[j];
                ranking[j] = ranking[j+1];
                ranking[j+1] = tmp;
            }
        }
    }
    
    //디버그용 메시지
    // for(i = 0; i < RANKMAX; ++i){
    //     printf("%d, %s\n", ranking[i].total, ranking[i].keyword);
    // }
    
    
}

SP_Answer Search(byte detail, byte grade, char *keyword,int c_socket, int s_socket){
	//디버그용 메시지
	//printf("\n(Server)-Search-\n detail:%c grade:%c keyword:'%s'\n",detail, grade, keyword);

	int count=1;
	int i;
	pid_t pid;
	SP_Answer answer;
	SP_Alternative alter;
	answer.type=SP_ANSWER;
	answer.detail=detail;
	char rcvBuffer[BUFSIZ];

	for(i=0;i<sizeof(qdata)/sizeof(qdata[1])+1;i++){
		//question 확인용 메세지
		//printf("%s\n",qdata[i].question);
		if(!strcmp(keyword,qdata[i].question)){ //원하는 값이 있을때
			answer.result=ANSWER_SUCCESS;
			if(detail==LOW){
				sprintf(answer.data,"%s",qdata[i].answer_low);
				return answer;

			}else if(detail==MID){
				sprintf(answer.data,"%s",qdata[i].answer_mid);
				return answer;
			}
			else if(detail==HIG){
				sprintf(answer.data,"%s",qdata[i].answer_high);
				return answer;
			}
		}
		else{ //원하는 값을 찾지 못했을때
			count++;
		}

		if(count==sizeof(qdata)/sizeof(qdata[1])){ //마지막까지 원하는 값을 찾지 못했을때
			//printf("final count : %d\n",count);
			//printf("fail\n");

			if((pid=fork())==-1){
				printf("Fork Error\n");
				continue;
			}else if(pid>0){ //부모 프로세스
				answer.result=ANSWER_NOTFOUND;
				sprintf(answer.data,"NOTFOUND");
				return answer;
			}else{ //자식 프로세스
				alter=Modify(keyword);

				char modiBuffer[BUFSIZ];

				size_t packet_length=0;
				char *modString;

				packet_length+=sizeof(alter.type);
				packet_length+=strlen(alter.data);

				modString=(char *)malloc(packet_length);
				memset(modString,0,packet_length);

				modString[0] = alter.type;

				strcat(modString,alter.data);



				printf("before send  and [%s]\n", modString);
				ssize_t numBytesModiSent=send(c_socket,modString,packet_length,0);
				if(numBytesModiSent==-1){
					getTime();
					printf("Send Error\n");

					exit(1);
				}                               
			}
		}
	}
}

SP_Alternative Modify(char *keyword){
	SP_Alternative alter;
	int i;
	char st[1024];
	char copy_st[1024];
	int count=0;
	int nofound=0;
	alter.type=SP_MODIFY;

	for(i=0;i<sizeof(qdata)/sizeof(qdata[1]);i++){
		if(strstr(qdata[i].question,keyword)){
			count++;
			sprintf(st,"%d",count);
			strcat(st,". ");
			strcat(st,qdata[i].question);
			strcat(st,"|");
			printf("%s",st);
			strcat(copy_st,st);
		}else{
			nofound++;
		}
	}
	if(nofound==sizeof(qdata)/sizeof(qdata[1])){
		sprintf(alter.data,"%s","NO PROPOSED MODIFICATION");
		printf("alter : %s",alter.data);
		return alter;
	}else{
		printf("st : %s",st);
		sprintf(alter.data,"%s",copy_st);
		return alter;
	}
}

SP_RANK Rank_renew(){
    //디버그용 메시지
    //printf("\n(Server)-Renew-\n ranking[0].total:%d, ranking[0].keyword:'%s'\n", (int)ranking[0].total, ranking[0].keyword);
    
    SP_RANK rank;
    int i;
    time_t tm_time;
    struct tm *st_time;

    time( &tm_time);
    st_time = localtime( &tm_time);
    //=================================================
    // ranking[10] 읽어와서 SP_RANK형으로 반환
    //=================================================
    rank.type = SP_UNI_RANK;
    
    strftime(rank.renew_time, 20, "%Y/%m/%d/%H/%M/%S", st_time);
    for(i = 0; i < RANKMAX; ++i){
        
        // sprintf(rank.data, "%s%c", rank.data, TOKEN);
        // sprintf(rank.data, "%s%c", rank.data, ranking[i].total);
        sprintf(rank.data, "%s%c", rank.data, TOKEN);
        sprintf(rank.data, "%s%s", rank.data, ranking[i].keyword);
        
    }
    
    //ranking[10][BUFSIZE];
    return rank;
}
