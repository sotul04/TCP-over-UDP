#ifndef config_tcp_hpp
#define config_tcp_hpp

#define MAXLINE 1500

#define HANDSHAKE_TIMEOUT 10
#define CLIENT_TIMEOUT 20
#define RETRANSMIT_TIMEOUT 5
#define BROADCAST_LISTEN_TIMEOUT 30

#define MAX_SEGMENT_SIZE MAXLINE
#define MAX_PAYLOAD_SIZE 1460

#define WINDOW_SIZE 8
#define CLEANER_TIME 10.0f
#define MIN_CLEANER_TIME 0.1f
#define CLEANER_LIMIT 5

#define RETRIES 10

#endif