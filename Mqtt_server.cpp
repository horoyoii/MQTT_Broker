#include"Mqtt_server.hpp"
#include "Connection.hpp"

using boost::asio::ip::tcp;


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
                                    
                                    std::make_shared<Connection>(std::move(_socket))->start();
                                }

                            

                            doAccept();

                        });

}
