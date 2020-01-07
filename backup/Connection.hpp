#ifndef CONNECTION
#define CONNECTION


#include<boost/asio.hpp>
#include<boost/bind.hpp>
#include<iostream>


/**


*/
class Connection : public std::enable_shared_from_this<Connection> {
public:
    Connection(boost::asio::ip::tcp::socket socket)
    :_socket(std::move(socket)), max_length(1024){}

    void start(){
        doRead();
    }
    


private:
    boost::asio::ip::tcp::socket _socket;    
    char _data[1024];
    int max_length;
    
    void doRead();
    void handle_read(const boost::system::error_code& error, size_t bytes_transferred);
};

typedef std::shared_ptr<Connection> ConnectionPtr;


#endif
