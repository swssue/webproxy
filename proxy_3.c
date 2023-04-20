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
// #define MAX_CACHE_SIZE 1049000
#define MAX_CACHE_SIZE 35000
#define MAX_OBJECT_SIZE 102400

/*cache struct create*/
typedef struct cacheStorage {
  char* cache_path; /*경로 지정*/
  char* content_buf; /*body 내용 저장*/
  struct cacheStorage* nextStorage; /*다음 노드 정보*/
  struct cacheStorage* prevStorage; /*이전 노드 정보*/
  int content_length; /*body 사이즈*/
} cacheStorage;

/*global root for cache*/
cacheStorage *root = NULL;

/*total_size for cache*/
int total_cachesize = 0;

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
    "Firefox/10.0.3\r\n";

int doit(int fd);
void read_requesthdrs(rio_t *rp);
int parse_uri(char *uri, char *hostname, char *port, char* path);
int make_header(char* hostname, char* port, char* path, rio_t rio, char* makeHeader);

void *thread(void *vargp);

cacheStorage* find_linked(char *path);

int main(int argc, char **argv) {
  int listenfd, *connfd, server_listenfd;
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;
  pthread_t tid;

  /* Check command line args */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  listenfd = Open_listenfd(argv[1]);
  while (1) {
    clientlen = sizeof(clientaddr);
    connfd = Malloc(sizeof(int));
    *connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);  // line:netp:tiny:accept
    // 소켓 구조체를 호스트와 서비스이름 스트링으로 바꾸기
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
    printf("Accepted connection from (%s, %s)\n", hostname, port);
    Pthread_create(&tid, NULL, thread, connfd);
    // doit(connfd);   // line:netp:tiny:doit
    // Close(connfd);  // line:netp:tiny:close
  }
}

int doit(int fd)
{
  int is_static, serverfd;
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char filename[MAXLINE], cgiargs[MAXLINE];
  char hostname[MAXLINE],port[MAXLINE],path[MAXLINE], makeHeader[MAXLINE];
  char server_buf[MAXLINE];
  rio_t rio,server_rio;

  char content_body[MAXLINE];
  int content_length=0;

  /* Read request line and headers */
  Rio_readinitb(&rio, fd);
  Rio_readlineb(&rio, buf, MAXLINE);
  printf("Request headers: \n");
  printf("%s", buf);
  sscanf(buf, "%s %s %s", method, uri, version);
  
  parse_uri(uri, hostname, port, path);

  char* clientPath = (char*)malloc(MAXLINE);
  strcpy(clientPath,path);

  cacheStorage* find_cache = find_linked(clientPath);
 
  /*chache에 찾는 값이 있는 경우*/
  if(find_cache!=NULL){
    // printf("cache : %s\n",find_cache->cache_path);
    delete_linked(find_cache);
    insert_linked(find_cache);
    // printf("%s\n",find_cache->content_buf);
    Rio_writen(fd,find_cache->content_buf,find_cache->content_length);

    printf("cache is in cache storage, cache update.\n");
    return 0;
  }

  /*chache에 찾는 값이 없는 경우*/
  serverfd = Open_clientfd(hostname,port);
  Rio_readinitb(&server_rio, serverfd);
  make_header(hostname,port,path,rio,makeHeader);
  printf("%s",makeHeader);
  Rio_writen(serverfd,makeHeader,strlen(makeHeader));

  char* cache_buf = (char*)malloc(MAX_OBJECT_SIZE);

  strcpy(cache_buf, "");
 
  size_t n;
  while((n = Rio_readlineb(&server_rio, server_buf, MAXLINE)) != 0) {
    // printf("Proxy received %d bytes from server and sent to client\n", n);
    content_length+=n;
    strcat(cache_buf,server_buf);
    Rio_writen(fd, server_buf,n);
  }

  printf("Proxy received %d bytes from server and sent to client\n", content_length);
  

  /*현재 값을 추가할 때, max 보다 큰 경우*/
  if (total_cachesize+content_length > MAX_CACHE_SIZE){
    delete_tail();
  }

  /*현재 값을 추가할 때, max 보다 작은 경우*/
  total_cachesize +=content_length;
  printf("total_cache : %d\n",total_cachesize);
  init_cache(clientPath, content_length, cache_buf);

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

/* Thread routine */
void *thread(void *vargp) 
{  
    int connfd = *((int *)vargp);
    Pthread_detach(pthread_self()); //line:conc:echoservert:detach
    Free(vargp);                    //line:conc:echoservert:free
    doit(connfd);
    Close(connfd);
    return NULL;
}
/* $end echoservertmain */

/*start find linked list*/
cacheStorage* find_linked(char *path){
  cacheStorage *bp;
  
  for (bp = root; bp != NULL; bp = bp->nextStorage) {
    // printf("path: %s\n",path);
    // printf("cache path: %s\n",bp->cache_path);
    // strcmp == 0 인 경우 동일하다. 
    if (strcmp(bp->cache_path, path) == 0) {
      return bp;
    }
  }
  return NULL;
}
/*end find linked list*/


void init_cache(char* path, int content_length, char* body){
  /*init cache*/
  cacheStorage *cs = (cacheStorage *)calloc(1, sizeof(cacheStorage));
  cs->cache_path = path; /*경로 지정*/
  cs->content_buf = body; /*body 내용 저장*/
  cs->nextStorage = NULL; /*다음 노드 정보*/
  cs->prevStorage = NULL; /*이전 노드 정보*/
  cs->content_length = content_length; /*body 사이즈*/
  printf("/%s init cache success\n\n",path);
  insert_linked(cs);
}

/*start insert linked list*/
void insert_linked(cacheStorage *insertNode){
    insertNode->nextStorage = root;
    if (root!=NULL){
        root->prevStorage = insertNode;
    }
    root = insertNode;
}
/*end insert linked list*/

/*start delete linked list*/
void delete_linked(cacheStorage *deleteNode){
    if (deleteNode!=root){
      deleteNode->prevStorage->nextStorage = deleteNode->nextStorage;
      // 삭제 했는데 남은게 NULL이 아닌 경우 
      if (deleteNode->nextStorage!=NULL)
          deleteNode->nextStorage->prevStorage = deleteNode->prevStorage;
    //가장 최근 값
    }else{
        root = deleteNode->nextStorage;
    }

}
/*end delete linked list*/

/*start find tail list*/
void delete_tail(){
  cacheStorage* tail;
  for(tail = root; tail!=NULL; tail = tail->nextStorage){
    // printf("find tail : %s\n",tail->cache_path);
    if (tail->nextStorage == NULL){
      break;
    }
  }
  total_cachesize-=tail->content_length;
  tail->prevStorage->nextStorage = NULL;
  // printf("삭제되는 path: %s\n",tail->cache_path);
  // printf("갱신되는 path의 next값: %s, %s\n",tail->prevStorage->cache_path,tail->prevStorage->nextStorage);
  free(tail);
}
/*end find tail list*/