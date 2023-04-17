#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(){

    // create a socket
    int network_socket;
    // AF_INET : IPv4 체계, SOCK_STREAM : TCP
    network_socket = socket(AF_INET, SOCK_STREAM,0);
    
    // specify an address for the socket 
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(9002);
    server_address.sin_addr.s_addr = INADDR_ANY;

    //create a connect
    //Client의 소켓, Server의 정보, server의 사이즈
    int connection_status = connect(network_socket, (struct sockaddr *) &server_address, sizeof(server_address));

    if (connection_status == -1){
        printf("connection error\n");
    }

    // receive data from server
    char server_response[256];
    
    // network_socket : server data, server_response : server data 저장 공간, sizeof(server_response) : server_response의 사이즈
    recv(network_socket,&server_response,sizeof(server_response),0);

    // print out the server's response
    printf("The server data : %s\n", server_response);
    
    // and then close the socket
    close(network_socket);
    
    return 0;
}