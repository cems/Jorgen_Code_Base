#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <strings.h>
#include <stdarg.h>
#include <pthread.h>
#include <errno.h>
#include "neighbor.h"
#include "session.h"
#include "gui.h"

/* List to maintain info about the peers */
struct PEER *peer_list = (struct PEER *) NULL;

/* Returns all neigbor information maintained in the structure given the port number*/
int session_get_neighbor_port(int port) {

	struct PEER *local_peer_list = peer_list;
        
	while (local_peer_list) {
		if (local_peer_list->port == port) {
                        return 1;
                }
                local_peer_list = (struct PEER *) local_peer_list->next;
        }

        return 0;
}

/* Returns all neigbor information maintained in the structure */
int session_get_neighbor_addr(struct sockaddr_in addr, uint32_t session_id, char *host_name) {

        struct PEER *local_peer_list = peer_list;

	while (local_peer_list) {
		log_mesg("Compare %s and %s, %d and %d\n", local_peer_list->host_name, host_name, 
								local_peer_list->session_id, session_id);
                if (!memcmp(local_peer_list->host_name, host_name, strlen(host_name))) {
                        log_mesg("Looks like a duplicate session. %d\n", local_peer_list->session_id);
                        return local_peer_list->port;
		}
		log_mesg("Compare COOL %s and %s, %d and %d\n", local_peer_list->host_name, host_name, 
								local_peer_list->session_id, session_id);
		local_peer_list = (struct PEER *) local_peer_list->next;
	}
	return 0;
}


uint32_t session_get_neighbor_session_addr(struct sockaddr_in addr, uint32_t session_id, char *host_name) {

        struct PEER *local_peer_list = peer_list;
    
        while (local_peer_list) {
                if (!memcmp(local_peer_list->host_name, host_name, strlen(host_name))) {
                        return local_peer_list->session_id;
                }   
                local_peer_list = (struct PEER *) local_peer_list->next;
        }   
        return 0;
}

void session_update_neighbor_session_seq (uint32_t session_id, char *host_name) {
        struct PEER *local_peer_list = peer_list;
    
        while (local_peer_list) {
                if (!memcmp(local_peer_list->host_name, host_name, strlen(host_name)) && local_peer_list->session_id == session_id) {
                                local_peer_list->seq_num++;

                }   
                local_peer_list = (struct PEER *) local_peer_list->next;
        }   
        //session_print_neighbor(peer_list);
}

/* Updates the number of remaninig packets to be sent */
void neighbor_update_packets (int port, int packets) {
        struct PEER *local_peer_list = peer_list;
    
        while (local_peer_list) {

                log_mesg("Loop No of Packets reamining %d %d %d\n", packets, local_peer_list->port, port);
                if (local_peer_list->port == port) {
                        local_peer_list->packets = packets;
                        log_mesg("UPDATING TOTAL PACKETS %d\n", local_peer_list->packets);

                        break;
                }   
                local_peer_list = (struct PEER *) local_peer_list->next;
        }   
        //session_print_neighbor(peer_list);
}


/*
void session_initialize_neighbor_list() {
	peer_list == NULL;
}*/


/* Assign a new port to each of the clients
	This also reuses ports. It scans the entire list and assigns the first free port.
*/
int session_get_port() {

	int port = LISTEN_PORT_NUMBER + 1;

	while (1) {
		if (session_get_neighbor_port(port)) {
			port++;
			continue;
		} else {
			return port;
		}
	}
}


/* Adds a new session of a client as a neighbor in the neighbor list */
void session_add_neighbor(session_t *session) {
		
	struct PEER *local_peer_list;
	/* Allocate the node */
	struct PEER *peer_node;

	local_peer_list = peer_list;

	peer_node = (struct PEER *)malloc(sizeof(struct PEER));
	peer_node->peer_address = session->address;
	peer_node->session_id = session->session_id;
	peer_node->port = session->port;
	peer_node->thread_id = session->thread_id;
	peer_node->socket = session->socket;
        peer_node->seq_num = 0;
        memcpy(peer_node->host_name, session->host_name, strlen(session->host_name));
	peer_node->next = NULL;

	printf("Adding neighbor Addr %s, port %d, session %d , thread %d Host %s\n", 
                        inet_ntoa(peer_node->peer_address.sin_addr), peer_node->port, 
                        peer_node->session_id, peer_node->thread_id, peer_node->host_name); 

	if (peer_list == NULL) {
		/* First node */
		peer_list = peer_node;
	} else {
		while (local_peer_list->next != NULL) {
			local_peer_list = (struct PEER *) local_peer_list->next;
		}
		local_peer_list->next = peer_node;
	}

//        session_print_neighbor(peer_list);
}

/* Deletes a client's session from the neighbor list once all data has been transferred */
void session_del_neighbor(int port) {
        struct PEER *local_peer_list = (struct PEER *) peer_list, *prev = (struct PEER *) peer_list;

	if (peer_list == NULL) {
		log_mesg("Nothing to delete.\n");
		return;
	}
	
        if (local_peer_list->port == port) {
        	log_mesg("Deleting neighbor Addr %s, port %d, session %d , thread %d\n", inet_ntoa(local_peer_list->peer_address.sin_addr), local_peer_list->port, 
									local_peer_list->session_id, local_peer_list->thread_id); 
		peer_list = peer_list->next;
		/* Close the socket */	
		sleep(1);
		close(local_peer_list->socket);
		pthread_cancel(local_peer_list->thread_id);
		free (local_peer_list);
          //      session_print_neighbor(peer_list);
		return;
	}
	
	prev = local_peer_list;
        local_peer_list = (struct PEER *) local_peer_list->next;
	        	
	
	while (local_peer_list) {
                if (local_peer_list->port == port) {
	                log_mesg("Deleting neighbor Addr %s, port %d, session %d , thread %d\n", inet_ntoa(local_peer_list->peer_address.sin_addr), 
                                         local_peer_list->port, local_peer_list->session_id, local_peer_list->thread_id); 
			prev->next = local_peer_list->next;
			/* Close the socket */	
			close(local_peer_list->socket);
			pthread_cancel(local_peer_list->thread_id);
			free(local_peer_list);
//                        session_print_neighbor(peer_list);
			return;
		}
		prev = local_peer_list;
		local_peer_list = (struct PEER *) local_peer_list->next;
	}
	log_mesg("Already deleted.\n");
}
