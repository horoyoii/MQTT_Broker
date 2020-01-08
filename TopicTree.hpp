#include<unordered_map>
#include<string>
#include<vector>

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

typedef int ConnectionPtr;

struct Node{
    std::string topic;
    std::unordered_map<std::string, Node*> children;
    std::vector<ConnectionPtr> subscribers;
    
    Node(){}

    Node(std::string topic)
    :topic(topic){}

};


class TopicTree{
public:
    TopicTree(){
        root = new Node;
    }
    
    /**
        (#) multi level wildcard 
         - will be stored as other string.
         - this node is a leaf node.

    */    
    void subscribe(ConnectionPtr conn, std::string topics){
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
        (cur->subscribers).push_back(conn);

    }
    
private:
    Node* root;

};
