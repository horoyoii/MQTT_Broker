#ifndef MQTT_SERVER_H
#define MQTT_SERVER_H

#include<boost/asio.hpp>
#include<iostream>


class MqttServer{
public:
    MqttServer(int port);
    void run();


private:
    boost::asio::io_service         _ioService;
    boost::asio::ip::tcp::acceptor  _acceptor;
    boost::asio::ip::tcp::socket    _socket;
    
    void doAccept();
};


#endif
