#ifndef BROKER_H
#define BROKER_H

#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/epoll.h>
 
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

#include "Message.hpp"

/**

[epoll server 구현] - NON_BLOCKING 모드 
https://github.com/hnakamur/luajit-examples/blob/master/socket/c/epoll-server.c

*/

class Broker{
public:
    Broker()
    :MAX_EVENTS(100){
        int res;
        
        // 1) make epoll fd 
        res = epoll_init();
        
        // 2) make listening socket 
        res = make_listening_socket();

        // 3) add listening socket as epoll events
        res = register_servSock();
        
    }   
    
    void run(){
        struct epoll_event *events;
        events = (struct epoll_event *)malloc(sizeof(struct epoll_event) * MAX_EVENTS);

        int nEvents, i, res;
        while(1){

            /**
                epoll의 장점 : select와 달리 i/o event 발생 fd가 events 배열에 전달되어 
                application으로 notify됨. 
                - epoll_wait의 2번째 params인 events 배열에 저장됨.
                - MAX_EVENT : 한번에 처리할 event의 수. 
            */
            printf("epoll wait called\n");
            nEvents = epoll_wait(epfd, events, MAX_EVENTS, -1);
            printf("epoll_wait wake with [%d] events\n", nEvents);
            
            for(i=0; i< nEvents; i++)
            {   
                /**
                    there is a notification on listening socket comming from OS, 
                    which means it needs to handle the incomming connection requests.                  
                */
                printf(" %d fd occurs\n", events[i].data.fd); 
                if( events[i].data.fd == servSock ){
                    
                    while(1){

                        /**
                            add_new_connection 안에서 NON_BLOCKING accept()이 호출되고 그 결과가 반환됨. 
                        */
                        res = add_new_connection();
                        if(res == EAGAIN || res == EWOULDBLOCK){ // 더 이상 처리할 connection 요청이 없음.
                            break;
                        }
                    } // end while 

                }else{
                    /**
                        a notification from client sockets.
                        
                    */  
                    handle_message(events[i].data.fd);
                }

            } 

        } // end while
    }

private:
    
    
    /**
        epoll_create wrapper 
        - it create a file descripter for epoll.
        - return
          1 : if success 
          0 : if fail
    */
    int epoll_init(){
        //TODO : what if it fails 
        epfd = epoll_create1(0);
        
        return epfd > 0 ? 1 : 0;
    }

    /**
        make a listening socket with NON_BLOCKING mode 

    */    
    int make_listening_socket(){
        struct sockaddr_in addr;
        socklen_t addrlen;

        addrlen = sizeof(addr);
        
        if((servSock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
            return 0;
   
        addr.sin_family = AF_INET;
        addr.sin_port = htons(1883);
        addr.sin_addr.s_addr = htonl(INADDR_ANY);

        if(bind (servSock, (struct sockaddr *)&addr, addrlen) == -1)
            return 0;
        
        //TODO : manage max socket connection
        listen(servSock, 32);
        
        // make NON_BLOCKING mode
        int flags = fcntl(servSock, F_GETFL, 0);
        flags |= O_NONBLOCK;
        fcntl(servSock, F_SETFL, flags);

        return 1;
    }
    
    int register_servSock(){
        struct epoll_event ev;

        ev.events = EPOLLIN; // TODO : add EPOLLET - edge trigger mod
        ev.data.fd = servSock;
        
        return epoll_ctl(epfd, EPOLL_CTL_ADD, servSock, &ev);       
    }

    /**
        new connection would be accepted by accept().
        and register the client socket fd as epoll event by epoll_ctl() 
    */
    int add_new_connection(){
        struct sockaddr_in cli_addr;
        socklen_t cli_len = sizeof(cli_addr);
        int cliSock;

        cliSock = accept(servSock, (struct sockaddr*)&cli_addr, &cli_len);
        
        if(cliSock == -1)
            return errno;

        // 1) add this socket fd to epoll 
        struct epoll_event ev;
        ev.data.fd = cliSock;
        ev.events = EPOLLIN;
        epoll_ctl(epfd, EPOLL_CTL_ADD, cliSock, &ev);
        
        printf("new connection added : fd - %d\n", cliSock);
        return 1;
    }
    
    /**
        1) parse the type of packet from the client request 
        2) 
    */
    void handle_message(const int cli_fd){
        char buf[1024];
        ssize_t count = read(cli_fd, buf, sizeof(buf));   
        
        
        // 0) handle exceptions... 
        // --------------------------------
        if(count == 0){
            printf("client send a close request\n");
            
            epoll_ctl(epfd, EPOLL_CTL_DEL, cli_fd, NULL);
            close(cli_fd);
        
        }else if(count == -1){
            printf("read error\n");
        }
        
        // 1) parse the type of packet 
        // --------------------------------       
        MessageType type = Message::getMessageType(buf[0]);

        // 1) identify message type
        if( type == MessageType::CONNECT){
            // ** CONNECT packet **  
            printf("connect request come\n");
            
            // 1-1) send Conn Ack packet to client 
            handle_connection_request(cli_fd);       

            //TODO: 1-2) add this client to connection pool

            sleep(4);
        }else if( type == MessageType::SUBSCRIBE){
            // ** SUBSCRIBE packet ** 
            printf("subscribe request come\n");
            
            handle_subscribe_request();

            sleep(10);
        }
        else if( type == MessageType::PUBLISH){
            printf("publish packet come\n");
        }
         
      
           
    }
    
    /**
        send Conn Ack resposne to client
    */
    void handle_connection_request(int cli_fd){
            
        static uint8_t connackOk[] = {32, 2, 0, 0}; // 8 bits size
        char* reply = reinterpret_cast<char*>(connackOk);
        // 0010 0000 / 0000 0010 / 0000 0000 / 0000 0000
          
        write(cli_fd, reply, 4);
           
    }
    

    void handle_subscribe_request(){

    }


    int epoll_add(const int fd){
        struct epoll_event ev;

        ev.events = EPOLLIN | EPOLLOUT | EPOLLERR;
        ev.data.fd = fd;
        
        return epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
    }
    

    int epfd;
    int servSock;
    const int MAX_EVENTS;
};







#endif // BROKER_H
