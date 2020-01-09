#ifndef CONNECTION
#define CONNECTION
#include<memory>

/**
    Connection for Subscribers

*/
class Connection{
public:
    Connection(int cli_fd)
    :cli_fd(cli_fd){

    }
    
    void sendMessage(char* buf){
        

        //write(cli_fd, );
    }

    void disconnect(){
        // cur_node remove this connection    
    }

private:
    int cli_fd;
    Node* cur_node;     // a node in a topic tree where this connection is included
    


};

//typedef std::shared_ptr<Connection> ConnectionPtr;


#endif // CONNECTION
