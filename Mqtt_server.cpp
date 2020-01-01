#include"Mqtt_server.hpp"

using boost::asio::ip::tcp;

class session
{
public:
  session(boost::asio::io_service& io_service)
  //boost 1.66이후 (Ubuntu 18.10 이후) 버전의 경우 io_context를 사용
  //session(boost::asio::io_context& io_service)
    : socket_(io_service)
  {
  }
 
  tcp::socket& socket()
  {
    return socket_;
  }
 
  void start()
  {
    //client로부터 연결됨
    cout << "connected" << endl;
    //client로부터 비동기 read 실행
    socket_.async_read_some(boost::asio::buffer(data_, max_length),
        boost::bind(&session::handle_read, this,
          boost::asio::placeholders::error,
          boost::asio::placeholders::bytes_transferred));
  }
 
private:
  void handle_read(const boost::system::error_code& error,
      size_t bytes_transferred)
  {
    if (!error)
    {
      cout << data_ << endl;
    }
    else
    {
      delete this;
    }
  }
 
  tcp::socket socket_;
  enum { max_length = 1024 };
  char data_[max_length];
};



MqttServer::MqttServer(int port)
    :_ioService(),
    _acceptor(_ioService, tcp::endpoint(tcp::v4(), port)),
    _socket(_ioService)
{
    doAccept();

}

void MqttServer::run(){
    _ioService.run();
}


void MqttServer::doAccept(){
    std::cout<<"doAccept()"<<std::endl;

    _acceptor.async_accept(_socket,
                            [this](boost::system::error_code error){
                                if(!_acceptor.is_open()) 
                                    return;
                                if(!error){
                                    // when new connection comes...
                                    std::cout<<"new connection comes"<<std::endl;                  

                                }

                            

                            doAccept();

                        });

}
