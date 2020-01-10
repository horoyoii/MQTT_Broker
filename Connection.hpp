#ifndef CONNECTION
#define CONNECTION
#include<memory>
#include<iostream>
#include<unistd.h>


/**
    Connection for Subscribers
    
    This object is created when MQTT_CONNECTION and CONNACK packets are exchanged, not about TCP connection.
    
    each connection has its own identifier.
    and client socket file descriptor can be used as the unique identifier.
*/
class Connection{
public:

    Connection(int cli_fd);
           
    int sendMessage(char* buf, int size);
    
    void disconnect();
            
    int get_cid();

    ~Connection();
    

private:
    int cli_fd;
   
};

typedef std::shared_ptr<Connection> ConnectionPtr;

#endif // CONNECTION
