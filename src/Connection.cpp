#include "Connection.hpp"


Connection::Connection(int cli_fd)
:cli_fd(cli_fd){

}
    
int Connection::sendMessage(char* buf, int size){
    ssize_t cnt = write(cli_fd, buf, size);

    return cnt;
}

void Connection::disconnect(){
    
    // close socket
    close(cli_fd);
}
    
int Connection::get_cid(){
    return cli_fd;
}

Connection::~Connection(){
    std::cout<<"connection destructor called"<<std::endl;        
}

