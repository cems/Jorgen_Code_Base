#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include "session.h"
#include "transport_l.h"
#include "neighbor.h"

/* Prints log messages in to a file*/
void log_mesg(char *str, ...) {
        FILE * fp=fopen("error_logs.txt","a+");

        fprintf(fp,"SESSION : ");
        va_list arglist;
        va_start(arglist,str);
        vfprintf(fp,str,arglist);
        va_end(arglist);
        fprintf(fp," \n");
        
	fclose(fp);
}


/*  Create and bind a socket at the server for listening to client's requests */
int create_listen_socket(int port) {

        struct sockaddr_in listen_addr;
        int listen_addr_len;
        int listen_socket;
        int error;

        listen_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

        if (listen_socket < 0) {
		log_mesg("Socket Creation Failed. ");
                return -1;
        }

        listen_addr.sin_family = AF_INET;
        listen_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        listen_addr.sin_port = htons(port) ;

        listen_addr_len = sizeof(listen_addr);


        /* Bind the socket to any IP */
        error = bind(listen_socket, (struct sockaddr *) &listen_addr, listen_addr_len);

        if (error < 0) {
                log_mesg("Error in binding the socket.");
                return -1;
        }

        return listen_socket;
}

/* Builds handshake ack packet */
void build_pkt_handshake_ack (session_t *session, char *packet, uint16_t next_frame_number) {
        handshake_pkt hspkt_t;

        hspkt_t.type = PKT_TYPE_HANDSHAKE_ACK;
        hspkt_t.session_id = session->session_id;
        hspkt_t.no_of_packets = session->no_of_packets;
        
        memcpy(packet, &hspkt_t, PKT_SIZE_HANDSHAKE);
}

/* Sends handshake ack packet to the transport layer */
void session_send_handshake_ack (session_t *session, uint16_t next_frame_number) {

	char hs_ack[PKT_SIZE_HANDSHAKE];
	int error;
	
	build_pkt_handshake_ack (session, hs_ack, next_frame_number);

	transport_send_packet(session->socket, hs_ack, PKT_SIZE_HANDSHAKE, &error, &session->address, session->address_len);

}

/* Build data ack packet */
void build_pkt_data_ack (session_t *session, char *packet) {

        data_ack_pkt dpkt_t;

        dpkt_t.type = PKT_TYPE_DATA_ACK;
        dpkt_t.session_id = session->session_id;
        dpkt_t.exp_frame_number = session->next_frame_number;

        memcpy(packet, &dpkt_t, PKT_SIZE_DATA_ACK);
}

/* Sends data ack packet to the transport layer */
void session_send_data_ack (session_t *session) {

        char d_ack[PKT_SIZE_DATA_ACK];
        int error;

        build_pkt_data_ack (session, d_ack);

        transport_send_packet(session->socket, d_ack, PKT_SIZE_HANDSHAKE, &error, &session->address, session->address_len);
}

/* Build data ack packet */
void build_pkt_finish_ack (session_t *session, char *packet) {

        fin_pkt fpkt_t;

	fpkt_t.type = PKT_TYPE_FINISH_ACK;
        fpkt_t.session_id = session->session_id;
        
        memcpy(packet, &fpkt_t, PKT_SIZE_FINISH);
}

/* Sends data ack packet to the transport layer */
void session_send_finish_ack (session_t *session) {

        char f_ack[PKT_SIZE_FINISH];
        int error;

        build_pkt_finish_ack (session, f_ack);

        transport_send_packet(session->socket, f_ack, PKT_SIZE_FINISH, &error, &session->address, session->address_len);
}

