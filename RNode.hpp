#pragma once 

#include<string>
#include<unordered_map>

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


class RNode{
private:
    std::string                                 topic;
    std::unordered_map<std::string, RNode*>     children;
    RNode*                                      parent; 
    Packet*                                     packet;

public:
    RNode(std::string topic)
    :topic(topic){}

    RNode* find_child(std::string topic){
        std::unordered_map<std::string, RNode*>::iterator iter = children.find(topic);
        if(iter == children.end())
            return nullptr;

        return iter->second;
    }
        
       
   RNode* make_child(std::string topic){
        RNode* newNode = new RNode(topic);
        newNode->parent = this;
        
        children.insert({topic, newNode});
    
        return newNode;
    }

    void setPacket(char* buf, ssize_t buf_size){
        if(packet != nullptr)
            delete packet;

        packet = new Packet(buf, buf_size);

    }
    
    Packet* getPacket(){
        return packet;
    }
};


