#ifndef CONNECTION_MANAGER
#define CONNECTION_MANAGER


#include "a_Connection.hpp"
#include <set>

/**
    Manage all connection to the broker 

*/
class ConnectionManager {
public:
 
    ConnectionManager();
  
    void start(ConnectionPtr c);
    void stop(ConnectionPtr c);
    //void stopAll();
  
private:

    std::set<ConnectionPtr> _connections;
};








#endif
