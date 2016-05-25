#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "types.h"

SP_Answer splitPlainMsg(char *rcvBuffer);/*버퍼 내용을 구조체로 변환하는 함수*/
SP_Alternative splitModifiedMsg(char *rcvBuffer);/*버퍼 내용을 구조체로 변환하는 함수*/

void plainAnswerHandler(SP_Answer rcvdAnswer);/*기본 응답 처리*/
void modifiedAnswerHandler(SP_Alternative rcvdModified);/*수정 응답 처리*/

/* 서버의 메세지를 Type에 따라 분류하는 함수 */
void typeCheckerRcvdMsg(char *rcvBuffer){

//	int bytesRcvd;/*수신된 버퍼의 크기*/

	SP_Answer rcvdAnswer;/*기본 응답 메세지 구조체*/
	SP_Alternative rcvdModified;/*수정 응답 메세지 구조체*/

	/* 소켓 수신 */
//	if((bytesRcvd = recv(c_socket, rcvBuffer, BUFSIZE -1, 0)) < 0){
//		perror("recv() failed");/*수신 실패 처리*/
//		exit(1);
//	}
	
//	rcvBuffer[bytesRcvd] = '\0';/*문자열 마지막 NULL char 삽입*/
//	close(c_socket);
	

	printf("Answer>>");
	/* TYPE 헤더에 따른 처리 */
	switch(rcvBuffer[0]){
		case SP_ANSWER:/* plain answer */
			rcvdAnswer = splitPlainMsg(rcvBuffer);/*버퍼 내용을 구조체로 변환*/
			plainAnswerHandler(rcvdAnswer);/*구조체를 이용한 기본응답 처리*/
			break;

		case SP_MODI:/* modified answer */
			rcvdModified = splitModifiedMsg(rcvBuffer);/*버퍼 내용을 구조체로 변환*/
			modifiedAnswerHandler(rcvdModified);/*구조체를 이용한 수정응답 처리*/
			break;

		case SP_UNI:/* renew answer */
			break;
	}
}

/*기본응답: 버퍼 내용을 구조체로 변환*/
SP_Answer splitPlainMsg(char *rcvBuffer){
	SP_Answer rcvdAnswer;

	rcvdAnswer.type = *rcvBuffer++;
	rcvdAnswer.result = *rcvBuffer++;
	rcvdAnswer.detail = *rcvBuffer++;
	strcpy(rcvdAnswer.data, rcvBuffer);

	return rcvdAnswer;
}

/*수정응답: 버퍼 내용을 구조체로 변환*/
SP_Alternative splitModifiedMsg(char *rcvBuffer){
	SP_Alternative rcvdModified;

	rcvdModified.type = *rcvBuffer++;
	strcpy(rcvdModified.data, rcvBuffer);

	return rcvdModified;
}

/*기본응답: 구조체를 이용한 기본응답 처리*/
void plainAnswerHandler(SP_Answer rcvdAnswer){

	switch(rcvdAnswer.result){
		case ANSWER_SUCCESS:/*Success*/
			printf(" Detail LV : %c\n\t %s\n", rcvdAnswer.detail, rcvdAnswer.data);
			break;

		case ANSWER_NOTFOUND:/*Not Found*/
			printf(" 요청에 대한 결과를 찾을 수 없습니다.\n");
			break;

		case ANSWER_GRADELOW:/*Permission deny*/
			printf(" 표시할 수 없는 응답입니다.\n");
			break;
	}
}

/*수정응답: 구조체를 이용한 수정응답 처리*/
void modifiedAnswerHandler(SP_Alternative rcvdModified){
	
		// 여기에 구현하시면 됩니다.
		// 기본적으로 받아온 rcvdModified를 출력하는 형태입니다.
}
