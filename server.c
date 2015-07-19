#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <mqueue.h>

#define MAX_SIZE 1024
#define END_MATCHING_LINES "*"
#define MAX_NO_OF_REQUESTS 3

/* Global variables */
char* filename, *client_queue, *keyword, *sqname;

/* Prototypes */
void parse_input(char* );
void send_response(char* );
int file_exist(const char*);

int main(int argc, char **argv){

  mqd_t server_mq;
  struct mq_attr attr;
  char buffer[MAX_SIZE];
  int stop = 0;
  static int concurrent_request_count = 0;

  /* Reading command line arguements */
  if (argc < 2) { 
    puts("/n/nPlease provide server queue name in the command line arguements. 1. <sqname>");
    exit(EXIT_FAILURE);
  }
  else { 
    sqname = argv[1]; 
  }

  /* Initializing server queue attributes */
  attr.mq_flags = 0;
  attr.mq_maxmsg = 3;
  attr.mq_msgsize = MAX_SIZE;
  attr.mq_curmsgs = 0;

  /* Create the server posix message queue */
  int rc = mq_unlink(sqname);
  server_mq = mq_open(sqname,  O_CREAT | O_RDONLY, 0644, &attr);

  /* The below do-while loop will receive the message */
  while(1) {
    
    printf("\nServer listening to client requests ... Currently serving %d requests. :) \n", concurrent_request_count);    
    /* Including the logic for forking the incoming requests to the child */

    int msgsize;
    msgsize = mq_receive(server_mq, buffer, MAX_SIZE, NULL);

    if (msgsize > 0 && ++concurrent_request_count <= MAX_NO_OF_REQUESTS) { // means fork
      int pid = fork();
      if (pid == 0) { 
	 sleep(10);
	 puts("\n------------------------- SERVER WORKING ON CLIENT RESPONSE --------------------------------\n");
	 printf("Client request with client queue name : %s being served ....\n", client_queue);
	 printf("Received request message %s\n",  buffer);

	 /* Parsing the incoming input */
	 parse_input(buffer);

	 printf("\nFilename specified : %s, Size : %zu", filename, strlen(filename));
	 printf("\nClient Queue : %s, Size : %zu", client_queue, strlen(client_queue));
	 printf("\nKeyword specified : %s, Size : %zu", keyword, strlen(keyword));

	 /* Send response back */
	 send_response(filename);
	 --concurrent_request_count;
	 puts("\n------------------------- SERVER JOB DONE --------------------------------\n");
	 // exit(EXIT_SUCCESS); 
      }
      else { // Keep the parent process listening to requests.
	if (concurrent_request_count > MAX_NO_OF_REQUESTS) {
	  puts("Server serving to its capacity.... Please try in 10 seconds.");
	  --concurrent_request_count;
	}
	continue;
      }
    }
  }

  /* Close and server queue and unlink it */
  mq_close(server_mq);
  mq_unlink(sqname);

  return 0;
}

void parse_input(char buffer[]) {

      /* Parsing the incoming input */
      int i = 1;
      char *token = buffer;
      while ( (token = strtok(token, "|")) != NULL ) {
	if (i == 1) {
	  client_queue = token;
	  i += 1;
	} 
	else if (i == 2) {
	  filename = token;
	  i += 1;
	}
	else {
	  keyword = token;
	}	
      token = NULL; // If not given then you have an infinite loop as token never changes and logic runs over the same string
   }
}

void send_response(char *filename) {
  FILE *fp;
  char ch[MAX_SIZE];

  /* Sending the message back to the client queue */
  int success, total_lines_sent;
  mqd_t client_mq = mq_open(client_queue, O_WRONLY);

  /* check if the file exists before opening it */
  int result = file_exist(filename);
  if (result == 0) {
    puts("File does not exist. Please try some other file.");
    char *s = "File does not exist. Please try with some other file name.";
    mq_send(client_mq, s, strlen(s)+1, 1);
  }
  else {
    /* Open the file to read */
    fp = fopen(filename, "r");  
    printf("\nSending response back to : %s from file : %s, based on keyword : %s\n ", client_queue, filename, keyword);
  
    /* Go throught the file and print a line only if the line contains the given keyword */
    total_lines_sent = 0;
    while ( fgets(ch, MAX_SIZE, fp) ) {
      char ch_copy[MAX_SIZE];
      strncpy(ch_copy, ch, sizeof(ch));
      ch_copy[strlen(ch_copy)-1] = '\0';
      char *pch = strtok(ch_copy, " ");
      while (pch != NULL) {
	if (strcmp(pch, keyword) == 0 ) {
	  total_lines_sent++;
	  success = mq_send(client_mq,  ch, strlen(ch)+1, 1);
	  break;
	}
	pch = strtok(NULL, " ");
      } 
    }
   
    if (ferror(fp)) {
      fprintf(stderr, "Error reading the file");
    }
    fclose(fp);
  } 
  
  success = mq_send(client_mq, END_MATCHING_LINES, strlen(END_MATCHING_LINES), 1); // sending endline character
  printf("Total number of lines sent: %d\n", total_lines_sent);
  puts("Message successfully send back to the client. Please check client window !");
  memset(ch, 0, MAX_SIZE);
}

int file_exist(const char *filename) {
   struct stat st;
   int result = stat(filename, &st);
   return result == 0; // 0 means file exists and -1 means does not exist
} 
