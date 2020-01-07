#include "Connection.hpp"


void Connection::doRead(){
    
    auto self(shared_from_this());

    _socket.async_read_some(boost::asio::buffer(_data, max_length),
        boost::bind(&Connection::handle_read, this, 
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));   
        


}

void Connection::handle_read(const boost::system::error_code& error,
      size_t bytes_transferred){

    if(!error){
        std::cout<<"recv : "<<_data<<std::endl;

    }else{
        std::cout<<"done"<<std::endl;
        delete this;
    }
}
