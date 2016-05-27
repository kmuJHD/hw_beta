#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "types.h"

SP_RANK splitRenewMsg(char *rcvBuffer);/* ReNew 메세지를 구조체로 변환하는 함수 */
void recvRenewHandler(SP_RANK rcvdRenew);/* SP_RANK 구조체를 이용해 내용을 처리하는 함수 */

/* ReNew 메세지의 Type에 따른 처리를 하는 함수 */
void typeCheckerRcvdRenew(char *rcvBuffer){

	SP_RANK rcvdRenew;

	/* 타입에 따른 분기 */
	switch(rcvBuffer[0])
	{
		case SP_UNI_RANK:/* SP_UNI_RANK = '1' */

			/* ReNew 메세지를 구조체로 변환 */
			rcvdRenew = splitRenewMsg(rcvBuffer);

			/* SP_RANK 구조체를 이용해 내용 출력 */
			recvRenewHandler(rcvdRenew);
			break;
	
		default:/* 기능 미정 */
			return;
	}

}

/* ReNew 메세지를 구조체로 변환하는 함수 */
SP_RANK splitRenewMsg(char *rcvBuffer){

	SP_RANK rcvdRenew;

	int i = 0;
	char *strBuffer;

	/* type 처리 */
	rcvdRenew.type = *rcvBuffer++;

	/* 시간, data 처리부 */
	while(*rcvBuffer != '\0')
	{
		/* 파이프를 처음 만났을 때 처리 완료 */
		if(*rcvBuffer == '|'){
			/* 시간 처리 */
			strcpy(rcvdRenew.renew_time, strBuffer);

			/* data 처리 */
			strcpy(rcvdRenew.data, rcvBuffer);

			free(strBuffer);
			return rcvdRenew;
		}
		/* 파이프 앞의 문자열 저장 */
		strBuffer = (char *)realloc(strBuffer ,sizeof(char)*(i+1));
		strBuffer[i++] = *rcvBuffer++;
	}
	/* 위의 조건문에서 반환되지 못했을 경우 실패 */
	sprintf(rcvdRenew.renew_time, "%s", "FAILED!");
	sprintf(rcvdRenew.data, "%s", "FAILED!");
	return rcvdRenew;
}

/* SP_RANK 구조체를 이용해 내용을 처리하는 함수 */
void recvRenewHandler(SP_RANK rcvdRenew){
	int i = 1;
	char *strToken;

	printf("\t<< 정보갱신 >>\n");

	/* 파이프를 기준으로 문자열 분리 */
	strToken = strtok(rcvdRenew.data, "|");

	/* 순위 출력 */
	while(strToken != NULL){
		printf("\t%2d. %s\n", i++, strToken);
		strToken = strtok(NULL, "|");
	}

	printf("    [ 갱신시간-->%s ]\n", rcvdRenew.renew_time);
}
