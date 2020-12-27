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

struct termios tattr_orig;
int port_number;
int port_flag = 0;
int log_flag = 0;
int sockfd;
int compress_flag = 0;

z_stream in_stream;
z_stream out_stream;

// Restore original terminal modes upon exit
void terminate()
{   
    const struct termios tattr_original = tattr_orig;
    if(tcsetattr(0, TCSANOW, &tattr_original) == -1)
    {   
        fprintf(stderr, "Could not return termios to default: %s\n", strerror(errno));
        exit(-1);
    }
}

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

// Free data structures of out_stream upon exit
void deallocate_outstream()
{
    deflateEnd(&out_stream);
}

// Free data structures of in_stream upon exit
void deallocate_instream()
{
    inflateEnd(&in_stream);
}

int main(int argc, char* argv[])
{
    struct termios tattr;
    int log_fd;
    while(1)
    {
	int c;
        // Construct argument options
        static struct option long_options[] = {
            {"port", required_argument, NULL, 1},
            {"log", required_argument, NULL, 2},
	    {"compress", no_argument, NULL, 3},
            {0, 0, 0, 0}};
        c = getopt_long(argc, argv, "", long_options, NULL);
        if(c == -1)
            break;
        switch(c)
        {
            case 1:
		// Read in specified port number
                port_flag = 1;
                port_number = atoi(optarg);
                break;
            case 2:
		// Create log file descriptor
                log_flag = 1;
		log_fd = creat(optarg, 0666);
		if(log_fd == -1)
		{
		    fprintf(stderr, "Failed to create log file descriptor: %s\n", strerror(errno));
		    exit(1);
		}
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
	// Check if port argument was used
	if(port_flag == 0)
        {
            fprintf(stderr, "Error: Port not specified.\n");
            exit(-1);
        }
        // Create TCP socket
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if(sockfd == -1)
        {
            fprintf(stderr, "Failed to create socket: %s\n", strerror(errno));
            exit(1);
        }
        // Initialize address structure
        struct sockaddr_in server_address;
	struct hostent *addr;
	addr = gethostbyname("localhost");
        bzero((char *) &server_address, sizeof(server_address));
        server_address.sin_family = AF_INET;
        memcpy((char *) &server_address.sin_addr.s_addr, (char *) addr->h_addr, addr->h_length);
        server_address.sin_port = htons(port_number);
        // Connect to server
        //const struct sockaddr server = server_address;
        if(connect(sockfd, (const struct sockaddr *) &server_address, sizeof(struct sockaddr)) == -1)
        {
            fprintf(stderr, "Failed to connect socket to remote host: %s\n", strerror(errno));
            exit(1);
        }
	
        // Copy original terminal settings and modify
         if(tcgetattr(0, &tattr) == -1)
        {
            fprintf(stderr, "Couldn't retrieve termios: %s\n", strerror(errno));
            exit(-1);
        }
        tattr_orig = tattr;
        tattr.c_iflag = ISTRIP;
        tattr.c_oflag = 0;
        tattr.c_lflag = 0;
        // Set to character-at-a-time mode
        if(tcsetattr(0, TCSANOW, &tattr) == -1)
        {
            fprintf(stderr, "Couldn't modify termios: %s\n", strerror(errno));
            exit(-1);
        }
        atexit(terminate);

            struct pollfd pollfds[2];
            watch_fd(&pollfds[0], 0, POLLIN | POLLHUP | POLLERR);
            watch_fd(&pollfds[1], sockfd, POLLIN | POLLHUP | POLLERR);
            while(1)
            {
                if(poll(pollfds, 2, -1) == -1)
                {
                    fprintf(stderr, "Error with polling: %s\n", strerror(errno));
                    exit(1);
                }
                // Case of input from keyboard
                if(pollfds[0].revents & POLLIN)
                {
                    char buf[256];
                    char *p = buf;
                    char buf2[2] = {0x0D, 0x0A};
                    int write_flag;
                    int read_flag = read(0, &buf, 256*sizeof(char));
                    if(read_flag == -1)
                    {
                        fprintf(stderr, "System call read failure: %s\n", strerror(errno));
                        exit(1);
                    }
		    // Output to terminal
		    for(int i = 0; i < read_flag; i++)
                    {
                        if(*p == 0x0D || *p == 0x0A)
                        {
                            write_flag = write(1, &buf2, 2*sizeof(char));
                            if(write_flag < 0)
                            {
                                fprintf(stderr, "System call write failure: %s\n", strerror(errno));
                                exit(1);
                            }
                        }
                        else
                        {
                            write_flag = write(1, p, sizeof(char));
                            if(write_flag == -1)
                            {
                                fprintf(stderr, "System call write failure: %s\n", strerror(errno));
                                exit(1);
                            }
                        }
                        p += 1;
                    }
		    p = buf;
		    // Handle compression of output
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
		    // Write output to log file
		    if(log_flag == 1 && read_flag > 0)
		    {
			if(dprintf(log_fd, "SENT %d bytes: ", read_flag) == -1)
			{
			    fprintf(stderr, "Error with dprintf: %s\n", strerror(errno));
			    exit(1);
			}
			if(write(log_fd, &buf, read_flag) == -1) 
			{
			    fprintf(stderr, "System call write failure: %s\n", strerror(errno));
			    exit(1);
			}
			if(write(log_fd, "\n", 1) == -1)
			{
			    fprintf(stderr, "System call write failure: %s\n", strerror(errno));
			    exit(1);
			}
		    }
		    // Write compressed input to socket
                    for(int i = 0; i < read_flag; i++)
                    {	 
                        if(write(sockfd, p, sizeof(char)) == -1)
			{
                            {
                                fprintf(stderr, "System call write failure: %s\n", strerror(errno));
                                exit(1);
                            }
                        }
                        p += 1;
                    }
                }
                // Case of input from socket
                if(pollfds[1].revents & POLLIN)
                {
                    char buf[2048];
                    char *p = buf;
                    char buf2[2] = {0x0D, 0x0A};
                    int write_flag;
                    int read_flag = read(sockfd, &buf, 256*sizeof(char));
                    if(read_flag == -1)
                    {
                        fprintf(stderr, "System call read failure: %s\n", strerror(errno));
                        exit(1);
                    }
                    // Disconnect if input from socket has stopped		    
                    if(read_flag == 0)
                        break;
		    // Write input to log file
		    if(log_flag == 1 && read_flag > 0)
		    {
			if(dprintf(log_fd, "RECEIVED %d bytes: ", read_flag) == -1)
			{
			    fprintf(stderr, "Error with dprintf: %s\n", strerror(errno));
			    exit(1);
			}
			if(write(log_fd, &buf, read_flag) == -1) 
			{
			    fprintf(stderr, "System call write failure: %s\n", strerror(errno));
			    exit(1);
			}
			if(write(log_fd, "\n", 1) == -1)
			{
			    fprintf(stderr, "System call write failure: %s\n", strerror(errno));
			    exit(1);
			}
		    }
		    // Handle decompression of input
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
		    // Write input to terminal
                    for(int i = 0; i < read_flag; i++)
                    {
                        if(*p == 0x0A)
                        {
                            write_flag = write(1, &buf2, 2*sizeof(char));
                            if(write_flag == -1)
                            {
                                fprintf(stderr, "System call write failure: %s\n", strerror(errno));
                                exit(1);
                            }
                        }
                        else
                        {
                            write_flag = write(1, p, sizeof(char));
                        }
                        p += 1;
                    }
                }
                // Case where either stdin or socket disconnects
                if((pollfds[0].revents & POLLHUP) || (pollfds[1].revents & POLLHUP))
                {
                    char buf[2048];
                    char *p = buf;
                    char buf2[2] = {0x0D, 0x0A};
                    int write_flag;
                    int read_flag;
                    while((read_flag = read(sockfd, &buf, 256*sizeof(char))) != 0)
                    {
                        if(read_flag == -1)
                        {
                            fprintf(stderr, "System call read failure: %s\n", strerror(errno));
                            exit(1);
                        }
		    // Write input to log file
		    if(log_flag == 1 && read_flag > 0)
                    {
                        if(dprintf(log_fd, "RECEIVED %d bytes: ", read_flag) == -1)
                        {
                            fprintf(stderr, "Error with dprintf: %s\n", strerror(errno));
                            exit(1);
                        }
                        if(write(log_fd, &buf, read_flag) == -1)
                        {
                            fprintf(stderr, "System call write failure: %s\n", strerror(errno));
                            exit(1);
                        }
                        if(write(log_fd, "\n", 1) == -1)
                        {
                            fprintf(stderr, "System call write failure: %s\n", strerror(errno));
                            exit(1);
                        }
                    }
			// handle decompression of input
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
			// Write input to terminal
                        for(int i = 0; i < read_flag; i++)
                        {
                            if(*p == 0x0A)
                            {
                                write_flag = write(1, &buf2, 2*sizeof(char));
                                if(write_flag == -1)
                                {
                                    fprintf(stderr, "System call write failure: %s\n", strerror(errno));
                                    exit(1);
                                }
                            }
                            else
                            {
                                write_flag = write(1, p, sizeof(char));
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
                // Case of socket error
                if(pollfds[1].revents & POLLERR)
                {
                    fprintf(stderr, "Poll error in socket: %s\n", strerror(errno));
                    break;
                }

        }
        exit(0);
}

