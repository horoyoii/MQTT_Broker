#ifndef BROKER_H
#define BROKER_H

#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/epoll.h>
 
#include <string>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <cstring>
#include <memory>

#include <iostream>

#include "Message.hpp"

#include "Connection.hpp"
#include "ConnectionManager.hpp"
#include "TopicTree.hpp"
#include "MessageManager.hpp"

/**

[epoll server 구현] - NON_BLOCKING 모드 
https://github.com/hnakamur/luajit-examples/blob/master/socket/c/epoll-server.c

[mqtt broker reference]
https://github.com/ayourtch/uTT

[MQTT packet 분석 사이트 ]
http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc385349278
*/

class Broker{
public:
    Broker()
    :MAX_EVENTS(100), msgManager(false){
        int res;
        
        // 1) make epoll fd 
        res = epoll_init();
        
        // 2) make listening socket 
        res = make_listening_socket();

        // 3) add listening socket as epoll events
        res = register_servSock();
        
        
        // 4) Initialize the connection manager.
        connManager.setRoot(topicTree.getRoot());        
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
             
            for(i=0; i< nEvents; i++)
            {   
                /**
                    there is a notification on listening socket comming from OS, 
                    which means it needs to handle the incomming connection requests.                  
                */
                //printf(" %d fd occurs\n", events[i].data.fd); 
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
                    handle_event(events[i].data.fd);
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

        // 0) Make client socket NON_BLOCKING  
        int flags = fcntl(cliSock, F_GETFL, 0);
        
        flags |= O_NONBLOCK;
        int s = fcntl(cliSock, F_SETFL, flags);


        // 1) add this socket fd to epoll 
        struct epoll_event ev;
        ev.data.fd = cliSock;
        ev.events = EPOLLIN; //| EPOLLET;
        epoll_ctl(epfd, EPOLL_CTL_ADD, cliSock, &ev);
        


        printf("new connection added : fd - %d\n", cliSock);
        return 1;
    }
    
    /**
        1) parse the type of packet from the client request 
        2) 
    */
    void handle_event(const int cli_fd){
        char buf[65536];
        ssize_t count = read(cli_fd, buf, sizeof(buf));   
        
        //printf("socket read count : %d\n", count);

        // 0) handle exceptions... 
        // --------------------------------
        if(count == 0){
            printf("\n[%d] client send a close request\n", cli_fd);
            
            //ConnectionPtr conn = connManager.get_connection(cli_fd);
            int res = connManager.disconnect(cli_fd);
            
            if(res == 0){
                close(cli_fd);
            }

            epoll_ctl(epfd, EPOLL_CTL_DEL, cli_fd, NULL);
            return;
             
        }else if(count == -1){
            printf("read error\n");
        }
        
        // 1) parse the type of packet 
        // --------------------------------       
        MessageType type = Message::getMessageType(buf[0]);

        // 1) identify message type
        if( type == MessageType::CONNECT){
            // ** CONNECT packet **  
            printf("\n[MQTT / %d] CONNECT request come\n", cli_fd);
            
            // 1-1) send Conn Ack packet to client 
            handle_connection_request(cli_fd);       

            /** 
             1-2) make a new connection object 
                 add it to the connection manager 
            ----------------------------------------------*/
            ConnectionPtr newCon = std::make_shared<Connection>(cli_fd);
            
            connManager.add_connection(cli_fd, newCon);
                        
        }else if( type == MessageType::SUBSCRIBE){
            // ** SUBSCRIBE packet ** 
            printf("\n[MQTT / %d] SUBSCRIBE request come\n", cli_fd);
            ConnectionPtr conn = connManager.get_connection(cli_fd);   
            
            handle_subscribe_request(conn, buf);
            
        }
        else if( type == MessageType::PUBLISH){
            printf("\n[MQTT / %d] PUBLISH request come / read size : %d \n", cli_fd, count);
            unsigned char   remainingLength = (unsigned char)buf[1];
            
            /**
                /TODO : PUBLISH 와 DISCONNECT 가 같이 read 될 수도 있고 아닐 수도 있다. 
            */
            if(remainingLength + 2 == count){
                //printf("제대로 읽힘\n");
                // PUBLISH packet만 읽힌 경우.
                
                handle_publish_request(cli_fd, buf, count);
            }else{
                // PUBLISH + DISCONNECT 같이 읽힘.
                //printf("같이 읽힘\n");
                
                handle_publish_request(cli_fd, buf, count-2);
                handle_disconnect_request(cli_fd);
             }
            
                        
        }
        else if(type == MessageType::PINGREQ){
            printf("\n[MQTT / %d] PINGREQ request come\n", cli_fd);

            handle_ping_request(cli_fd, buf, count);
        
        }else if(type == MessageType::DISCONNECT){
            //TODO : from subscriber or publisher ? 
            printf("\n[MQTT / %d] DISCONNECT request come\n", cli_fd);
            handle_disconnect_request(cli_fd);

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
    

    void handle_subscribe_request(ConnectionPtr conn, char* payload){
        /**
            - Fixed Header 
            byte 1 : MQTT Control packet type(4bit) + reserved(4bit)
            byte 2 : remaining length(variable header + payload)
            
            - Varibale Header 
            byte 3 : packet identifier MSB
            byte 4 : packet identifier LSB 
           
            - payload 
              * string with UTF-8 encoded 
            byte 1      : length MSB 
            byte 2      : length LSB - it menas topic maximum length is 2^16
            byte 3..N   : Topic 
            byte (N+1)  : requested QoS
        */        
        unsigned char remainingLength = (unsigned char)payload[1];
        

        /**
            // 네트워크 바이트 순서 : 빅 엔디안(MSB부터 저장.)
            [0000 0100] [0000 0000] // 1024
            
            [0000 0000] [0000 0100] // 4 
        */
        uint16_t topicLength = ntohs( *(uint16_t*)&payload[4]); // read payload[4][5]


        // 1) Get topic string!!
        std::string topic(payload+6, topicLength);
        std::cout<<topic<<std::endl;
        
        /** 2) store the tocic info to the TopicTree and corresponding connection 
        ---------------------------------------------------------------------------*/
        Node* nnode = topicTree.subscribe(conn, topic);    
        connManager.add_mapping_info(conn->get_cid(), nnode);
        
        /** 3) send Sub Ack packet 
        ----------------------------------------------------------------------------*/
        uint8_t subAck = static_cast<uint8_t>(MessageType::SUBACK); // 9        
        static uint8_t subOk[] = {subAck<<4, 3, payload[2], payload[3], 0}; // 8 bits size
        char* reply = reinterpret_cast<char*>(subOk);
 
        conn->sendMessage(reply, 5);    
    
        /** 4) Send RETAINED messages if any.
        ----------------------------------------------------------------------------*/
        // topic & conn 
        msgManager.sendRetainedMessage(msgManager.getRootNode(), topic, conn); 

    }
    
    /**
        PUB with QoS 0 don't need the PUB ACK packet.
        PUB with QoS 1 needs PUB ACK packet.
        PUB with QoS 2 needs PUB REC, REL, COMP

    */
    void handle_publish_request(int cli_fd, char* buf, ssize_t buf_size){
        /**
            PUBLISH packet 
            - Fixed header[2 byte]
                byte 1 : mqtt Control packet[4] DUP[1]  QoS[2] RETAIN[1]
                byte 2 : remaining length (variablle header + payload)
            - Varibale header : Topic name + Packet identifier 
                byte 1 : length MSB
                byte 2 : length LSB
                byte 3 : 'a'
                byte 4 : '/'
                byte 5 : 'b'
                - ### Identifier is included only when QoS is 1 or 2.
                byte 6 : packet identifier MSB
                byte 7 : packet identifier LSB

            - Payload : application message itself 
        */
        

        // 1) Parse topic and application message
        // -------------------------------------------------------
        unsigned char   remainingLength = (unsigned char)buf[1];
        uint16_t        topicLength = ntohs(*(uint16_t*)&buf[2]);
        
        std::string     topic(buf+4, topicLength);
        int             QoS = Message::get_QoS_level(buf);
        int             offset;
        
        /** 2) Parse the QoS level
        -------------------------------------------------------------*/        
        if(QoS == 0){
            offset = 2+2+static_cast<int>(topicLength);
        }else if(QoS == 1){
            offset = 2+4+static_cast<int>(topicLength);
            
            // Parse the Packet Id
            uint16_t PacketId  = ntohs(*(uint16_t*)&buf[offset-2]);
                    
        }
       
        std::string app_message(buf+offset);  


        // 2) Send application message to corresponding subscribers.
        // -------------------------------------------------------
        std::cout<<"topic : "<<topic<<"\n message : "<<app_message<<std::endl;
        topicTree.publish(topicTree.getRootNode(), topic,buf, buf_size);        


        /** 3) Check whether a Retain flag has been set.
            store the topic and message.
        -----------------------------------------------------------*/
        bool isRetained = Message::hasRetainFlag(buf);
        if(isRetained){
            printf("retained \n");
            msgManager.setRetainedMessage(topic, app_message, buf, buf_size);    
        }

        
    }
   
   /**
        1) When the broker receives the PUB packet 
            1-1) It stores the message internally.
            1-2) with the coresponding Pacekt Id.
        2) Send the PUB packet to all of the subscribers whose topics matchs the publisher's.
           2-1) Find the subscribers in a topic tree
           2-2) Write the packet 
           2-3) Wait for the all PUB ACK packet to come in a amount of time. 
              2-3-1) If timeout, 
                        Resend the PUB to the subscriber.
              2-3-2) If not timeout, 
                        delete the message, send the PUB ACK to publisher(client)
           
        3) When the publisher(client) also have a timeout, 
          It resend the PUB message with DUP=1 and same PID(?)
           


        * RESEND [ 2.3.1 in the MQTT 3.11 ]
        If a client re-send a pub packet, then it MUST used the same PID. 
   */
    void handle_publish_with_QoS_1(){
        /**
            PUB ACK -----------
            byte 1 : Type[0:4]
            byte 2 : Remaining Length = 2
            byte 3 : Packet Identifier MSB
            byte 4 : Packet Identifier LSB
        
            1) When a broker get the msg with QoS 1, 
            2) the broker send the msg to all subscriber immediately
            3) wait for the all PUB ACK from the subscribers.   
            4) replies with a PUB ACK to the publisher(client).
            
            **At least once
            -> When the publishing client does not get the PUB ACK in a reasonable amount of time,
             the client RESEND the PUB msg to the broker.  
            -> Then broker have already deleted the before msg. so RESEND the duplicated msg to sub. 
              
        */
        


    }

    /**
        PINGREQ packet is sent from a client.

        It indicates to the server that the client is alive.
        Server responses with PINGRESP packet, which means server is also alive.

        PINGREQ and PINGRESP packet is composed of 2 bytes.

    */ 
    void handle_ping_request(int cli_fd, char* buf, ssize_t size){

        uint8_t PingRESP = static_cast<uint8_t>(MessageType::PINGRESP); // 13
        
        static uint8_t pingOK[] = {PingRESP<<4, 0}; // 8 bits size
        char* reply = reinterpret_cast<char*>(pingOK);        
        
        
        write(cli_fd, reply, 2);

    }

    void handle_disconnect_request(int cli_fd){
        
        close(cli_fd);
        epoll_ctl(epfd, EPOLL_CTL_DEL, cli_fd, NULL);
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
    TopicTree topicTree;
    ConnectionManager connManager;
    MessageManager msgManager;
};







#endif // BROKER_H
