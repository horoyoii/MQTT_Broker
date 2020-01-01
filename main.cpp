#include "a_MqttServer.hpp"
#include <iostream>

using namespace std;


int main(int argc, char*[]) {
    cout << "C++ MQTT server" << endl;
    try {
        constexpr int port = 1883;
        MqttServer server(port);
        server.run();
        cout<<"end system"<<endl;

    } catch(const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
