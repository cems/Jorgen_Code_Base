#ifndef SESSION_H
#define SESSION_H

/* Common port number through which all senders send the Hello and Finish packet */
#define LISTEN_PORT_NUMBER      8000 

/* Maximum size of data chunk that can be sent in a packet */
#define MAX_FRAME_SIZE 		  	1000

/* Define packet sizes */
#define PKT_SIZE_HELLO			7 + 10/* sizeof(uint8_t) + sizeof(uint16_t) + sizeof(char)*16 */ 
#define PKT_SIZE_HANDSHAKE		9 + 15
#define PKT_SIZE_DATA 			11 + MAX_FRAME_SIZE
#define PKT_SIZE_DATA_ACK 		9
#define PKT_SIZE_FINISH			5 /* sizeof(uint8_t) + sizeof(uint32_t) */ 
#define PKT_SIZE_MAX 			PKT_SIZE_DATA /* Maximum of all the above packet types */

/* packet types */
#define PKT_TYPE_HELLO_REQUEST		  1
#define PKT_TYPE_HELLO_RESPONSE 	  2
#define PKT_TYPE_HANDSHAKE  	  	  3
#define PKT_TYPE_HANDSHAKE_ACK    	  4
#define PKT_TYPE_DATA             	  5
#define PKT_TYPE_DATA_ACK         	  6
#define PKT_TYPE_FINISH             	  7
#define PKT_TYPE_FINISH_ACK         	  8

/* Data that need to be passed from the main session layer to the sub session layer */
typedef struct sub_thread_data
{	
	struct sockaddr_in address;
	uint16_t port;	
} peer_data;


typedef struct session_parameters {
        uint32_t session_id;
        uint32_t socket;
        struct sockaddr_in address;
        int address_len;
        uint16_t port;
        uint16_t no_of_packets;
        uint32_t no_of_bytes;
        uint16_t next_frame_number;
        char host_name[10];
	pthread_t thread_id;
} session_t;

/* Definition of different packets. Update the size above if the values are modified */
#pragma pack(push, 1)

typedef struct hello_packet {
	uint8_t  type;
	uint32_t session_id;
	uint16_t port;
        char host_name[10];
} hello_pkt;

typedef struct handshake_packet {
        uint8_t  type;
        uint32_t session_id;
        uint16_t no_of_packets;
        char file_name[15];
} handshake_pkt;

typedef struct data_packet {
        uint8_t  type;
        uint32_t session_id;
        uint32_t length;
        uint16_t seq_num;
        char     data[MAX_FRAME_SIZE];
} data_pkt;

typedef struct acknowledgement {
        uint8_t  type;
        uint32_t session_id;
        uint16_t exp_frame_number;
} data_ack_pkt;

typedef struct finish_packet {
        uint8_t  type;
        uint32_t session_id;
	uint16_t port;
} fin_pkt;

#pragma pack(pop)

extern pthread_t receive(void);

void log_mesg(char *str, ...);
#endif
