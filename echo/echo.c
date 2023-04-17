/*
 * echo - read and echo text lines until client closes connection
 */
/* $begin echo */
#include "csapp.h"

void echo(int connfd) 
{
    size_t n; 
    char buf[MAXLINE]; 
    rio_t rio;

    // connfd에서 버퍼를 DMA 방식으로 메모리에서 적제 
    Rio_readinitb(&rio, connfd);
    while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) { //line:netp:echo:eof
	printf("server received %d bytes\n", (int)n);
    // connfd에 버퍼 쓰기
	Rio_writen(connfd, buf, n);
    }
}
/* $end echo */
