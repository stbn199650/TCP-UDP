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

#define BUFFERSIZE 1
typedef struct{
    int id;
    int buf_size;
}PackInfo;

struct SendPack{
    PackInfo head;
    char buf[BUFFERSIZE];
}SendData;

struct RecvPack{
    PackInfo head;
    char buf[BUFFERSIZE];
}RecvData;

int buffer_size;
const int sec_time = 0;
const int usec_time = 50;

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

    buffer_size = 1;
    function(argv); 

	return 0;
}

int function(char *argv[]){
    
    int i, j, n, percent = 0;
    int sockfd, new_sockfd, port;
    char buffer[buffer_size];
    char resend = '0';
    double count = 0;
    //socket
    struct hostent *ip;
    struct sockaddr_in my_addr; //local address
    struct sockaddr_in client_addr; //client address
    struct sockaddr_in serv_addr; //local address
    socklen_t client_addr_size;
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
        new_sockfd = accept(sockfd, (struct sockaddr *)&client_addr,&client_addr_size);
        if(new_sockfd < 0){
            printf("Accept Error\n");
            exit(1);
        }
        printf("Accept Success\n\n"); 

        //Send file size first
        n = write(new_sockfd,&total_filesize,sizeof(double));
        if(n < 0){
            printf("Send file size Error\n");
            exit(1);
        }

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
                if((count*100 > total_filesize*percent) && percent<=100){
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
                if((count*100 > total_filesize*percent) && percent<=100){
                    t=time(NULL);
                    printf("Send: %4d%%  %s\n", percent, ctime(&t));
                    percent+=5;
                }
            
            }
        }
        t_end = clock();  //time end of send

        printf("Sending Finish!!\n"); 
        printf("Throughput: %lf  bytes/sec\n\n",total_filesize/((double)(t_end-t_start)/(double)CLOCKS_PER_SEC));
        
        fclose(fp);
        close(sockfd);
        close(new_sockfd);

        return 0;
    }
    /*---------------------tcp recv---------------------*/
    else if(strcmp(argv[1],"tcp")==0 && strcmp(argv[2],"recv")==0){
        
        //TCP socket
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
	    if(sockfd < 0){
		    printf("ERROR opening socket.\n\n");
		    exit(1);
    	}
        printf("Create Socket Success\n");
        
        //set address
        bzero((char *)&serv_addr,sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;   //IP connect
        serv_addr.sin_port = htons(port);
        //serv_addr.sin_addr.s_addr = inet_addr(argv[3]);
        bcopy((char *)ip->h_addr, (char *)&my_addr.sin_addr.s_addr,ip->h_length);

        //connect
        n = connect(sockfd, (struct sockaddr *)&serv_addr,sizeof(serv_addr));
        if(n < 0){
            printf("Connect Failed\n");
            exit(1);
        }
        printf("Connect Success\n\n");
        
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
        printf("FILE Size: %.2lf  bytes\n\n",total_filesize);

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
                if((count*100 >= total_filesize*percent) && percent<=100){
                    t=time(NULL);
                    printf("Receive: %4d%%  %s\n", percent, ctime(&t));
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
                if((count*100 >= total_filesize*percent) && percent<=100){
                    t=time(NULL);
                    printf("Receive: %4d%%  %s\n", percent, ctime(&t));
                    percent+=5;
                }    

            }

        }//end while
        printf("Receivinging Finish!!\n\n");

        fclose(fp);
        close(sockfd);

        return 0;
    }
    /*---------------------upd send-----------------------*/
    else if(strcmp(argv[1],"udp")==0 && strcmp(argv[2],"send")==0){
    
        //UDP socket
        sockfd = socket(PF_INET, SOCK_DGRAM, 0);
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
        printf("Total file size is %lu  bytes\n",file_state.st_size);
        total_filesize = file_state.st_size;
        fivepercent_filesize = total_filesize / 20;

        //open file
        fp = fopen(file_name,"rb");
        if(!fp){
            printf("File open fiailed\n");
            exit(1);
        }

        //binding
        n = bind(sockfd,(struct sockaddr *)&my_addr,sizeof(struct sockaddr));
        if(n < 0){
            printf("Binding Error\n");
            exit(1);
        }
        printf("Binding Success\n");
    
        //accept check message
        bzero(buffer,buffer_size);
        client_addr_size = sizeof(client_addr);
        n = recvfrom(sockfd,buffer,buffer_size,0,(struct sockaddr *)&client_addr,&client_addr_size);
        if(n < 0){
            printf("recvfrom Error\n");
            exit(1);
        }
        printf("Client is ready to receive data\n\n");

        //Send file size first
        n = sendto(sockfd,&total_filesize,sizeof(double),0,(struct sockaddr *)&client_addr,sizeof(client_addr));
        if(n < 0){
            printf("Send file size failed\n");
            exit(1);
        }

        int len = 0, send_id = 0, recv_id = 0;
        buffer_size = 1;
        //start to send
        t_start=clock();
        while(1){
            if(feof(fp) || percent > 100)
                break;    
    
            PackInfo pack_info;
            if(send_id == recv_id){
                send_id++;
                
                //read file
                len = fread(SendData.buf,sizeof(char),BUFFERSIZE,fp);
                SendData.head.id = send_id;//use id to record order
                SendData.head.buf_size = len;//record data length

                //send data
                n = sendto(sockfd,(char *)&SendData,sizeof(SendData),0,(struct sockaddr *)&client_addr,sizeof(client_addr));
                if(n < 0){
                    printf("Send data failed\n");
                    break;
                }
                //printf("Send data Success\n");
                
                //receive check
                n = recvfrom(sockfd,(char *)&pack_info,sizeof(pack_info),0,(struct sockaddr *)&client_addr,&client_addr_size);
                if(n < 0){
                    printf("recvfrom() Error\n");
                    break;
                }
                recv_id = pack_info.id;

                if(send_id == recv_id){
                    count+=buffer_size;
                    if((count*100 >= total_filesize*percent) && percent<=100){
                    
                        t=time(NULL);
                        printf("Send: %4d%%  %s\n", percent, ctime(&t));
                        percent+=5;
                    }
                }
            }
            /*-----------------send_id != recv_id----------------*/
            else{
                //send_id != recv_id, resend
                n = sendto(sockfd,(char *)&SendData,sizeof(SendData),0,(struct sockaddr *)&client_addr,sizeof(client_addr));
                if(n < 0){
                    printf("Send data failed\n");
                    break;
                }
                //printf("Send data Success\n");

                n = recvfrom(sockfd,(char *)&pack_info,sizeof(pack_info),0,(struct sockaddr *)&client_addr,&client_addr_size);
                if(n < 0){
                    printf("recvfrom() Error\n");
                    break;
                }
                recv_id = pack_info.id; 
           
                if(send_id == recv_id){
                    count+=buffer_size;
                    if((count*100 > total_filesize*percent) && percent<=100){
                        t=time(NULL);
                        printf("Send: %4d%%  %s\n", percent, ctime(&t));
                        percent+=5;
                    }
                }
            }

        }//end while loop
        t_end = clock();

        printf("Sending Finish!!!\n");
        printf("Throughput: %lf  bytes/sec\n\n",total_filesize/((double)(t_end-t_start)/(double)CLOCKS_PER_SEC));


        fclose(fp);
        close(sockfd);
        close(new_sockfd);

        return 0;
    }
    /*---------------------upd recv-----------------------*/
    else if(strcmp(argv[1],"udp")==0 && strcmp(argv[2],"recv")==0){

        //UDP socket
        sockfd = socket(PF_INET, SOCK_DGRAM, 0);
	    if(sockfd < 0){
		    printf("ERROR opening socket.\n\n");
		    exit(1);
    	}
    
        //set address
        bzero((char *)&my_addr,sizeof(my_addr));
        my_addr.sin_family = AF_INET;   //IP connect
        my_addr.sin_port = htons(port);
        bcopy((char *)ip->h_addr, (char *)&my_addr.sin_addr.s_addr,ip->h_length);
        
        //open file
        fp = fopen(file_name,"wb");
        if(!fp){
            printf("File open fiailed\n");
            exit(1);
        }

        bzero(buffer,buffer_size);
        strcpy(buffer,"client is ready to receive data\n");
        n = sendto(sockfd,buffer,strlen(buffer),0,(struct sockaddr *)&my_addr,sizeof(my_addr));
        if(n < 0){
            printf("Send check failed\n");
            exit(1);
        }
        printf("Send check success\n");

        //receive file size
        n = recvfrom(sockfd,&total_filesize,sizeof(total_filesize),0,NULL,NULL);
        if(n < 0){
            printf("recv file size Error\n");
            exit(1);
        }
        printf("Total File Size: %.2lf  bytes\n\n",total_filesize);

        socklen_t my_addr_size = sizeof(my_addr);
        int id = 1;
        int len = 0;
        //Start receiving data
        while(count<total_filesize){
            
            PackInfo pack_info;
            
            n = recvfrom(sockfd,(char *)&RecvData,sizeof(RecvData),0,(struct sockaddr *)&my_addr,&my_addr_size);
            if(n < 0){
                printf("recv data Error\n");
                exit(1);
            }
            else{   //n >= 0
                if(RecvData.head.id == id){
                    pack_info.id = RecvData.head.id;
                    pack_info.buf_size = RecvData.head.buf_size;
                    id++;
                    n = sendto(sockfd,(char *)&pack_info,sizeof(pack_info),0,(struct sockaddr *)&my_addr,sizeof(my_addr));
                    if(n < 0){
                        printf("Send check message Error\n");
                        break;
                    }
                    //printf("Send check message success\n");

                    //write file
                    fwrite(RecvData.buf,sizeof(char),RecvData.head.buf_size,fp);
                
                    //calculate
                    count+=RecvData.head.buf_size;
                    if((count*100 >= total_filesize*percent) && percent<=100){
                        t=time(NULL);
                        printf("Receive: %4d%%  %s\n", percent, ctime(&t));
                        percent+=5;
                    }    

                }
                /*------------RecvData.head.id < id--------------------*/
                else if(RecvData.head.id < id){     //resent package
                    pack_info.id = RecvData.head.id;
                    pack_info.buf_size = RecvData.head.buf_size;

                    n = sendto(sockfd,(char *)&pack_info,sizeof(pack_info),0,(struct sockaddr *)&my_addr,sizeof(my_addr));
                    if(n < 0){
                        printf("Send check message Error\n");
                        break;
                    }
                    //printf("Send check message success\n");
                
                }
            }
        
        }//end while 
        printf("Receiving Finish!!!\n");
            
        fclose(fp);
        close(sockfd);

        return 0;
    }

    return 0;
}


