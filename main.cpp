#include <iostream>
#include "src/Broker.hpp"
using namespace std;


int main(int argc, char*[]) {
    cout << "C++ MQTT server" << endl;
    
    Broker broker;

    broker.run();


    return 0;
}
