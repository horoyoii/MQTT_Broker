#include"ConnectionManager.hpp"


ConnectionManager::ConnectionManager() {}

void ConnectionManager::start(ConnectionPtr c) {
    _connections.insert(c);
    c->start();
}

void ConnectionManager::stop(ConnectionPtr c) {
    _connections.erase(c);
    c->stop();
}
