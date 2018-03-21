#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<netdb.h>   //hostent
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>

int tcp_send(char *argv[]);
int tcp_recv(char *argv[]);
int udp_send(char *argv[]);
int udp_recv(char *argv[]);

int main(int argc,char *argv[]){
    
    char tcp_or_upd[4]={}, send_or_recv[4]={};
    
    if(strcmp(argv[1],"tcp") == 0){    //tcp
        
        if(strcmp(argv[2],"send") == 0)
            tcp_send(argv);        
        else if(strcmp(argv[2],"recv") == 0)
            tcp_recv(argv);        
   
    }else if(strcmp(argv[1],"udp") == 0){  //udp

        if(strcmp(argv[2],"send") == 0)
            udp_send(argv);        
        else if(strcmp(argv[2],"recv") == 0)
            udp_recv(argv);        
    }

    return 0;
}

int tcp_send(char *argv[]){

    int sockfd = 0, port = 0, new_sockfd = 0, n = 0;
    long file_size = 0;
    char buffer[256] = {};
    struct sockaddr_in server, client;
    struct hostent *host;
    FILE *fp;
    socklen_t client_len;

    printf("in tcp_send function\n");

    //open file    
    fp=fopen(argv[5],"r");
    if(!fp){
        printf("File do not exist\n");
        return 0;
    }

    //get file size
    fseek(fp,0L,SEEK_END);  //seek to the end of file
    file_size = ftell(fp);
    fseek(fp,0L,SEEK_SET);  //seek back to the beginning of file
    printf("File Size %ld\n",file_size);

    //create socket
    sockfd=socket(AF_INET,SOCK_STREAM,0);
    if(sockfd < 0){
        error("Failed to create socket\n");
        return 0;
    }
    printf("Socket Created\n");

    //get ip and port
    if(!(host = gethostbyname(argv[3]))){
        printf("ERROR host");
        return 1;
    }
    port = atoi(argv[4]);
    printf("port %d\n",port);

    //define address of socket
    bzero((char *)&server,sizeof(server));  //initialize struct to 0
    server.sin_family = AF_INET;
    bcopy((char *)host->h_addr,(char *)&server.sin_addr.s_addr,host->h_length);
    server.sin_port = htons(port);
    
    //binding
    if(bind(sockfd,(struct sockaddr *)&server,sizeof(server))<0){
        error("binding error\n");
        return 1;
    }
    printf("binding success\n");

    //wait for TCP client's request
    printf("listening...\n");
    listen(sockfd,5);
    printf("lACCEPT istening\n");

    //accept client request
    client_len = sizeof(client);
    new_sockfd = accept(sockfd, (struct sockaddr *)&client, &client_len);
    if(new_sockfd < 0){
        error("Accept error\n");
        return 1;
    }
    printf("ACCEPT request from client\n");
    
    //init buffer
    //bzero(buffer, 256);
    //send file size first
    n = write(new_sockfd, &file_size, sizeof(file_size));
    if(n < 0){
        error("ERROR write file size\n");
        return 1;
    }
    printf("SUCCESS send file size\n");

    



    fclose(fp);

    return 0;
}

int tcp_recv(char *argv[]){

    int sockfd = 0, port = 0, new_sockfd = 0, n = 0;
    long file_size = 0, count = 0;
    char buffer[256] = {};
    struct sockaddr_in server,client;
    struct hostent *host;
    FILE *fp;
    socklen_t client_len;

    printf("In tcp_recv function\n");
    //open file    
    fp=fopen(argv[5],"w");
    if(!fp){
        printf("File do not exist\n");
        return 0;
    }
    printf("SUCCESS open file\n");

    sockfd=socket(AF_INET,SOCK_STREAM,0);
    if(sockfd < 0){
        error("Failed to create socket\n");
        return 0;
    }
    printf("Socket Created\n");

    //get ip and port
    if(!(host = gethostbyname(argv[3]))){
        printf("ERROR host");
        return 1;
    }
    port = atoi(argv[4]);
    printf("port %d\n",port);
    
    //define address of socket
    bzero((char *)&server,sizeof(server));  //initialize struct to 0
    server.sin_family = AF_INET;
    //bcopy((char *)host->h_addr,(char *)&server.sin_addr.s_addr,host->h_length);
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);
    
    //connect to server
    if(connect(sockfd,(struct sockaddr *)&server,sizeof(server)) < 0){
        error("FAILED to connect to server");
        return 1;
    }
    printf("SUCCESS connect\n");

    //init buffer
    bzero(buffer, 256);
    //read file size first
    n = read(new_sockfd, &file_size, sizeof(file_size));
    if(n < 0){
        error("ERROR read file size\n");
        return 1;
    }
    
    printf("FILE size %ld\n",file_size);
    printf("START receiving\n\n");
    
    //start receive
    while(count < file_size){
    
    
    
    
    }


    fclose(fp);

    return 0;
}

int udp_send(char *argv[]){


    return 0;
}

int udp_recv(char *argv[]){
    

    return 0;
}
