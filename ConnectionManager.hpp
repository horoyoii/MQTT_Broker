#pragma once
#include<unordered_map>

#include "Connection.hpp"
#include "Node.hpp"

class ConnectionManager{
public:
    ConnectionManager(){

    }

    void setRoot(Node* root){
        this->root = root;
    }
    void add_connection(int cid, ConnectionPtr conn){
        connections.insert({cid, conn});

    }

    int disconnect(int cid){

        /** 
            close request can be invaild when that belongs to a publisher.
        ---------------------------------------------------*/
        std::unordered_map<int, ConnectionPtr>::iterator iter = connections.find(cid);
        if(iter == connections.end())
            return 0;
        
        // close the socket()
        (iter->second)->disconnect();
        
        // remove conn object from connection pool        
        int res =connections.erase(cid);
        printf("erase from connection pool\n");

        // remove conn object from TopicTree 
        Node* node = mapping_info.find(cid)->second;
        node->remove_subscriber(cid);        
        
        while(node != root){ // not apply to the root node
            if(!node->hasSubscriber() && !node->hasChildren()){
                // if no subscriber and no children, then delete
                std::string topic = node->getTopic();
                Node* delNode = node;
                node = node->getParent();
                node->remove_child(topic);

                mapping_info.erase(cid);

                delete delNode;
            }else
                break;
        }
        return res;
    }

    void add_mapping_info(int cid, Node* node){
        mapping_info.insert({cid, node});
    }
    
    ConnectionPtr get_connection(int cid){
        return (connections.find(cid))->second;
    }

private:
    std::unordered_map<int, ConnectionPtr>  connections;
    std::unordered_map<int, Node*>          mapping_info;
    Node* root;
};
