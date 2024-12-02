#include "node.hpp"

Node::Node(string ip, uint16_t port) {
    this->connection = new TCPSocket(ip, port);
}

void Node::run() {
    
}

Node::~Node() {
    delete connection;
}