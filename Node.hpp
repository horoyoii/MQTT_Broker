#pragma once

#include<string>
#include<unordered_map>

#include "Connection.hpp"


class Node{
public:
    std::string                                 topic;
    std::unordered_map<std::string, Node*>      children;
    std::unordered_map<int, ConnectionPtr>      subscribers;
    
    Node();

    Node(std::string topic);

    /**
        Find the child of the topic.
        @return the Node pointer of the topic. if not exists, nullptr is returned.

        Based on this function, TopicTree.publish function will determine whether 
        continue to traverse or make a new node.

    */    
   Node* find_child(std::string topic); 
    
   void send_message(char* buf, ssize_t buf_size);

   void add_subscriber(ConnectionPtr conn);
    
   void remove_subscriber(int cid);

};