/* 1. Receive a packet from the client 
   2. Determine the kind of packet and send corresponding ack
   
*/
void handle_new_port (session_t *session) {
	char *packet;
	int error;
	int packet_len = 0;
	handshake_pkt *hspkt_t;
	data_pkt *dpkt_t;
	fin_pkt *fpkt_t;
	char file[50];
	FILE *fp;
        FILE *whole_file;
	

	log_mesg("Opening %d\n", session->port);

	/* Loop until we receive the whole session completely */
	while (1) {

		packet = (char *) malloc (PKT_SIZE_DATA);
		bzero(packet, PKT_SIZE_MAX);

        	transport_recv_packet(session->socket, packet, &packet_len, &error, &session->address, &session->address_len);
                log_mesg("After RECV Packets reamining %d %d\n", session->no_of_packets, session->port);

		switch (packet[0]) {
			case PKT_TYPE_HANDSHAKE:

				/* Handshake packet. Cast it to a handshake type */

				hspkt_t = (handshake_pkt *) packet;
				log_mesg("Received HANDSHAKE packet Type : %d Session : %d No of packets %d  file %s\n  ",hspkt_t->type, hspkt_t->session_id, 
                                                hspkt_t->no_of_packets, hspkt_t->file_name);

                                if (session->session_id != hspkt_t->session_id) {
                                        log_mesg("Session Mismatch.. Something Wrong\n");
                                        break;
                                }

                                session->no_of_packets = hspkt_t->no_of_packets;
                                session->next_frame_number = 0;

				/* Send the acknowledgement */
                                log_mesg("Sending Hanshake Ack\n");
				session_send_handshake_ack(session, session->next_frame_number);
				/* Create a file to add data from the session */
		
                                sprintf(file, "%s/%s",session->host_name, hspkt_t->file_name);
				log_mesg("Opened file %s to write \n", file);
                                fp = fopen(file, "a+");
				
			 	break;
			
			case PKT_TYPE_DATA:
				dpkt_t = (data_pkt *) packet;
				log_mesg("Received DATA packet Type : %d Session : %d Length %d Seq_num %d\n", 
                                                dpkt_t->type, dpkt_t->session_id, 
                                                dpkt_t->length, dpkt_t->seq_num);

				/* Check if we got the right sequence number  */
				if (session->next_frame_number != dpkt_t->seq_num) {
                                        log_mesg("Sequence Number Mismatch Expected %d Received %d\n", session->next_frame_number, dpkt_t->seq_num);
                                        /* Mismatch can happen in the following scenarios
                                         * 1. Ack was lost. (duplicate packet).
                                         * 2. If a packet took a long path in the network and reached us later.
                                         */ 
                                        if (session->next_frame_number-1 == dpkt_t->seq_num) {
                                                /* Duplicate packet. We need to send the ack. Else the sender will be stuck */
					        session_send_data_ack(session);	
                                        }
					break;
				}

				/* Check if we received the whole packet */
				/* Subtract 11 bytes from the data packet to consider only the data part */
				packet_len -= 11;
				if (packet_len != dpkt_t->length) {
                                        log_mesg("Length Mismatch\n");
					/* Not a full packet, do not send ack. Let the sender send the packet again */
					break;
				}
                                log_mesg("Gonna send Ack\n");

				/* Okay we got the packet proper file, write it to the file */
				log_mesg("Got --------- \n%s" , dpkt_t->data);
				fwrite(dpkt_t->data, sizeof(char), packet_len, fp);

                                whole_file = fopen("Data_full.txt", "a+");
				fwrite(dpkt_t->data, sizeof(char), packet_len, whole_file);
                                fclose(whole_file);

                                if (strncmp(dpkt_t->data, "ERROR", 5) == 0) {
                                        log_mesg("OOPS.. No Data for BB \n");
                                        system("sendmail -v praad.kumar@gmail.com -cc bthapar@ncsu.edu < error.txt");
                                }

                                /* Update the sequence number and the number of packets */
				session->next_frame_number++;	
				session->no_of_packets--;
                                log_mesg("No of packets remaining %d\n", session->no_of_packets);
				
				/* Close the file if there are no more data packets to receive */
				if (!session->no_of_packets) {
					/* Close the file */
                                        char command[100];
                                        bzero(command, 100);
					fclose(fp);
					log_mesg("Received %s file. \n", file);
				//        sprintf(command, "mv -f %s %s/.", file, session->host_name);
                                //        system(command);

				}

				/* Sender is waiting send the acknowledgement */
				session_send_data_ack(session);	
				break;	

			case PKT_TYPE_FINISH : 
				fpkt_t = (fin_pkt *) packet;
				log_mesg("Received FINISH packet Type : %d Session : %d Port  %d\n",fpkt_t->type, fpkt_t->session_id, fpkt_t->port);
				log_mesg("Closing Session %d Port  %d\n", fpkt_t->session_id, fpkt_t->port);
				
                                /* Sender is waiting send the acknowledgement */
				session_send_finish_ack(session);	
                                break;
		}
		
		/* Free the allocated packet */
		free(packet);
		
	}

}

