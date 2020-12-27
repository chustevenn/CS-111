/* NAME: Steven Chu
 * EMAIL: schu92620@gmail.com
 * ID: 905094800
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <poll.h>
#include <errno.h>
#include <math.h>
#include <getopt.h>
#include <mraa.h>
#include <time.h>
#include <mraa/aio.h>
#include <poll.h>
#include <sys/socket.h>
#include <netdb.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

int interval = 1;
int log_flag = 0;
int log_fd;
char scale = 'F';
mraa_aio_context sensor;
mraa_gpio_context button;
time_t start, end;
struct pollfd p_fd;
int report = 0;
int stop = 0;
int port;
int sockfd;
char *id;
char *hostname;
SSL *ssl;

// Exit function to close pins and log file descriptor
void close_pins()
{
  if(log_flag == 1)
    close(log_fd);
  mraa_aio_close(sensor);
  mraa_gpio_close(button);
  SSL_shutdown(ssl);
  SSL_free(ssl);
}

// Helper function to calculate temperature from sensor value
double temperature(int sensor_val)
{
  double R = 1023.0 / ((double) sensor_val) - 1.0;
  int B = 4275;
  int R0 = 100000;
  R = R0*R;
  double temp = 1.0 / (log(R/R0)/B + 1/298.15) - 273.15;
  if(scale == 'F')
    {
      return temp*(9/5) + 48;
    }
  else
    return temp;
}

int main(int argc, char* argv[])
{
  // Handle arguments
  while(1)
    {
      int c;
      static struct option long_options[] =
	{{"period", required_argument, NULL, 1},
	 {"scale", required_argument, NULL, 2},
	 {"log", required_argument, NULL, 3},
	 {"id", required_argument, NULL, 4},
	 {"host", required_argument, NULL, 5},
	 {0, 0, 0, 0}};
      c = getopt_long(argc, argv, "", long_options, NULL);
      if(c == -1)
	break;
      switch(c)
	{
	   // Set period, invalid if less than 1
	   case 1:
	     interval = atoi(optarg);
	     if(interval < 1)
	       {
		 fprintf(stderr, "Invalid period.\n");
		 exit(1);
	       }
	     break;
	   // Set scale, invalid if not Celsius or Fahrenheit
	   case 2:
	     scale = optarg[0];
	     if(scale != 'F' && scale != 'C')
	       {
		 fprintf(stderr, "Invalid scale.\n");
		 exit(1);
	       }
	   // Create a log file descriptor
	   case 3:
	     log_flag = 1;
	     log_fd = creat(optarg, 0666);
	     if(log_fd == -1)
	       {
		 fprintf(stderr, "Failed to create the log file descriptor: %s\n", strerror(errno));
		 exit(1);
	       }
	     break;
	   case 4:
	     id = optarg;
	     if(strlen(id) != 9)
	     {
		 fprintf(stderr, "Provided ID is not 9 digits.\n");
		 exit(1);
	     }
	     break;
	   case 5:
	     hostname = optarg;
	     break;
	   default:
	     fprintf(stderr, "Unrecognized argument received.\n");
	     exit(1);
	}
	port = atoi(argv[argc-1]);
	if(port <= 0)
	{
	    fprintf(stderr, "Provided port number was not greater than 0.\n");
	    exit(1);
	}
    }
  // Initialize sensor and button pins
  sensor = mraa_aio_init(1);
  button = mraa_gpio_init(60);
  // Set button as input pin
  mraa_gpio_dir(button, MRAA_GPIO_IN);
  // Initialize TCP socket
  struct hostent *server;
  struct sockaddr_in server_add;
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if(sockfd == -1)
  {
	fprintf(stderr, "Error opening TCP socket: %s\n", strerror(errno));
	exit(2);
  }
  server = gethostbyname(hostname);
  if(server == NULL)
  {
	fprintf(stderr, "Error findng host: %s\n", strerror(errno));
	exit(2);
  }
  memset((char *) &server_add, 0, sizeof(server_add));
  server_add.sin_family = AF_INET;
  memcpy((char *) &server_add.sin_addr.s_addr, (char *) server->h_addr, server->h_length);
  server_add.sin_port = htons(port);
  if(connect(sockfd, (struct sockaddr *) &server_add, sizeof(server_add)) == -1)
  {
	fprintf(stderr, "Error connecting to server: %s\n", strerror(errno));
	exit(2);
  }
  // Initialize SSL
  OpenSSL_add_all_algorithms();
  SSL_load_error_strings();
  if(SSL_library_init() < 0)
  {
	fprintf(stderr, "Error initializing SSL.\n");
	exit(2);
  }
  // Create SSL context
  SSL_CTX *context = SSL_CTX_new(TLSv1_client_method());
  if(context == NULL)
  {
	fprintf(stderr, "Failed to create SSL context.\n");
	exit(2);
  }
  // Bind SSL to socket
  ssl = SSL_new(context);
  if(ssl == NULL)
  {
	fprintf(stderr, "Unable to complete SSL setup.\n");
	exit(2);	
  }
  if(SSL_set_fd(ssl, sockfd) < 0)
  {
	fprintf(stderr, "Failed to bind socket file descriptor.\n");
	exit(2);
  }
  // Connect SSL
  if(SSL_connect(ssl) != 1)
  {
	fprintf(stderr, "Failed to connect SSL.\n");
        exit(2);
  }
  // Report ID
  char id_report[128];
  sprintf(id_report, "ID=%s\n", id);
  if(SSL_write(ssl, id_report, strlen(id_report)) < 0)
  {
       fprintf(stderr, "Failed to write with SSL.\n");
       exit(2);
  }
  if(log_flag)
  {
	dprintf(log_fd, "ID=%s\n", id);
  }
  // Note start time
  time(&start);
  // Initialize polling structure
  p_fd.fd = sockfd;
  p_fd.events = POLLIN | POLLHUP | POLLERR;
  atexit(close_pins);
  // Begin loop for sensor readings
  while(1)
    {
      // Note current time, if enough time has passed, signal for a report
      time(&end);
      if(difftime(end, start) >= interval)
	{
	  report = 1;
	  time(&start);
	}
      else
	{
	  report = 0;
	}
      // Report if signal and sensor is active
      if(report == 1 && stop == 0)
	{
	  double temp = mraa_aio_read(sensor);
	  if(temp == -1)
	    {
		fprintf(stderr, "Error reading sensor input.\n");
		exit(2);
	    }
	  temp = temperature(temp);
	  // Note local time to output with reading
	  time_t timer;
	  time(&timer);
	  struct tm *time = localtime(&timer);
	  char report[256];
	  sprintf(report, "%02d:%02d:%02d %.1f\n", time->tm_hour, time->tm_min, time->tm_sec, temp);
	  if(SSL_write(ssl, report, strlen(report)) < 0)
	  {
		fprintf(stderr, "Failed to write with SSL.\n");
		exit(2);
	  }
	  if(log_flag == 1)
	    {
	      dprintf(log_fd, "%02d:%02d:%02d %.1f\n", time->tm_hour, time->tm_min, time->tm_sec, temp);
	    }
	}
      // Read input from button, if pressed, shut down.
      int shutdown = mraa_gpio_read(button);
      if(shutdown == -1)
	{
	  fprintf(stderr, "Error reading button input.\n");
	  exit(2);
	}
      if(shutdown == 1)
	{
	  time_t timer;
          time(&timer);
          struct tm *time = localtime(&timer);
	  char shutdown_report[256];
          sprintf(shutdown_report, "%02d:%02d:%02d SHUTDOWN\n", time->tm_hour, time->tm_min, time->tm_sec);
	  if(SSL_write(ssl, shutdown_report, strlen(shutdown_report)) < 0)
          {
                fprintf(stderr, "Failed to write with SSL.\n");
                exit(2);
          }
          if(log_flag == 1)
            {
              dprintf(log_fd, "%02d:%02d:%02d SHUTDOWN\n", time->tm_hour, time->tm_min, time->tm_sec);
            }
	  exit(0);
	}
      // Poll standard in 
      if(poll(&p_fd, 1, 0) == -1)
	{
	  fprintf(stderr, "Error polling: %s\n", strerror(errno));
	  exit(2);
	}
      // If input is available, read it and handle commands.
      // If command is invalid, do nothing.
      if(p_fd.revents & POLLIN)
	{
	  char buffer[128]; 
	  memset(buffer, 0, 128);
	  SSL_read(ssl, buffer, 128);	
	  if(log_flag == 1)
	    dprintf(log_fd, "%s", buffer);
	  if(strcmp(buffer, "SCALE=F\n") == 0)
	    scale = 'F';
	  else if(strcmp(buffer, "SCALE=C\n") == 0)
	    scale = 'C';
	  else if(strcmp(buffer, "STOP\n") == 0)
	    stop = 1;
	  else if(strcmp(buffer, "START\n") == 0)
	    stop = 0;
	  else if(strcmp(buffer, "OFF\n") == 0)
	    {
	      time_t timer;
	      time(&timer);
	      struct tm *time = localtime(&timer);
	      char shutdown_report[256];
	      sprintf(shutdown_report, "%02d:%02d:%02d SHUTDOWN\n", time->tm_hour, time->tm_min, time->tm_sec);
	      if(SSL_write(ssl, shutdown_report, strlen(shutdown_report)) < 0)
	      {
		fprintf(stderr, "Failed to write with SSL.\n");
		exit(2);
	      }
	      if(log_flag == 1)
		{
		  dprintf(log_fd, "%02d:%02d:%02d SHUTDOWN\n", time->tm_hour, time->tm_min, time->tm_sec);
		}
	      exit(0);
	    }
	  else if(strncmp(buffer, "PERIOD=", strlen("PERIOD=")) == 0)
	    {
	      char *period = buffer + strlen("PERIOD=");
	      interval = atoi(period);
	    }
	  else if(strncmp(buffer, "LOG ", strlen("LOG ")) == 0)
	    {} 
	}
      // Handle poll disconnects and errors
      if(p_fd.revents & POLLHUP)
	  exit(0);

      if(p_fd.revents & POLLERR)
	{
	  fprintf(stderr, "POLLERR detected: %s\n", strerror(errno));
	  exit(2);
	} 
    }
  exit(0);
}
