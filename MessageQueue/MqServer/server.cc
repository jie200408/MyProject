#include "broker.hpp"

int main() {
    mq::BrokerServer server(8085, "./data/");
    server.start();
    return 0;
}