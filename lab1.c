#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>	//sockaddr_in, in_addr
#include <sys/socket.h>	//socklen_t
#include <sys/stat.h>
#include <netdb.h>	//hostent
#include <time.h>
#include <stdarg.h>
#include <arpa/inet.h>
#include <unistd.h>

int buffer_size;
const int sec_time = 0;
const int usec_time = 50;

int tcp_send(char * argv[]);	
int tcp_recv(char * argv[]);	
int udp_send(char * argv[]);	
int udp_recv(char * argv[]);	
int function(char *argv[]);

int main(int argc, char * argv[]){

	//print imformation

	printf("\n");
	printf("protocol      : %s\n", argv[1]);
	printf("send/recv     : %s\n", argv[2]);
	printf("ip address    : %s\n", argv[3]);
	printf("port number   : %s\n", argv[4]);
	printf("data path     : %s\n", argv[5]);
	printf("\n");

    if(strcmp(argv[1],"tcp") == 0)
        buffer_size = 1;
    else if(strcmp(argv[1],"udp") == 0)
        buffer_size = 64;
   function(argv); 

	return 0;
}

int function(char *argv[]){
    
    int i, j, n, percent = 0;
    int sockfd, new_sockfd, port;
    char buffer[buffer_size];
    double count=0;
    //socket
    struct hostent *ip;
    struct sockaddr_in my_addr; //local address
    struct sockaddr_in client_addr; //client address
    struct sockaddr_in serv_addr; //local address
    int client_addr_size = 0;
    time_t t;
    clock_t t_start, t_end;
    //file
    char file_name[100];
    double total_filesize = 0;
    double send_filesize = 0; //file size has been send
    double fivepercent_filesize = 0;
    struct stat file_state;
    FILE *fp;

	ip = gethostbyname(argv[3]);
	port = atoi(argv[4]);
	strcpy(file_name,argv[5]);
    

    if(strcmp(argv[1],"tcp")==0 && strcmp(argv[2],"send")==0){
        
        //TCP socket
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
	    if(sockfd < 0){
		    printf("ERROR opening socket.\n\n");
		    exit(1);
    	}
    
        //set address
        bzero((char *)&my_addr,sizeof(my_addr));
        my_addr.sin_family = AF_INET;   //IP connect
        my_addr.sin_addr.s_addr = htonl(INADDR_ANY);//server IP,allow all ip to connect
        my_addr.sin_port = htons(port);
    
        //get file state
        if(lstat(file_name,&file_state) < 0){
            printf("lstat() failed\n");
            exit(1);
        }
        printf("The file size is %lu  bytes\n",file_state.st_size);
        total_filesize = file_state.st_size;
        fivepercent_filesize = total_filesize / 20;
        fp = fopen(file_name,"rb");
        if(!fp){
            printf("File open fiailed\n");
            exit(1);
        }
    
        //Binding
        printf("Binding...\n");
        n = bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr));
        if(n < 0){
            printf("Binding Error\n");
            exit(1);
        }

        //Keep listening, queue length is 5
        printf("Listening...\n");
        n = listen(sockfd, 5);
        if(n < 0){
            printf("Listen Error\n");
            exit(1);
        }
        
        //Wait for client
        client_addr_size = sizeof(client_addr);
        printf("Ready to sendfile\n");
        new_sockfd = accept(sockfd, (struct sockaddr *)&client_addr,&client_addr_size);
        if(new_sockfd < 0){
            printf("Accept Error\n");
            exit(1);
        }
        printf("Accept Success"); 

        //Send file size first
        n = write(new_sockfd,&total_filesize,sizeof(double));
        if(n < 0){
            printf("Send file size Error\n");
            exit(1);
        }
        printf("Send file size Success\n");

        t_start = clock();  //time begin to send
        //send file
        while(1){
            //end of file, break
            if(feof(fp))
                break;

            bzero(buffer,buffer_size);
            if((total_filesize - count) < buffer_size){
                
                //read file
                fread(buffer,1,total_filesize-count,fp);
                //send data
                n = write(new_sockfd,buffer,total_filesize-count);
                if(n < 0){
                    printf("Send data failed\n");
                    exit(1);
                }

                count+=buffer_size;
                if((count*100 > total_filesize*percent) && percent<100){
                    t=time(NULL);
                    printf("Send: %4d%%  %s\n", percent, ctime(&t));
                    percent+=5;
                }
                break;

            }else{
                //read file
                fread(buffer,1,buffer_size,fp);
                //send data
                n = write(new_sockfd,buffer,buffer_size);
                if(n < 0){
                    printf("Send data failed\n");
                    exit(1);
                }
            
                count+=buffer_size;
                if((count*100 > total_filesize*percent) && percent<100){
                    t=time(NULL);
                    printf("Send: %4d%%  %s\n", percent, ctime(&t));
                    percent+=5;
                }
            
            }
        }
        t_end = clock();  //time end of send

        printf("Throughput: %lf\tbytes/sec\n\n",total_filesize/((double)(t_end-t_start)/(double)CLOCKS_PER_SEC));
        printf("Sending Finish!!\n\n");
        
        close(sockfd);
        close(new_sockfd);
        return 0;
    }
    /*---------------------tcp recv---------------------*/
    else if(strcmp(argv[1],"tcp")==0 && strcmp(argv[2],"recv")==0){
        printf("tcp recv\n");
        
        //TCP socket
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
	    if(sockfd < 0){
		    printf("ERROR opening socket.\n\n");
		    exit(1);
    	}
        printf("create socket success\n");
        
        //set address
        bzero((char *)&serv_addr,sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;   //IP connect
        serv_addr.sin_port = htons(port);
        //serv_addr.sin_addr.s_addr = inet_addr(argv[3]);
        bcopy((char *)ip->h_addr, (char *)&my_addr.sin_addr.s_addr,ip->h_length);

        //connect
        n = connect(sockfd, (struct sockaddr *)&serv_addr,sizeof(serv_addr));
        if(n < 0){
            printf("connect() Failed\n");
            exit(1);
        }
        printf("connect() success\n");
        
        //open file
        fp = fopen(file_name,"wb");
        if(!fp){
            printf("File open fiailed\n");
            exit(1);
        }

        bzero(buffer,buffer_size);
        n = read(sockfd,&total_filesize,sizeof(double));
        if(n < 0){
            printf("Read error\n");
            exit(1);
        }
        printf("file size: %lf\n",total_filesize);

        //start receiving
        while(count < total_filesize){
            bzero(buffer,buffer_size);
            if((total_filesize - count)< buffer_size){
                n = read(sockfd,buffer,total_filesize-count);
                if(n < 0){
                    printf("Read error\n");
                    exit(1);
                }
            
                //write file
                fwrite(buffer,buffer_size,total_filesize-count,fp);
            
                count+=buffer_size;
                if((count*100 > total_filesize*percent) && percent<100){
                    t=time(NULL);
                    printf("Send: %4d%%  %s\n", percent, ctime(&t));
                    percent+=5;
                }
                
                break;           
            }else{
                n = read(sockfd,buffer,buffer_size);
                if(n < 0){
                    printf("Read error\n");
                    exit(1);
                }
            
                //write file
                fwrite(buffer,buffer_size,buffer_size,fp);
                //calculate
                count+=buffer_size;
                if((count*100 > total_filesize*percent) && percent<100){
                    t=time(NULL);
                    printf("Send: %4d%%  %s\n", percent, ctime(&t));
                    percent+=5;
                }    

            }

        }
        printf("Receivinging Finish!!\n\n");
        
        close(sockfd);
        fclose(fp);

        return 0;
    }
    /*---------------------upd send-----------------------*/
    else if(strcmp(argv[1],"udp")==0 && strcmp(argv[2],"send")==0){
    
    
    
    
    
    
    
    
    
    
    }




    return 0;
}


