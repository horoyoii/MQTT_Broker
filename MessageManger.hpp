#pragma once
#include<string>
#include "Connection.hpp"

using namespace std;

class MessageManager{
public:
    
    
    /**
        store the message of which the retain flag is set. 
    */
    void setRetainedMessage(string topic, string message, char* buf, ssize_t buf_size){
        
    }
    

    /**
        Send retained messages if any.
    */ 
    void sendRetainedMessage(string topic, ConnectionPtr sub){

    }

private:


};
