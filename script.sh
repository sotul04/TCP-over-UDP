# g++ -o server segment/segment.cpp segment/segment_handler.cpp message/message.cpp message/messageFilter.cpp socket/socket.cpp socket/TCPSocket.cpp node/node.cpp node/Server.cpp
# g++ -o client segment/segment.cpp segment/segment_handler.cpp message/message.cpp message/messageFilter.cpp socket/socket.cpp socket/TCPSocket.cpp node/node.cpp node/Client.cpp
g++ -o node segment/segment.cpp segment/segment_handler.cpp message/message.cpp message/messageFilter.cpp socket/socket.cpp socket/TCPSocket.cpp nodes/node.cpp nodes/Client.cpp nodes/Server.cpp node.cpp
