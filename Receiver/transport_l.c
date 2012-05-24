#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <linux/sockios.h>
#include "session.h"
#include "transport_l.h"

/* Sends a packet to the transport layer by invoking sendto() system call */
void transport_send_packet(int socket_t, char *packet, int packet_len, int *error, struct sockaddr_in *address, int address_len) {

        int result;

        result = sendto(socket_t, packet, packet_len, 0, (struct sockaddr *)address, (socklen_t)address_len);

        if (result == -1) {

                printf("Packet not sent. %s %d \n", packet, packet_len);
        }

}

/* Receives a packet from the transport layer by invoking recvfrom() system call */
void transport_recv_packet(int socket_t, char *packet, int *packet_len, int *error, struct sockaddr_in *address, int *address_len) {


        *packet_len = recvfrom(socket_t, packet, PKT_SIZE_MAX, 0, (struct sockaddr *)address, (socklen_t *)address_len);

        if (*packet_len == -1) {

                printf("Received a NUll packet \n");
        }
}
