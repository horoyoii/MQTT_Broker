#include "Node.hpp"

Node::Node(){}

Node::Node(std::string topic)
:topic(topic){}


/**
    Find the child of the topic.
    @return the Node pointer of the topic. if not exists, nullptr is returned.

    Based on this function, TopicTree.publish function will determine whether 
    continue to traverse or make a new node.

*/    
Node* Node::find_child(std::string topic){
    std::unordered_map<std::string, Node*>::iterator iter = children.find(topic);
    if(iter == children.end())
        return nullptr;

    return iter->second;
}



void Node::send_message(char* buf, ssize_t buf_size){     
    
    for(std::pair<int, ConnectionPtr> sub : subscribers){
        int count = (sub.second)->sendMessage(buf, buf_size);

        printf("[MQTT / %d] PUBLISH to client / write size : %d\n", (sub.second)->get_cid(), count);
        
    }

}

void Node::add_subscriber(ConnectionPtr conn){
    subscribers.insert({conn->get_cid(), conn});
}

void Node::remove_subscriber(int cid){
    subscribers.erase(cid);
   
}

void Node::remove_child(std::string topic){
    children.erase(topic);
}

Node* Node::getParent(){
    return parent;
}

std::string Node::getTopic(){
    return topic;
}


bool Node::hasSubscriber(){
    return !subscribers.empty();
}

bool Node::hasChildren(){
    return !children.empty();
}

Node::~Node(){
    std::cout<<"Node destructor"<<std::endl;
}
