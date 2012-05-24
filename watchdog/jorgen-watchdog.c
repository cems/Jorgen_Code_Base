#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>
#include <errno.h>

#define LINK_BROKEN 1
#define LINK_NOT_BROKEN 0
#define FILE_WRITE 1
#define FILE_NOT_WRITE 0
#define TIMEOUT 43200
#define INTERVAL 20*60
#define CONNECTED 1
#define NOT_CONNECTED 0

int detectLinkBreakage(char ip_addr[], char port[])
{
  int sockdes, rv;//sockdes->socket descriptor, rv->return value.
  struct addrinfo help, *server_addrinfo, *info;//TCP connection variables.

  memset(&help, 0, sizeof help);
  help.ai_family = AF_UNSPEC;
  help.ai_socktype = SOCK_STREAM;

  if ((rv = getaddrinfo(ip_addr, port, &help, &server_addrinfo)) != 0) {
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
      return LINK_BROKEN;
  }

  // loop through all the results and connect to the first we can
     for (info = server_addrinfo; info != NULL; info = info->ai_next) {
            if ((sockdes = socket(info->ai_family, info->ai_socktype,info->ai_protocol)) == -1) {
                continue;
            }
            // connection to the server is initiated after which this program halts to listen 
            if (connect(sockdes, info->ai_addr, info->ai_addrlen) == -1) {
                close(sockdes);
        	if (errno == ECONNREFUSED)
                    return LINK_NOT_BROKEN;
            	continue;
            }
            close(sockdes);
            break;
     }
  
  
     if (info == NULL)
         return LINK_BROKEN;
     else
         return LINK_NOT_BROKEN;
}

void sendMail(FILE *fp, char *sub, char *mail_body)
{
  char buffer[50];
    int temp;

  sprintf(buffer, "\nSending mail\n");
  temp = fwrite(buffer, sizeof(char), strlen(buffer), fp);

  memset(buffer, 0, sizeof(buffer));
  //sprintf(buffer, "mail -s %s praad.kumar@gmail.com < %s", sub, mail_body);
  //system((char *)buffer);
  system("sendmail -v praad.kumar@gmail.com -cc bthapar@ncsu.edu < error.txt");
}

//some routine to get file credentials.
int checkLogFile(char *log_file, int *size)
{
  FILE *fp1;
  int final;

  fp1 = fopen(log_file, "rb");
  if (!fp1)
        return FILE_NOT_WRITE;
  fseek(fp1, 0L, SEEK_END);
  final = ftell(fp1);//check with size of 20 min ago.
  fclose(fp1);

  if (final > *size) {
        *size = final;
        return FILE_WRITE;
  } else {
        *size = final;
        return FILE_NOT_WRITE;
  }
}

/*
 * Main, periodically checks for connection error and mails report accordingly.
 */
int main( int argc, char *argv[] )
{
  FILE *fp1, *fp2;//fp1->used to write into backup, fp2->to read command output.
  int temp, logfile_size = 0;
  char buffer[100];//buffer to store command output.
  time_t rawtime, start, end;//timestamp.
  double dif;//variable to keep track of timeout.
  struct tm * timeinfo;//timestamp.
  int connected = CONNECTED;//status variable to record connection status.

  if (argc != 6) {
      printf("\nArguments: dumpfile ip-addr port logfile mail-body");
      exit;
  }


  while(1) {
  fp1 = fopen(argv[1], "a+");
  if (!fp1)
        return EINVAL;

     fprintf(fp1,"Checking for connection..\n");
     time (&rawtime);
     timeinfo = localtime (&rawtime);
     memset(buffer, 0, sizeof(buffer));
     sprintf(buffer, "\nThe current date/time is: %s\n", asctime(timeinfo));
     temp = fwrite(buffer, sizeof(char), strlen(buffer), fp1);

     memset(buffer, 0, sizeof(buffer));
     fp2 = popen("ifconfig p2p1", "r");
     while (fgets(buffer, sizeof(buffer)-1, fp2) != NULL)
            temp = fwrite(buffer, sizeof(char), strlen(buffer), fp1);
     pclose(fp2);

     if (detectLinkBreakage(argv[2], argv[3])) {
     	 fprintf(fp1,"LINK BROKEN..\n");
         if (connected == NOT_CONNECTED) {//was disconnected earlier.
             time(&end);
             dif = difftime (end,start);
             if (dif > TIMEOUT) {
                 sendMail(fp1, "12 hours ended!! No connectivity!!", argv[5]);
                 time(&start);//resetting it for the next 12 hours.
             }
         } else {//disconnected first time.
             connected = NOT_CONNECTED;
             time(&start);
             sendMail(fp1, "Link Broken!!", argv[5]);
         }
     } else {
     	fprintf(fp1,"LINK NOT BROKEN..\n");
        if (connected == NOT_CONNECTED) {//was disconnected earlier.
            connected = CONNECTED;
        }
     }
     if (checkLogFile(argv[4], &logfile_size)) {//Checking if logfile is being written into.
     	fprintf(fp1,"FILE WRITTEN..\n");
//	sendMail(fp1, "Connectivity back, logfile written", argv[5]);
     } else {
     	fprintf(fp1,"FILE NOT WRITTEN..\n");
	sendMail(fp1, "Connectivity back, logfile NOT written", argv[5]);
     }
     fclose(fp1);
     sleep(INTERVAL);
  }

  return 0;
}
