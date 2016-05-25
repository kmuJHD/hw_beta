/*
*   CP Client Process
*   SP Server Process
*
*
*/


//Client 에서 보내는 패킷 type 
#define CP_QUESTION '0'
#define CP_RENEW '1'

//상세수준
#define LOW '0'
#define MID '1'
#define HIG '2'

//등급
#define FIR '0'
#define SEC '1'
#define THI '2'

#define SP_ANSWER '0'
#define SP_MODI '1'
#define SP_UNI '2'

#define SP_UNI_KEYWORD '0'
#define SP_UNI_RANK '1' 

#define ANSWER_SUCCESS '0'
#define ANSWER_NOTFOUND '1'
#define ANSWER_GRADELOW '2' 

//버프 크기
#define BUFSIZE 1024
// 연속되는 문자열 구분자
#define TOKEN '|'


typedef unsigned char byte; 

typedef struct Packet{
    byte type;
    byte *data;
}Packet;

typedef struct CP_Question{
    byte type;
    byte detail;
    byte grade;
    byte data[BUFSIZE];
}CP_Question;

typedef struct CP_Renew{
    byte type;
} CP_Renew;

typedef struct SP_Answer{
	byte type;
	byte result;
	byte detail;
    byte data[BUFSIZE];
}SP_Answer;

typedef struct SP_Alternative{
    byte type;
    byte data[BUFSIZE];
}SP_Alternative;

typedef struct SP_RANK{
	byte type;
    byte renew_time[BUFSIZE];
    byte data[BUFSIZE];
}SP_RANK;
