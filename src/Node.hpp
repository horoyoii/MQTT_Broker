#pragma once

#include<string>
#include<unordered_map>

#include "Connection.hpp"


class Node{
private: 
    std::string                                 topic;
    std::unordered_map<std::string, Node*>      children;
    Node* parent;
public:
    
    // for Tnode
    std::unordered_map<int, ConnectionPtr>      subscribers;

    // for all
    Node();
    Node(std::string topic);

   // for all
   Node* find_child(std::string topic); 
    

   void send_message(char* buf, ssize_t buf_size);

   // for Tnode
   void add_subscriber(ConnectionPtr conn);
   
   // for all 
   Node* make_child(std::string topic); 
        
   // for Tnode
   void remove_subscriber(int cid);
   
   // for Tnode
   void remove_child(std::string topic);

   // for all
   Node* getParent();
   std::string getTopic();
   
   // for Tnode 
   bool hasSubscriber();

   // for all
   bool hasChildren();  

    ~Node();
};


