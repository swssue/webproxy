/* $begin tinymain */
/*
 * tiny.c - A simple, iterative HTTP/1.0 Web server that uses the
 *     GET method to serve static and dynamic content.
 *
 * Updated 11/2019 droh
 *   - Fixed sprintf() aliasing issue in serve_static(), and clienterror().
 */
#include "csapp.h"
#include <stdio.h>

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
    "Firefox/10.0.3\r\n";

void doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *hostname, char *port, char* path);
int make_header(char* hostname, char* port, char* path, rio_t rio, char* makeHeader);

int main(int argc, char **argv) {
  // listenfd : client socket open, connfd : client connect, server_listenfd : server socket open
  int listenfd, connfd, server_listenfd;
  
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;

  /* Check command line args */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  listenfd = Open_listenfd(argv[1]);
  while (1) {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);  // line:netp:tiny:accept
    // 소켓 구조체를 호스트와 서비스이름 스트링으로 바꾸기
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
    printf("Accepted connection from (%s, %s)\n", hostname, port);
    doit(connfd);   // line:netp:tiny:doit
    Close(connfd);  // line:netp:tiny:close
  }
}

void doit(int fd)
{
  int is_static, serverfd, server_connfd;
  struct stat sbuf;
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char filename[MAXLINE], cgiargs[MAXLINE],temp[MAXLINE];
  char hostname[MAXLINE],port[MAXLINE],path[MAXLINE], makeHeader[MAXLINE];
  char server_port[MAXLINE], server_hostname[MAXLINE], server_buf[MAXLINE];
  rio_t rio,server_rio;
  socklen_t serverlen;
  struct sockaddr_storage serveraddr;

  /* Read request line and headers */
  Rio_readinitb(&rio, fd);
  Rio_readlineb(&rio, buf, MAXLINE);
  printf("Request headers: \n");
  printf("%s", buf);
  sscanf(buf, "%s %s %s", method, uri, version);
  
  
  parse_uri(uri, hostname, port, path);

  serverfd = Open_clientfd(hostname,port);
  Rio_readinitb(&server_rio, serverfd);
  make_header(hostname,port,path,rio,makeHeader);
  printf("%s",makeHeader);
  Rio_writen(serverfd,makeHeader,strlen(makeHeader));

  size_t n;
  while((n = Rio_readlineb(&server_rio, server_buf, MAXLINE)) != 0) {
    printf("Proxy received %d bytes from server and sent to client\n", n);
    Rio_writen(fd, server_buf,n);
  }
  Close(serverfd);
}

/* 헤더를 읽고 무시 */
void read_requesthdrs(rio_t *rp)
{
  char buf[MAXLINE];

  Rio_readlineb(rp, buf, MAXLINE);
  /* 버퍼에 \r\n이 들어왔을 때 종료
    http의 헤더는 \r\n으로 종료된다 */
  while(strcmp(buf, "\r\n")){
    Rio_readlineb(rp, buf, MAXLINE);
    printf("%s", buf);
  }
  return;
}

int parse_uri(char *uri, char *hostname, char *port, char* path)
{
  char* b_hostname = strstr(uri,"//");
  if (b_hostname!=NULL){
    b_hostname = b_hostname+2;
  }
  else{
    b_hostname = uri;
  }

  char* portP = strchr(b_hostname,':');
  if (portP!=NULL){
    *portP = '\0';
    sscanf(b_hostname, "%s", hostname);
    char *pathP =strchr(portP+1,'/');
    if (pathP!=NULL){
    *pathP = '\0';
    sscanf(portP+1, "%s", port);
    sscanf(pathP+1, "%s", path);
    }
  }
  else{
    char *pathP = strchr(b_hostname,'/');
    *pathP = '\0';
    sscanf(b_hostname, "%s", hostname);
    port ="8000";
    sscanf(pathP+1,"%s",path);    
  }
    printf("port : %s\n\n",port);

  printf("hostname : %s\n\n",hostname);

  printf("path : %s\n\n",path);
}

int make_header(char* hostname, char* port, char* path, rio_t rio, char* makeHeader){
  char *buf[MAXLINE], headerHR[MAXLINE], requestRHR[MAXLINE], otherHR[MAXLINE];
  strcpy(buf,"");
  sprintf(requestRHR,"GET /%s HTTP/1.0\r\n",path);
  while(strcmp(buf,"\r\n")){
    Rio_readlineb(&rio, buf, MAXLINE);
    if (strncasecmp(buf,"Host",strlen("Host"))){
      sprintf(headerHR,buf,strlen(buf));
      continue;
    }
    
    else if (strncasecmp(buf,"user-Agent",strlen("user-Agent"))){

    }

    else if (strncasecmp(buf,"Connection",strlen("Connection"))){

    }

    else if(strncasecmp(buf,"Proxy-Connection ",strlen("Proxy-Connection"))){

    }
    else{

      sprintf(otherHR,buf,strlen(buf));

    }
  }

    if (headerHR != NULL){
      sprintf(headerHR,"Host : %s\r\n",hostname);
    }

    sprintf(makeHeader,"%s%s%s%s%s%s%s",requestRHR,headerHR,otherHR,user_agent_hdr,"Connection : close\r\n","Proxy-Connection : close\r\n","\r\n");
  
}
