/* NAME: Steven Chu
 * EMAIL: schu92620@gmail.com
 * ID: 905094800
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <termios.h>
#include <poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <zlib.h>


z_stream in_stream;
z_stream out_stream;
int compress_flag = 0;

// Set poll to watch a target file descriptor
void watch_fd(struct pollfd *pollfds, int target, short events)
{
    pollfds->fd = target;
    pollfds->events = events;
}

// Handle signal SIGPIPE
void sigpipe_handler()
{
    write(2, "Signal SIGPIPE received.\n", 17); 
    exit(0);
}

// Free data structures of in_stream
void deallocate_instream()
{
    inflateEnd(&in_stream);
}

// Free data structures of out_stream
void deallocate_outstream()
{
    deflateEnd(&out_stream);
}

int main(int argc, char* argv[])
{
    int pipefd[2];
    int pipefd1[2];
    int pid;
    int shell_flag = 0;
    int port_flag = 0;
    int port_num;
    int sockfd;
    while(1)
    {
	int c;
	// Construct argument options to support only option --shell
	static struct option long_options[] = {
	    {"shell", required_argument, NULL, 1},
	    {"port", required_argument, NULL, 2},
	    {"compress", no_argument, NULL, 3},
	    {0, 0, 0, 0}};
	c = getopt_long(argc, argv, "", long_options, NULL);
	if(c == -1)
	    break;
	switch(c)
	{
	    case 1:
		shell_flag = 1;
		// Construct pipes to child
		if(pipe(pipefd) < 0 || pipe(pipefd1) < 0)
		{
		    fprintf(stderr, "System call pipe failure: %s\n", strerror(errno));
		    exit(1);
		}
		pid = fork();
		if(pid < 0)
		{
		    fprintf(stderr, "System call fork failure: %s\n", strerror(errno));
                    exit(1);
		}
		// child process
		if(pid == 0)
		{
		    // Close unused write end of pipe 1 and unused read end of pipe 2
		    if(close(pipefd[1]) == -1 || close(pipefd1[0]) == -1)
		    {
			fprintf(stderr, "Error closing file descriptors of child: %s\n", strerror(errno));
                        exit(1);
		    }	
		    // Redirecting stdin/stdout/stderr of child process
		    if(dup2(pipefd[0], 0) == -1)
		    {
			fprintf(stderr, "Failed to redirect stdin of child: %s\n", strerror(errno));
                        exit(1);
		    }
		    if(dup2(pipefd1[1], 1) == -1 || dup2(pipefd1[1], 2) == -1)
                    {
                        fprintf(stderr, "Failed to redirect stdout/stderr of child: %s\n", strerror(errno));
                        exit(1);
                    }
                    if(close(pipefd[0]) == -1 || close(pipefd1[1]) == -1)
                    {
                        fprintf(stderr, "Error closing file descriptors of child: %s\n", strerror(errno));
                        exit(1);
                    }
		    // Execute specified program
		    char *args[2];
		    args[0] = optarg;
		    args[1] = NULL;
		    if(execvp(optarg, args) == -1)
		    {
			fprintf(stderr, "Error executing %s in child: %s\n", optarg, strerror(errno));
                        exit(1);
		    }
		}
		break;
	    case 2:
		// Read in port number
		port_flag = 1;
		port_num = atoi(optarg);
		break;
	    case 3:
		// Initialize input and output streams
		compress_flag = 1;
                out_stream.zalloc = Z_NULL;
                out_stream.zfree = Z_NULL;
                out_stream.opaque = Z_NULL; 
                if(deflateInit(&out_stream, Z_DEFAULT_COMPRESSION) != Z_OK)
                {   
                    fprintf(stderr, "Failed to create compression stream.\n");
                    exit(1);
                }
                atexit(deallocate_outstream);
                in_stream.zalloc = Z_NULL;
                in_stream.zfree = Z_NULL;
                in_stream.opaque = Z_NULL; 
                if(inflateInit(&in_stream) != Z_OK)
                {   
                    fprintf(stderr, "Failed to create decompression stream.\n");
                    exit(1);
                }
                atexit(deallocate_instream);
                break;

	    default:
		fprintf(stderr, "Unrecognized argument received.\n");
		exit(1);
	}
    }
	// Check if port option was used
	if(port_flag == 0)
	{
	    fprintf(stderr, "Error: no port number specified.\n");
	    exit(-1);
	}
	// Create socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
	    fprintf(stderr, "Failed to create socket: %s\n", strerror(errno));
	    exit(1);
	}
	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port_num);
	int addr_len = sizeof(address);
	// Bind socket
	if(bind(sockfd, (const struct sockaddr *) &address, addr_len) < 0)
	{
	    fprintf(stderr, "Failed to bind socket: %s\n", strerror(errno));
	    exit(1);
	}
	// Listen for requests to connect
	if(listen(sockfd, 5) < 0)
	{
	    fprintf(stderr, "Error while listening: %s\n", strerror(errno));
	    exit(1);
	}
	// Accept connection on socket
	int socket_connect = accept(sockfd, (struct sockaddr *) &address, (socklen_t *) &addr_len);
	if(socket_connect < 0)
	{
	    fprintf(stderr, "Failed to connect socket: %s\n", strerror(errno));
	    exit(1);
	}
	else
	    fprintf(stderr, "Client successfully connected.\n");
	// parent process
	if(shell_flag == 1)
	{
	    // Close unused read end of pipe 1 and unused write end of pipe 2
	    if(close(pipefd[0]) == -1 || close(pipefd1[1]) == -1)
	    {   
		fprintf(stderr, "Error closing file descriptors of parent: %s\n", strerror(errno));
		exit(1);
	    }
	    if(signal(SIGPIPE, sigpipe_handler) == SIG_ERR)
	    {
		fprintf(stderr, "Error registering SIGPIPE handler: %s\n", strerror(errno));
		exit(1);
	    }
	    struct pollfd pollfds[2];
	    watch_fd(&pollfds[0], socket_connect, POLLIN | POLLHUP | POLLERR);
	    watch_fd(&pollfds[1], pipefd1[0], POLLIN | POLLHUP | POLLERR);
	    while(1)
	    {
		if(poll(pollfds, 2, -1) == -1)
		{
		    fprintf(stderr, "Error with polling: %s\n", strerror(errno));
		    exit(1);
		}
		// Case of input from socket
		if(pollfds[0].revents & POLLIN)
		{
		    // reserve extra space for decompression
		    char buf[2048];  
		    char *p = buf; 
		    int read_flag = read(socket_connect, &buf, 256*sizeof(char));
		    if(read_flag == -1)
		    {
			fprintf(stderr, "System call read failure: %s\n", strerror(errno));
			exit(1);
		    }
		    // Decompress input
		    if(compress_flag == 1)
		    {
			char inBuffer[2048];
			in_stream.avail_in = read_flag;
			in_stream.next_in = (unsigned char*) buf;
			in_stream.avail_out = 2048;
			in_stream.next_out = (unsigned char*) inBuffer;
			while(in_stream.avail_in > 0)
			{
			    if(inflate(&in_stream, Z_SYNC_FLUSH) != Z_OK)
			    {
				fprintf(stderr, "Failed to decompress input.\n");
				exit(1);
			    }
			}
			read_flag = 2048 - in_stream.avail_out;
			for(int i = 0; i < read_flag; i++)
			{
			    buf[i] = inBuffer[i];
			}
		    }
		    // Write input to shell
		    for(int i = 0; i < read_flag; i++)
		    {
			if(*p == 0x04)
			{
			    close(pipefd[1]);
			    break;
			}
			else if(*p == 0x03)
			{
			    if(kill(pid, SIGINT) == -1)
			    {
				fprintf(stderr, "Error sending SIGINT to child: %s\n", strerror(errno));
				exit(1);
			    }
			}
			else if(*p == 0x0D || *p == 0x0A)
			{
			    if(write(pipefd[1], "\n", 1) == -1)
                            {
                                fprintf(stderr, "System call write failure: %s\n", strerror(errno));
                                exit(1);
                            }
			}
			else
			{ 
			    if(write(pipefd[1], p, sizeof(char)) == -1)
			    {
				fprintf(stderr, "System call write failure: %s\n", strerror(errno));
				exit(1);
			    }
			}
			p += 1;
		    }
		}
		// Case of input from child
		if(pollfds[1].revents & POLLIN)
		{
		    char buf[256];
		    char *p = buf; 
		    int read_flag = read(pipefd1[0], &buf, 256*sizeof(char));
		    if(read_flag == -1)
		    {
			fprintf(stderr, "System call read failure: %s\n", strerror(errno));
			exit(1);
		    }
		    // Compress output
		    if(compress_flag == 1)
                    {
                        char outBuffer[256];
                        out_stream.avail_in = read_flag;
                        out_stream.next_in = (unsigned char*) buf;
                        out_stream.avail_out = 256;
                        out_stream.next_out = (unsigned char*) outBuffer;
                        while(out_stream.avail_in > 0)
                            if(deflate(&out_stream, Z_SYNC_FLUSH) != Z_OK)
                            {
                                fprintf(stderr, "Failed to compress output.\n");
                                exit(1);
                            }
                        read_flag = 256 - out_stream.avail_out;
                        for(int i = 0; i < read_flag; i++)
                        {
                            buf[i] = outBuffer[i];
                        }
                    }
		    // Write compressed output to socket
		    for(int i = 0; i < read_flag; i++)
		    {
			if(write(socket_connect, p, sizeof(char)) < 0)
			{
			    fprintf(stderr, "System call write failure: %s\n", strerror(errno));
			    exit(1);
			}
			p += 1;
		    }
		}
		// Case where either socket or child disconnects
		if((pollfds[0].revents & POLLHUP) || (pollfds[1].revents & POLLHUP))
		{
		    char buf[256];
		    char *p = buf; 
		    int read_flag;
		    while((read_flag = read(pipefd1[0], &buf, 256*sizeof(char))) != 0)
		    {
			if(read_flag == -1)
			{
			    fprintf(stderr, "System call read failure: %s\n", strerror(errno));
			    exit(1);
			}
			// Compress output
			if(compress_flag == 1)
			    {
				char outBuffer[256];
				out_stream.avail_in = read_flag;
				out_stream.next_in = (unsigned char*) buf;
				out_stream.avail_out = 256;
				out_stream.next_out = (unsigned char*) outBuffer;
				while(out_stream.avail_in > 0)
				    if(deflate(&out_stream, Z_SYNC_FLUSH) != Z_OK)
				    {
					fprintf(stderr, "Failed to compress output.\n");
					exit(1);
				    }
				read_flag = 256 - out_stream.avail_out;
				for(int i = 0; i < read_flag; i++)
				{
				    buf[i] = outBuffer[i];
				}
			    }
			// Write output to socket
			for(int i = 0; i < read_flag; i++)
			{ 
			    if(write(socket_connect, p, sizeof(char)) < 0)
			    {
				fprintf(stderr, "System call write failure: %s\n", strerror(errno));
                                exit(1);
			    }
			    p += 1;
			}
		    }
		    break;
		}
		// Case of stdin error
		if(pollfds[0].revents & POLLERR)
		{
		    fprintf(stderr, "Poll error in stdin: %s\n", strerror(errno));
		    break;
		}
		// Case of pipe 1 error
		if(pollfds[1].revents & POLLERR)
		{
		    fprintf(stderr, "Poll error in read pipe: %s\n", strerror(errno));
		    break;
		}
	  }
	int child_status;
	waitpid(pid, &child_status, 0);
	if(child_status == -1)
	{
	    fprintf(stderr, "System call waitpid failure: %s\n", strerror(errno));
	    exit(1);
	}
	fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n", WTERMSIG(child_status), WEXITSTATUS(child_status));
	exit(0);
    }

   // Default program without child process
    else if(shell_flag == 0)
    {
   // Continuously process input until escape sequence is detected
    while(1)
    {
	int read_bytes;
	char buf[256];
	char *p = buf;
	// Large read of 256 bytes
	read_bytes = read(socket_connect, &buf, 256*sizeof(char));
	if(read_bytes < 0)
	{
	    fprintf(stderr, "System call read failure: %s\n", strerror(errno));
	    exit(1);
	}
	    // EOF handling
	    if(*p == 0x04)
	    {
		exit(0);
	    }
	    else
	    {	
		if(write(socket_connect, p, sizeof(char)) < 0)
                {
                    fprintf(stderr, "System call write failure: %s\n", strerror(errno));
                    exit(1);
                }
	    }
	    p += 1;
	}
    }
}
