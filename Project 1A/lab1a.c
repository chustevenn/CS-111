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

struct termios tattr_orig;

void terminate()
{
    const struct termios tattr_original = tattr_orig;
    if(tcsetattr(0, TCSANOW, &tattr_original) == -1)
    {
	fprintf(stderr, "Could not return termios to default: %s\n", strerror(errno));
	exit(-1);	
    }
}

void watch_fd(struct pollfd *pollfds, int target, short events)
{
    pollfds->fd = target;
    pollfds->events = events;
}

void sigpipe_handler()
{
    write(2, "Signal SIGPIPE received.\n", 17); 
    exit(0);
}

int main(int argc, char* argv[])
{
    int pipefd[2];
    int pipefd1[2];
    int pid;
    struct termios tattr;
    while(1)
    {
	int c;
	// Construct argument options to support only option --shell
	static struct option long_options[] = {
	    {"shell", required_argument, NULL, 1},
	    {0, 0, 0, 0}};
	c = getopt_long(argc, argv, "", long_options, NULL);
	if(c == -1)
	    break;
	switch(c)
	{
	    case 1:
		// Creating two copies of termios
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
		// parent process
		else
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
		    watch_fd(&pollfds[0], 0, POLLIN | POLLHUP | POLLERR);
		    watch_fd(&pollfds[1], pipefd1[0], POLLIN | POLLHUP | POLLERR);
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
			    for(int i = 0; i < read_flag; i++)
			    {
			    	if(*p == 0x0D || *p == 0x0A)
            		    	{
                		    write_flag = write(1, &buf2, 2*sizeof(char));
                		    if(write_flag < 0 || write(pipefd[1], "\n", 1) == -1)
                		    {
                    		        fprintf(stderr, "System call write failure: %s\n", strerror(errno));
                    		        exit(1);
                		    }		    
			    	}
				else if(*p == 0x04)
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
				else
				{
				    write_flag = write(1, p, sizeof(char));
				    if(write_flag == -1 || write(pipefd[1], p, sizeof(char)) == -1)
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
			    char buf2[2] = {0x0D, 0x0A};
			    int write_flag;
			    int read_flag = read(pipefd1[0], &buf, 256*sizeof(char));
			    if(read_flag == -1)
                            {
                                fprintf(stderr, "System call read failure: %s\n", strerror(errno));
                                exit(1);
                            }
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
			// Case where either stdin or pipe 2 disconnects
			if((pollfds[0].revents & POLLHUP) || (pollfds[1].revents & POLLHUP))
			{
			    char buf[256];
                            char *p = buf;
                            char buf2[2] = {0x0D, 0x0A};
                            int write_flag;
			    int read_flag;
                            while((read_flag = read(pipefd1[0], &buf, 256*sizeof(char))) != 0)
			    {
                                if(read_flag == -1)
                                {
                                    fprintf(stderr, "System call read failure: %s\n", strerror(errno));
                                    exit(1);
                                }
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
			// Case of pipe 1 error
			if(pollfds[1].revents & POLLERR)
			{
			    fprintf(stderr, "Poll error in read pipe: %s\n", strerror(errno));
			    break;
			}
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
		break;
	    default:
		fprintf(stderr, "Unrecognized argument received.\n");
		exit(1);
	}
    }
    // Create two copies of the termios struct, one to hold original, one to hold modified   
    if(tcgetattr(0, &tattr) == -1)
    {
	 fprintf(stderr, "Couldn't retrieve termios: %s\n", strerror(errno));
	 exit(-1);
    }
    tattr_orig = tattr;
    // Modify termios struct
    tattr.c_iflag = ISTRIP;
    tattr.c_oflag = 0;
    tattr.c_lflag = 0;
    if(tcsetattr(0, TCSANOW, &tattr) == -1)
    {
	fprintf(stderr, "Couldn't modify termios: %s\n", strerror(errno));
	exit(-1);
    }
    // Continuously process input until escape sequence is detected
    while(1)
    {
	int read_bytes;
	int write_flag;
	char buf[256];
	char *p = buf;
	char buf2[2] = {0x0D, 0x0A};
	// Large read of 256 bytes
	read_bytes = read(0, &buf, 256*sizeof(char));
	if(read_bytes < 0)
	{
	    fprintf(stderr, "System call read failure: %s\n", strerror(errno));
	    exit(1);
	}
	// Character by character processing
	for(int i = 0; i < read_bytes; i++)
	{
	    // Carriage return and linefeed mapping
	    if(*p == 0x0D || *p == 0x0A)
	    {
	        write_flag = write(1, &buf2, 2*sizeof(char));
	    	if(write_flag < 0)
		{
		    fprintf(stderr, "System call write failure: %s\n", strerror(errno));
		    exit(1);
		}
	    }
	    // EOF handling
	    else if(*p == 0x04)
	    {
		terminate();
		exit(0);
	    }
	    else
	    {
		write_flag = write(1, p, sizeof(char));
		if(write_flag < 0)
                {
                    fprintf(stderr, "System call write failure: %s\n", strerror(errno));
                    exit(1);
                }
	    }
	    p += 1;
	}
    }

}
