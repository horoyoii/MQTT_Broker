#pragma once

#include<unordered_map>
#include<string>

#include "Connection.hpp"
#include "Node.hpp"
/**

    Topics 

            1) case-sensitive : myHome/temperature and MyHome/temperature is different.
            2) each LEVEL is separated by forward slash(/)

        Wildcards
            * to subscribe to multiple topics simulatenously.
            * only used by subscriber, not by publisher.
            * two different kinds of wildcards : single level / multi-level 
                
                1) single level : + 
                    
                    e.g. 
                        * myhome/room1/ + / temperature 
                        
                        this will match 
                        * myhome/room1/anywhere/temperature
                        * myhome/room1/somewhere/temperature

                        and will NOT match
                        * myhome/room1/somewhere/velocity 
                        
                2) multi level : #(hash symbol)
                   - this symbol is only at the end.
                    e.g.
                        * myhome/room1/# 
                      
                       this will match 
                       * myhome/room1/anywhere/temperaute
                       * myhome/room1/brightness

*/

class TopicTree{
public:
    TopicTree(){
        root = new Node;
    }
    
    /**
        Parse the topic string and make a new node if needed.
        Finally register a new subscriber to the node.
        
        (#) multi level wildcard 
         - will be stored as other string.
         - this node is a leaf node.

    */    
    Node* subscribe(ConnectionPtr conn, std::string topics){
        Node* cur = root;

        for(int i=0; i<topics.length(); i++){
            int start = i;
            while( topics[i] != '/' && i < topics.length()){
                i++;
            }

            std::string topic = topics.substr(start, i-start);
            // 1) process the topic
            /**
                move the child node if exists or create one   
             */    
            std::unordered_map<std::string, Node*>::iterator iter =  (cur->children).find(topic);      
            
 
            if(iter == (cur->children).end()){
                Node* newNode = new Node(topic);
                (cur->children).insert(make_pair(topic, newNode));
                 
                cur = newNode;
                printf("make new Node\n");
            }else{
                cur = iter->second; // second type is Node* 
            }

        }

        // 2) 
        /**
            add a connection info to the cur node 
        */
        cur->add_subscriber(conn);
        
        // also connection object needs to know where it belongs to
        //conn->set_node(cur);
                
         return cur;
    }
    
    /**
        Find the leaf node of topic tree and send the message to all of subscribers

        //TODO : handle the wildcard senario.
    */
    void publish(std::string topics, char* buf, ssize_t buf_size){
        Node* cur = root;
        
        // 1) find the leaf node to send the message 
        // -------------------------------------------------
        for(int i =0; i < topics.length(); i++){
            int start = i;
            while(topics[i] != '/' && i < topics.length()){
                i++;
            }
            std::string topic = topics.substr(start, i - start);
            
            cur = cur->find_child(topic);
            if(cur == nullptr){
                // no corresponding topic sequence
                return;
            }
        }
        
        // 2)TODO: send the message to all of subscribers
        // -------------------------------------------------
        cur->send_message(buf, buf_size);

    }
private:
    Node* root;
};
