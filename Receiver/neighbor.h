#ifndef NEIGHBOR_H
#define NEIGHBOR_H

#include <pthread.h>
#include "session.h"

/* Node to maintain the neighbor information */
struct PEER {
	pthread_t thread_id;
	struct sockaddr_in peer_address;
        char host_name[10];
	uint32_t session_id;
	uint32_t port;
	int socket;
        int seq_num;
        int packets;
	struct NODE *next;
};


extern struct PEER *peer_list;

extern void session_initialize_neighbor_list();
extern int session_get_neighbor_addr(struct sockaddr_in addr, uint32_t session_id, char *host_name);
extern void session_update_neighbor_session_seq (uint32_t session_id, char *host_name);
extern void session_add_neighbor(session_t *session);
extern void session_del_neighbor(int port);
extern void neighbor_update_packets(int port, int packets);
#endif
