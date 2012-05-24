#include "session.h"

void transport_send_packet(int socket_t, char *packet, int packet_len, int *error, struct sockaddr_in *address, int address_len);
void transport_recv_packet(int socket_t, char *packet, int *packet_len, int *error, struct sockaddr_in *address, int *address_len);
//void transport_recv_packet(int socket_t, char *packet, int *packet_len, int *error, struct sockaddr_in *address, int *address_len);
