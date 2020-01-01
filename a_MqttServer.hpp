#ifndef MQTT_SERVER_H_
#define MQTT_SERVER_H_

#include <cstdlib>
#include <iostream>
#include <memory>
#include <utility>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "ConnectionManager.hpp"

using boost::asio::ip::tcp;

class MqttServer{
public:
    MqttServer(short port)
      : _io_service(),
        _acceptor(_io_service, tcp::endpoint(tcp::v4(), port))
    {
      do_accept();

    }
 
    void run(){
        _io_service.run();
    }

private:
    
    /**
        Register the accept handler to io_service in a asynchronous way.
    */
    void do_accept(){
        std::cout<<"do_accept start"<<std::endl;

        Connection* new_connection = new Connection(_io_service);
  
        _acceptor.async_accept(new_connection->socket(),
             boost::bind(&MqttServer::handle_accept, this, new_connection,
             boost::asio::placeholders::error));
        
    }

    /**
       This function is called AFTER when a new connection request comes.
       
       Triggered by io_service as a callback fuction.
    */
    void handle_accept(Connection* new_connection,
      const boost::system::error_code& error)
    {
        std::cout<<"handle_accept start"<<std::endl;

        if (!error){
            new_connection->start();

            /**
                shared_ptr<Connection> con = make_shared<Connection>("aa", 12);

                typedef shared_ptr<Connection> ConnectionPtr;

                ConnectionPtr con = make_shared<Connection>("bb", 13);

            */

            //_connectionManager.start(
            //    
            //);
        }
        else{
            delete new_connection;
        }

        do_accept();

        std::cout<<"do_accept done"<<std::endl;
    }

    boost::asio::io_service _io_service;
    tcp::acceptor _acceptor;
    
    ConnectionManager _connectionManager;
};




#endif
