/*
 * adder.c - a minimal CGI program that adds two numbers together
 */
/* $begin adder */
#include "csapp.h"

int main(void) {
    char *buf, *p;
    char arg1[MAXLINE], arg2[MAXLINE], content[MAXLINE];
    int n1=0, n2=0;

    /* Extract the two arguments */
    if ((buf = getenv("QUERY_STRING")) != NULL) {
	p = strchr(buf, '&');
	*p = '\0';
  
	// strcpy(arg1, buf+8);
	// strcpy(arg2, p+9);
  sscanf(buf, "number1=%d", &n1);
	sscanf(p+1, "number2=%d", &n2);
  // atoi : string을 int 형으로 변환
	// n1 = atoi(arg1);
	// n2 = atoi(arg2);
    }
  // html 화면에 출력
    /* Make the response body */
    sprintf(content, "Welcome to add.com: ");
    sprintf(content, "%sTHE Internet addition portal.\r\n<p>", content);
    sprintf(content, "%sThe answer is: %d + %d = %d\r\n<p>", 
	    content, n1, n2, n1 + n2);
    sprintf(content, "%sThanks for visiting!\r\n", content);
  
  // 개발자 도구 네트워크 탭 응답 헤더
    /* Generate the HTTP response */
    printf("Connection: close\r\n");
    printf("Content-length: %d\r\n", (int)strlen(content));
    printf("Content-type: text/html\r\n\r\n");
    printf("%s", content);
    fflush(stdout);

    exit(0);
}
/* $end adder */