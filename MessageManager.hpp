#pragma once
#include<string>
#include<unordered_map>

#include "Connection.hpp"

using namespace std;

class Packet{
public:
    Packet(char* buf, ssize_t buf_size){
        m_buf = new char[buf_size];
        memcpy(m_buf, buf, buf_size);
        m_buf_size = buf_size;
    }
    
    ~Packet(){
        delete[] m_buf; 
    }
    
    char* getBuf(){
        return m_buf;
    }

    ssize_t getSize(){
        return m_buf_size;
    }

private:
    char* m_buf;
    ssize_t m_buf_size;
};

class MessageManager{
public:
    MessageManager() = delete;

    MessageManager(bool usingRedis = false)
    :usingRedis(usingRedis){
       
    }
    
    /**
        store the message of which the retain flag is set. 
    */
    void setRetainedMessage(string topic, string message, char* buf, ssize_t buf_size){
        unordered_map<string, Packet*>::iterator iter = msgs.find(topic);
         
        if(iter != msgs.end()){
            cout<<"retained message already cached so overwritten"<<endl;
            delete msgs.find(topic)->second;
        }
        
        msgs[topic] = new Packet(buf, buf_size); //= Packet(buf, buf_size);
        
    }
    

    /**
        Send retained messages if any.
    */ 
    void sendRetainedMessage(string topic, ConnectionPtr sub){

        if(msgs.find(topic) != msgs.end()){
            sub->sendMessage(msgs[topic]->getBuf(), msgs[topic]->getSize());   
        }
    }

private:
    bool usingRedis;
    unordered_map<string, Packet*> msgs;
};
