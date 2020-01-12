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

            /** 1) process the topic
                move the child node if exists or create one   
             ---------------------------------------------------- */    
            std::unordered_map<std::string, Node*>::iterator iter =  (cur->children).find(topic);      
            
            if(iter == (cur->children).end()){
                Node* newNode = new Node(topic);
                (cur->children).insert(make_pair(topic, newNode));
                 
                cur = newNode;
            }else{
                cur = iter->second; // second type is Node* 
            }

        }

        /** 2)  add a connection info to the cur node 
        ---------------------------------------------------*/
        cur->add_subscriber(conn);
                
        return cur;
    }
    
    /**
        Find the node of topic tree and send the message to all of subscribers
        
        wildcard(+)(#) : these are handled in the recursive way.

    */
    void publish(Node* cur, std::string topics, char* buf, ssize_t buf_size){
        
        for(int i =0; i < topics.length(); i++){
                
            int start = i;
            while(topics[i] != '/' && i < topics.length()){
                i++;
            }

            
            /**
                1) Find the multi level wildcard(#)
            ------------------------------------------------- */
            Node* child;
            if( (child = cur->find_child("#")) != nullptr){
                publish(child, "", buf, buf_size);
            }
            

            /**
              2) Find the single level wildcard(+)
            ------------------------------------------------ */        
            if( (child = cur->find_child("+")) != nullptr){
                
                if(i == topics.size()) // if the (+) is positioned at the end.
                    publish(child, "", buf, buf_size);        
                else
                    publish(child, topics.substr(i+1), buf, buf_size);       
            }
            

            /** 
              3) Parse a single topic from the topic string.
            ------------------------------------------------ */
            std::string topic = topics.substr(start, i - start);
            if( (cur = cur->find_child(topic)) == nullptr ) // no corresponding topic sequence
                return;
            

        }// end for
        
        /** 2)send the message to all of its subscribers
         --------------------------------------------------- */
        cur->send_message(buf, buf_size);

    }

    Node* getRootNode(){
        return root;
    }
private:
    Node* root;
};
