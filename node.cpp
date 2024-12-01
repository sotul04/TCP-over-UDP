#include "node.hpp"

Node::Node() {
    this->connection = new TCPSocket();
}