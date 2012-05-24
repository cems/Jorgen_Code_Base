#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <strings.h>
#include <pthread.h>
#include <errno.h>
#include "session.h"

/* Invoke server by ivoking a receive() function */
int main(int argc, char *argv[])
{
        pthread_t session;
        session = receive();
//        gui();
        pthread_join(session, NULL);
        return 0;
}
