#pragma once
#include<string>
#include<unordered_map>

#include "Connection.hpp"
#include "RNode.hpp"

using namespace std;

/**
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
*/


class MessageManager{
public:
    MessageManager() = delete;

    MessageManager(bool usingRedis = false)
    :usingRedis(usingRedis){
        root = new RNode("root"); 
    }
    
    /**
        store the message of which the retain flag is set. 
    */
    void setRetainedMessage(string topics, string message, char* buf, ssize_t buf_size){
        RNode* cur = root;
        RNode* child;

        for(int i=0; i<topics.length(); i++){
            int start = i;
            while( topics[i] != '/' && i < topics.length()){
                i++;
            }
            std::string topic = topics.substr(start, i-start);

            child =  cur->find_child(topic); 
            
            if(child == nullptr){
                // If not exist
                cur = cur->make_child(topic);                 
            }else{
                cur = child; 
            }

        } // end for

       // Add the packet to the node.
       cur->setPacket(buf, buf_size);
         
    }
    

    /**
        Send retained messages if any.
    */ 
    void sendRetainedMessage(RNode* cur, string topics, ConnectionPtr sub){
        /**
        if(msgs.find(topic) != msgs.end()){
            sub->sendMessage(msgs[topic]->getBuf(), msgs[topic]->getSize());   
        }
        */

        /** home/#/room
            home/room1/temp

            root 
            - home 

        */
        for(int i =0; i < topics.length(); i++){
                
            int start = i;
            while(topics[i] != '/' && i < topics.length()){
                i++;
            }

            /** 
              1) Parse a single topic from the topic string.
            ------------------------------------------------ */
            std::string topic = topics.substr(start, i - start);
            
             /**
             if(!topic.compare("+")){
                    
             }else if(!topic.compare("#")){
                
             }
             */
                                
             if((cur = cur->find_child(topic)) == nullptr){         
                return;        
             }

            /**
              2) Find the single level wildcard(+)
            ------------------------------------------------     
            if( (child = cur->find_child("+")) != nullptr){
                
                if(i == topics.size()) // if the (+) is positioned at the end.
                    publish(child, "", buf, buf_size);        
                else
                    publish(child, topics.substr(i+1), buf, buf_size);       
            }
            */


        }// end for
        // cur / conn
        
        sub->sendMessage(cur->getPacket()->getBuf(), cur->getPacket()->getSize());   

    
    }

    RNode* getRootNode(){
        return root;
    }
private:
    bool usingRedis;
    unordered_map<string, Packet*> msgs;
    RNode* root;
};
