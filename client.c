#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <mqueue.h>
#include <errno.h>

#define MAX_SIZE  1024
#define MSG_STOP  "exit"
#define END_MATCHING_LINES "*"

/* Global variables */
char *filename,  *sqname,  *keyword;

int main(int argc, char **argv) {
  
  mqd_t client_mq, server_mq;
  struct mq_attr attr;
  char buffer[MAX_SIZE], client_queue[80], *filename, *keyword;

  /* Reading command line input */
  if (argc < 4) {
    puts("\n\nPlease provide 3 command line arguements. 1. <sqname> 2. <filename> 3. <keyword>\n\n"); 
  }else {
    sqname = argv[1]; 
    filename = argv[2]; 
    keyword = argv[3]; 
printf("\nsqname : %s size : %zu, filename : %s size : %zu, keyword : %s size : %zu\n\n", sqname, strlen(sqname), filename, strlen(filename), keyword, strlen(keyword));
  } 

  /* Initialize client queue attributes */
  attr.mq_flags = 0;
  attr.mq_maxmsg = 10;
  attr.mq_msgsize = MAX_SIZE;
  attr.mq_curmsgs = 0;

  /* Generating the unique client queue name */
  pid_t pid = getpid();
  sprintf(client_queue, "/cq_%d", pid);

  /* Create a client posix message queue */
  client_mq = mq_open(client_queue, O_CREAT | O_RDWR,  0644, &attr);

  printf("> ");
  memset(buffer, 0, MAX_SIZE);
   
  /* Appending the file name and key word to the buffer */
  strcpy(buffer, client_queue); 
  strcat(buffer, "|");
  strcat(buffer, filename);
  strcat(buffer, "|");
  strcat(buffer, keyword);

  printf("Sending request to the server .... ");

  /* Open the server message queue and send a message to it with the client queue */
  server_mq = mq_open(sqname, O_WRONLY);
  int success;
  success = mq_send(server_mq, buffer, strlen(buffer)+1, 0);

  printf("Sending status = %s\n", strerror(success));

  /* Printing the received messgage */
  memset(buffer, 0, MAX_SIZE);
  puts("------------------------- SERVER RESPONSE --------------------------------");
  puts("\n\n---- Response ----\n");
  int msgsize, total_lines_recd=0;
  while ( (msgsize = mq_receive(client_mq, buffer, MAX_SIZE, NULL)) > 0) {
    if (buffer[0] == '*') {
      if (total_lines_recd == 0) puts("Sorry no matching lines received.");
      else printf("Total %d matching lines received.", total_lines_recd);
      printf("Received end char.\n");
      break;
    }
    else { // print the receiving lines
      total_lines_recd++;
      printf("%s\n", buffer);
      memset(buffer, 0, MAX_SIZE);
    }
  }

  /* Clean up resources used */
  mq_close(client_mq);
  mq_unlink(client_queue);
  puts("\n------------------------- REQUEST RESPONE DONE !!  --------------------------------");
  return 0;
}
