//username: rishabg
//student number : 796799

#include<unistd.h>
#include<stdlib.h>
#include<fcntl.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<head.h>
char * ROOT;

//p is the pointer to socket being created for the client.
void *connection(void *p) {
  FILE *file_read;
  char *file_data;
  char recv_buf[1024];
  char send_buf[1024];
  char file_name[99999];
  char errormsg[69];
  int recv_len;
  int file_size;
  int temp;
  recv_len = 0;
  file_size = 0;
  int *conn_sock_thread = (int *)p;//type cast the argument for use in this thread
  
  //size of bufferr necessary in which client ask for get request can be assume to be small.
  temp = (recv (*conn_sock_thread, recv_buf, sizeof(recv_buf), 0));//rec the info of client in recv buffer.
   
  //check for error when receiving
  if (temp < 0){
    perror("Receive Error");
    exit(errno);
  }
    
    
  temp = temp/sizeof(char);
  recv_buf[temp-2] = '\0';
 
  //check to make sure that a GET request was made as first 4 character match GET .
  if ((strncmp(recv_buf, "GET ", 4)) != 0) {
    strcpy(errormsg, "Invalid Command Entered\nPlease Use The Format: GET <file_name.html>\n");
    send(*conn_sock_thread, errormsg, 69, 0);
  }
  
    
   
    //Other wise the name of the file is copied character by charcter.
  else {
   
    recv_len = strlen(recv_buf);
    int i;
    int j = 0;  //remove the forward slash if neccissary
    for (i=4; i<recv_len ;i++, j++) {
      if ( (recv_buf[i] == '\0') || (recv_buf[i] == '\n') || (recv_buf[i] == ' ') ){ //If the end of the file path is reached, break.
        break;
      }
      else {
        file_name[j] = recv_buf[i];				//copy the file name character by character
      }
    }
    file_name[j] = '\0';//add a null terminator to the end of the file name string
    char data[999];
    strcat(data,ROOT);
    strcat(data,file_name);
    strcpy(file_name,data);
    file_read = fopen(file_name, "r");					//open the html file
    //If the file did open without errors
   
    if (file_read == NULL) {
        send(*conn_sock_thread, "HTTP/1.1 404 Not Found\n\n", 24, 0);
    }
    else{
         //it helps to open files of type css,java script, html and jpeg and
        //sends the content type as their names to allow browser to open files of type mentioned.
          printf("\nfile successfully read is %s\n",file_name);
          if(strstr(file_name, ".html"))
          {

            sprintf(send_buf, "HTTP/1.1 200 OK\nContent Length: %d\nConnection: close\nContent-Type: text/html\n\n",  file_size);
            //format and create string for output to client
          }
          else if(strstr(file_name, ".css"))
          {
            sprintf(send_buf, "HTTP/1.1 200 OK\nContent Length: %d\nConnection: close\nContent-Type: text/css\n\n", file_size); 
            //format and create string for output to client
          }
          else if(strstr(file_name, ".jpg")|| strstr(file_name, ".jpeg"))
          {
            sprintf(send_buf, "HTTP/1.1 200 OK\nContent Length: %d\nConnection: close\nContent-Type: image/jpeg\r\n\r\n", file_size);
            //format and create string for output to client
          }
          else if(strstr(file_name, ".js"))
          {
            sprintf(send_buf, "HTTP/1.1 200 OK\nContent Length: %d\nConnection: close\nContent-Type: text/javascript\n\n", file_size);
            //format and create string for output to client
          }
          fseek(file_read, 0L, SEEK_END);	//seek to the end of the the file
          file_size = ftell(file_read);		//get the byte offset of the pointer(the size of the file)
          fseek(file_read, 0L, SEEK_SET);	//seek back to the begining of the file
          file_data = (char *)malloc(file_size + 1);//allocate memmory for the file data
          int a = strlen(send_buf);		//get the length of the string we just created
          send(*conn_sock_thread, send_buf, a, 0);	//send the info to the client
        //read the file data into a string
        int f;
        int w;
        f=fread(file_data,  (file_size),1, file_read);
          if(f<0){
                perror("Read error");
                exit(errno);
           };
          file_data[file_size] = '\0';
        //write the content and send through the socket
        w=write(*conn_sock_thread,file_data,file_size);
        if(w<0){
            perror("Write Error");
            exit(errno);
        }
          free(file_data);						//free the allocated memory for file da
          fclose(file_read);
    }
  }
  close(*conn_sock_thread);//close the current connection
  fflush(stdout);		//make sure all output is printed
  pthread_exit(NULL);   //exit the pthread with null
}


int main(int argc, char *argv[]) {
  int sfd;
  int conn_sock[100];
  pthread_t threads[100]; //100 posible threads can be encountered at a time to a server.
  int thread_count = 0;
  struct sockaddr_in addr;
  ROOT = getenv("PWD");
  if (argc < 2)
    {
        fprintf(stderr,"ERROR, no port provided\n");
        exit(1);
    }
    if(argc==3){
        strcpy(ROOT,argv[2]);
    }
    printf("Server started at port no. %s%s%s with root directory as %s%s%s\n","\033[92m",argv[1],"\033[0m","\033[92m",ROOT,"\033[0m");
    if( (sfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket Error");
        exit(errno);
  }
  memset(&addr.sin_zero, 0, sizeof(addr.sin_zero));
  //fill in the struct with IPV4 config, stores the port number in form of representation
  // by help of host to network short and allows any ip address to connect to server.
  addr.sin_family = AF_INET;
  addr.sin_port = htons(atoi(argv[1]));
  addr.sin_addr.s_addr = INADDR_ANY;

  //bind help to check if the  given port number is free and ready to listen
  //for incoming connections.
  if( bind(sfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("Bind Error");
    return (errno);
  }
  //loop to count the number of connections/threads at max a server can have.
  while(thread_count < 100){
    if( listen(sfd, 10) != 0) {
      perror("Listen Error");
      return (errno);
    }
    //accept helps to connect any client who wants to connect to given socket who is actively listening.
    //accept gives the new socket file descriptor for new clinet connected.
    conn_sock[thread_count] = accept(sfd, NULL, NULL);
    if (conn_sock[thread_count] < 0) {
      perror("Accept Error");
      return (errno);
    }
    //create a thread and receive data and apply a function called connection which serves the client on
    //given thread
    pthread_create(&threads[thread_count], NULL, connection, &conn_sock[thread_count]);
    pthread_detach(threads[thread_count]);
    thread_count++;
  }
  //close the listening socket which it is listening on.
  close(sfd);
  return 0;
}
