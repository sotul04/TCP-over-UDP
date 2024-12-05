#include "node.hpp"

Node::Node(string ip, uint16_t port) {
    this->ip = ip;
    this->port = port;
    this->connection = new TCPSocket(ip, port);
}

Node::~Node() {
    delete connection;
}

void Node::setData(vector<Segment> segments)
{
    this->data = segments;
}

vector<Segment> Node::getData() const 
{
    return data;
}