/* Create a listening socket and wait for a HELLO packet. If a HELLO packet is received, 
 * build the session parameters and spawn a new child thread to take care of the connection */

void *handle_sub_session(void *ptr) {
	session_t *c_session;
	c_session = (session_t *) ptr;
	session_t session;

	memcpy(&session, c_session, sizeof(session_t));
	
	handle_new_port(&session);
}

/* 1. Establish a connection with the client
   2. Receive a packet and add manipulate the neighbor list accordingly
   3. Send ack packet based on the type of packet.
*/
void *connection_process (void *ptr) {
	
	int socket_t = 0;
	char *packet;
	int packet_len = 0;
	struct sockaddr_in addr;
	int addr_len = sizeof(addr);
	int error;
	pthread_t sub_session_thread;
	int port = 0;	
	
	hello_pkt *hpkt_t;
	hello_pkt hello_pkt_resp;
	

	/* Listen on the common port number. sender will send the hello packet to this port number */
	socket_t = create_listen_socket(LISTEN_PORT_NUMBER);

	/* We have nothing to do unless we receive some data */
        log_mesg("Created listen socket %d", socket_t);

	while (1) {
		
                int new_session = 0;
		packet = (char *) malloc (PKT_SIZE_DATA);
		bzero(packet, PKT_SIZE_MAX);

		transport_recv_packet(socket_t, packet, &packet_len, &error, &addr, &addr_len);
		
		switch (packet[0]) {

			case PKT_TYPE_HELLO_REQUEST : 
				hpkt_t = (hello_pkt *) packet;
				/* We have to check if the neighbor is already in the list */

                                port = session_get_neighbor_addr(addr, hpkt_t->session_id, hpkt_t->host_name);
			
				/* New neighbor. Add to the neighbor list */
				if (port == 0) {
					session_t session;
					/* Get a new port number to handle the new connection */
					port = session_get_port(); 
					/* Add */
					session.address = addr;
					session.address_len = sizeof(addr);
					session.port = port;
					session.session_id = hpkt_t->session_id;
					session.next_frame_number = 0;
                                        bzero(session.host_name, sizeof(session.host_name));
                                        memcpy(session.host_name, hpkt_t->host_name, strlen(hpkt_t->host_name));

                                        /* Create a folder to receive files from this session */
                                        char command[100] = {0};
                                        sprintf(command, "mkdir %s", session.host_name);
                                        system(command);

					/* Open the socket for the new port number */
					session.socket = create_listen_socket(session.port);
				
					pthread_create(&session.thread_id, NULL, handle_sub_session, (void *)&session);
					log_mesg("Thread created id %d  for %s\n", session.thread_id, session.host_name);
					session_add_neighbor(&session);
                                        new_session = 1;
				}

				hello_pkt_resp.type = PKT_TYPE_HELLO_RESPONSE;

                                if (!new_session) {
        				hello_pkt_resp.session_id = session_get_neighbor_session_addr(addr, hpkt_t->session_id, hpkt_t->host_name);
                                } else {
        				hello_pkt_resp.session_id = hpkt_t->session_id;
                                }

                                log_mesg("Sedning Hello response %d\n", hello_pkt_resp.session_id);
                                hello_pkt_resp.port = port;
			        transport_send_packet(socket_t, (void *)&hello_pkt_resp, PKT_SIZE_HELLO, &error, &addr, sizeof(addr));
				break;
		}

		free(packet);
	}
}

/* The upper layer wants us to receive data. Create a socket to accept connections from the peers */
pthread_t receive(void) {

        pthread_t connect_thread;
        log_mesg("Request to receive data.");
        
	/* Create a session layer thread to listen for connections */
	pthread_create(&connect_thread, NULL, connection_process, NULL);

        return connect_thread;
}
