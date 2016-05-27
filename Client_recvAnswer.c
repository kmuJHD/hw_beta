#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "types.h"

SP_Answer splitPlainMsg(char *rcvBuffer);/*버퍼 내용을 구조체로 변환하는 함수*/
SP_Alternative splitModifiedMsg(char *rcvBuffer);/*버퍼 내용을 구조체로 변환하는 함수*/

void plainAnswerHandler(SP_Answer rcvdAnswer, int *loop);/*기본 응답 처리*/
void modifiedAnswerHandler(SP_Alternative rcvdModified);/*수정 응답 처리*/

/* 서버의 메세지를 Type에 따라 분류하는 함수 */
void typeCheckerRcvdMsg(char *rcvBuffer, int *loop){

	SP_Answer rcvdAnswer;/*기본 응답 메세지 구조체*/
	SP_Alternative rcvdModified;/*수정 응답 메세지 구조체*/


	/* TYPE 헤더에 따른 처리 */
	switch(rcvBuffer[0]){

		case SP_ANSWER:/* plain answer */

			printf("Answer>> ");
			/* 버퍼의 내용을 구조체로 변환 */
			rcvdAnswer = splitPlainMsg(rcvBuffer);

			/* 구조체를 이용한 기본응답 처리 */
			plainAnswerHandler(rcvdAnswer, loop);
			
			break;

		case SP_MODIFY:/* modified answer */


			/* 버퍼 내용을 구조체로 변환 */
			rcvdModified = splitModifiedMsg(rcvBuffer);

			/* 구조체를 이용한 수정응답 처리 */
			modifiedAnswerHandler(rcvdModified);
			*loop = 0;
			
			break;

		default:
			return;
	}
}

/*기본응답: 버퍼 내용을 구조체로 변환*/
SP_Answer splitPlainMsg(char *rcvBuffer){

	SP_Answer rcvdAnswer;
	
	/* type 처리 */
	rcvdAnswer.type = *rcvBuffer++;

	/* result 처리 */
	rcvdAnswer.result = *rcvBuffer++;

	/* detail 처리 */
	rcvdAnswer.detail = *rcvBuffer++;

	/* data 처리 */
	strcpy(rcvdAnswer.data, rcvBuffer);

	return rcvdAnswer;
}

/*수정응답: 버퍼 내용을 구조체로 변환*/
SP_Alternative splitModifiedMsg(char *rcvBuffer){

	SP_Alternative rcvdModified;

	/* type 처리 */
	rcvdModified.type = *rcvBuffer++;

	/* data 처리 */
	strcpy(rcvdModified.data, rcvBuffer);

	return rcvdModified;
}

/*기본응답: 구조체를 이용한 기본응답 처리*/
void plainAnswerHandler(SP_Answer rcvdAnswer, int *loop){

	/* result 값에 따른 처리 */
	switch(rcvdAnswer.result){

		case ANSWER_SUCCESS:/* Success */
			printf(" Detail LV : %c\n\t %s\n", rcvdAnswer.detail, rcvdAnswer.data);
			break;

		case ANSWER_NOTFOUND:/* Not Found */
			printf(" 요청에 대한 결과를 찾을 수 없습니다.\n");
			*loop = 1;
			break;

		case ANSWER_GRADELOW:/* Permission deny */
			printf(" 표시할 수 없는 응답입니다.\n");
			break;
	}
}

/*수정응답: 구조체를 이용한 수정응답 처리*/
void modifiedAnswerHandler(SP_Alternative rcvdModified){
	
	char *strToken;
	
	printf("\t<< 다른 검색어 제안 >>\n");

	/* TOKEN으로 구분 */
	strToken = strtok(rcvdModified.data, "|");

	while(strToken != NULL){
		printf("\t  %s\n", strToken);
		strToken = strtok(NULL, "|");
	}

}
