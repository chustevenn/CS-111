/* NAME: Steven Chu
 * EMAIL: schu92620@gmail.com
 * ID: 905094800
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

void def_write()
{
    char buf;
    int read_flag;
    while(1)
    {
	read_flag = read(0, &buf, sizeof(char));
	if(read_flag == 0)
	    break; 	
	write(1, &buf, sizeof(char));
    }
}

void handle_segfault()
{
    fprintf(stderr, "Caught segmentation fault, exiting process.\n");
    exit(4);
}   

int main(int argc, char* argv[])
{
    while(1)
    {
	int input_fd;
	int output_fd;
	char* segfault_force;
	int c;
	static struct option long_options[] = {
	    {"input", required_argument, NULL, 1},
	    {"output", required_argument, NULL, 2},
	    {"segfault", no_argument, NULL, 3},
	    {"catch", no_argument, NULL, 4},
	    {0, 0, 0, 0}};
	c = getopt_long(argc, argv, "", long_options, NULL);
	if(c == -1)
	    break;
	switch(c)
	{
	    case 1:
		input_fd = open(optarg, O_RDONLY);
		if(input_fd >= 0)
		{
		    if(dup2(input_fd, 0) == -1)
		    {
			fprintf(stderr, "Error in redirecting input\n");
			exit(-1);
		    }
		    close(input_fd);
		}
		else
		{
		    fprintf(stderr, "Error in --input: Invalid input file, %s\n", strerror(errno));
		    exit(2);
		}
		break;
	    case 2:
		output_fd = creat(optarg, 0666);
		if(output_fd >= 0)
		{
		    if(dup2(output_fd, 1) == -1)
		    {
			fprintf(stderr, "Error in redirecting output\n");
			exit(-1);
		    }
		    close(output_fd);
		}
		else
		{
		    fprintf(stderr, "Error in --output: Could not open output file, %s\n", strerror(errno));
		    exit(3);
		}
		break;
	    case 3:
		segfault_force = NULL;
		*segfault_force = 'z';
		break;
	    case 4:
		signal(SIGSEGV, handle_segfault);
		break;
	    default:
		fprintf(stderr, "Unrecognized argument received\n");
		exit(1);
		break;
	}
    }
    def_write();
    exit(0);
}

		
	
