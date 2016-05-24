#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "types.h"

SP_Answer SplitPlainMsg(char *rcvBuffer);
SP_Alternative SplitModifiedMsg(char *rcvBuffer);
void PlainAnswerHandler(SP_Answer recvAnswer);
void ModifiedAnswerHandler(SP_Alternative recvModified);


void ClientRecvAnswer(int c_socket, char *rcvBuffer){
	int bytesRcvd;
	SP_Answer recvAnswer;
	SP_Alternative recvModified;

	printf("Answer>>");
	/***********송신**************/
	if((bytesRcvd = recv(c_socket, rcvBuffer, BUFSIZE -1, 0)) < 0){
		perror("recv() failed");
		exit(1);
	}
	rcvBuffer[bytesRcvd] = '\0';
	//		printf(rcvBuffer);
	close(c_socket);

	/*********타입에 따른 처리***************/
	switch(rcvBuffer[0]){
		case SP_ANSWER:// plain answer
			recvAnswer = SplitPlainMsg(rcvBuffer);// 문자열 -> 구조체
			PlainAnswerHandler(recvAnswer);// 구조체 처리
			break;
		case '1':// modified answer
			recvModified = SplitModifiedMsg(rcvBuffer);// 문자열 -> 구조체
			ModifiedAnswerHandler(recvModified);// 구조체 처리
			break;
		case SP_UNI:// renew answer
			break;
	}
}

// 일반 응답 -> 구조체
SP_Answer SplitPlainMsg(char *rcvBuffer){
	SP_Answer recvAnswer;
	recvAnswer.type = *rcvBuffer++;
	recvAnswer.result = *rcvBuffer++;
	recvAnswer.detail = *rcvBuffer++;
	strcpy(recvAnswer.data, rcvBuffer);
	//	recvAnswer.data = rcvBuffer;
	return recvAnswer;
}

// 수정 응답 -> 구조체
SP_Alternative SplitModifiedMsg(char *rcvBuffer){
	SP_Alternative recvModified;
	recvModified.type = *rcvBuffer++;
	recvModified.accuracy = *rcvBuffer++;
	strcpy(recvModified.data, rcvBuffer);
//	recvModified.data = rcvBuffer;
	return recvModified;
}

// 일반 응답 처리
void PlainAnswerHandler(SP_Answer recvAnswer){
	switch(recvAnswer.result){
		case ANSWER_SUCCESS:// Success
			printf(" Detail LV : %c\n\t %s\n", recvAnswer.detail, recvAnswer.data);
			break;
		case ANSWER_NOTFOUND:// Not Found
			printf(" 요청에 대한 결과를 찾을 수 없습니다.\n");
			break;
		case ANSWER_GRADELOW:// Permission deny
			printf(" 표시할 수 없는 응답입니다.\n");
			break;
	}
}

// 수정 응답 처리
void ModifiedAnswerHandler(SP_Alternative recvModified){
	switch(recvModified.accuracy-'0'){
		
	}	
}
