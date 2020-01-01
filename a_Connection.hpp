#ifndef CONNECTION
#define CONNECTION

#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "ConnectionManager.hpp"

using boost::asio::ip::tcp;

class Connection{
public:
    Connection(boost::asio::io_service& io_service)
      : socket_(io_service){ }
  
    tcp::socket& socket(){
      return socket_;
    }
  
    void start(){

      socket_.async_read_some(boost::asio::buffer(data_, max_length),
          boost::bind(&Connection::handle_read, this,
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }
    
    void stop(){
    }

private:

    /**
        This function is called when the data transmission from kernel to application buffer
        (in here, data_) is done so that this function can handle the data.

    */
    void handle_read(const boost::system::error_code& error,
        size_t bytes_transferred)
    {
        if (!error){
            std::cout<<"handle read "<<std::endl;
            std::cout<<"read data from kernel buffer : "<<data_<<std::endl;
            
            //=========================================================
            static uint8_t connackOk[] = {32, 2, 0, 0}; // 8 bits size
            char* data = reinterpret_cast<char*>(connackOk);

            
            // connack packet 
            // http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Figure_3.8_%E2%80%93
            //
            // 0010 0000 / 0000 0010 / 0000 0000 / 0000 0000

           

            //========================================================
            boost::asio::async_write(socket_,
                boost::asio::buffer(data, 4),
                boost::bind(&Connection::handle_write, this,
                boost::asio::placeholders::error));
        }
        else{
            std::cout<<"handle read delete called"<<std::endl;
            delete this;
        }
        std::cout<<"handle read done"<<std::endl;
    }
    
    /**
        This handler is called when the write operation is done.
        So define what to do next.
    */ 
    void handle_write(const boost::system::error_code& error){
        if (!error){
            std::cout<<"handle write() called after send the data"<<std::endl;
            
            socket_.async_read_some(boost::asio::buffer(data_, max_length),
                boost::bind(&Connection::handle_read, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
        }
        else{
            delete this;
        }

    }
  
    tcp::socket socket_;
    enum { max_length = 128 * 1024 };
    char data_[max_length];
};

typedef std::shared_ptr<Connection> ConnectionPtr;

#endif
