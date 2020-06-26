// NAME: Gawun Kim
// EMAIL: gawun92@g.ucla.edu
// ID: 305186572

#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <string.h>
#include <poll.h>
#include <signal.h>
#include <sys/wait.h>

#define CTRL_C   		'\x03'
#define CTRL_D    		'\x04'
#define CARRIAGE_RETURN '\r'
#define NEWLINE   		'\n'

static char crlf[2] = {CARRIAGE_RETURN, NEWLINE};

// flag = 0  initial value.
// flag = 1  when inserting "./lab1a"
// flag = 2  when inserting "./lab1a --shell=command"
static int flag = 0;
// pipie
static int Child_Pipe[2];   
static int Parent_Pipe[2];

static struct termios termios_p;
static struct option longopts[] = {
	{"shell",required_argument,NULL,'s'},
	{0,      0,                NULL,0}
};
static pid_t pid;
extern int errno;



void fnExit()
{
	tcsetattr(STDIN_FILENO, TCSANOW, &termios_p);
	if (flag) 
	{
		int stat_val;
		if (waitpid(pid, &stat_val, 0) == -1) 
		{
			fprintf(stderr, "ERROR in waitpid");
			exit(1);
		}
		else if (WIFEXITED(stat_val)) 
		{
			int SIGNAL = WTERMSIG(stat_val);
			int STATUS = WEXITSTATUS(stat_val);
			fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\n", SIGNAL, STATUS);
			exit(0);
		}
	}
	else
	{
		fprintf(stderr, "ERROR in the process of flag");
		exit(1);
	}
}


// This is for "./lab1a"
//
// closing not needed pipes and  duplicating needed pipes for now
// after that closing  remains
// closing --> close()    dupicliate --> dup2()
void run_CHILD()
{
	// closing for not needed pipes
	close(Child_Pipe[1]);    close(Parent_Pipe[0]);
	// duplicate for needed pipes
	dup2(Child_Pipe[0], 0);  dup2(Parent_Pipe[1], 1);
	// closing for remain pipes
	close(Child_Pipe[0]);    close(Parent_Pipe[1]);
	
	char path[] = "/bin/bash";
	char *arg[2];
	arg[0] = path;
	arg[1] = NULL;			
	int c = execvp(path, arg);
	if (c == -1)
	{ // executing it by bash
		fprintf(stderr, "ERROR : %s", strerror(errno));
		exit(1);
	}
}

// This is for "./lab1a --shell=temp"
void run_CHILD2(char temp[])		
{
	close(Child_Pipe[1]);
    close(Parent_Pipe[0]);
	dup2(Child_Pipe[0], 0);
	dup2(Parent_Pipe[1], 1);
	close(Child_Pipe[0]);
	close(Parent_Pipe[1]);
	
	int c = execlp("/bin/bash", "/bin/bash", "-c", temp, (char*) NULL);
	if ( c == -1)
	{
		fprintf(stderr, "ERROR : %s", strerror(errno));
		exit(1);
	}
}

	
void Shell_Copy() 
{
	
	struct pollfd poll_list[2];
	poll_list[0].fd = 0; 				// value by keyboard
	poll_list[1].fd = Parent_Pipe[0]; 	// Set output with parent
	poll_list[0].events = poll_list[1].events = POLLIN | POLLHUP | POLLERR;

 	int retval;
    char buffer[256];
    char temp_buf;  

    while (1) 
    {
        retval = poll(poll_list, (unsigned long) 2, -1);
        if (retval < 0) 
        {
            fprintf(stderr, "ERROR in the process of poll\n");
            exit(1);
        }

        if ( poll_list[0].revents & POLLIN )
        {
        	int bytes_read = read(0, buffer, sizeof(char)*256);
            if ( bytes_read < 0 )
            {
            	fprintf(stderr, "ERRPR in the process of reading from keyboard\n");
            	exit(1);
            } 
    		for ( int i=0; i<bytes_read; ++i ){	
    			temp_buf = buffer[i];
				switch (temp_buf)
				{
					case (CTRL_D):
						close(Child_Pipe[1]);
						break;
					case (CTRL_C):
						kill(pid, SIGINT);
						break;
					case (NEWLINE):
					case (CARRIAGE_RETURN):
						write(1, &crlf, 2*sizeof(char));
						char c = NEWLINE;
						write(Child_Pipe[1], &c, sizeof(char));
						continue;
					default:
						write(1, &temp_buf, sizeof(char));
						write(Child_Pipe[1], &buffer, sizeof(char));
				}
	        }
	        memset(buffer, 0, bytes_read);
        	if ( bytes_read < 0 )
        	{
            	fprintf(stderr, "ERROR in the process to read from keyboard\n");
            	exit(1);
            } 
        }

        //read from shell pollfd
        if ( poll_list[1].revents & POLLIN ) 
        {
        	int bytes_read = read(Parent_Pipe[0], buffer, sizeof(char)*256);
            if (bytes_read < 0)
            {
            	fprintf(stderr, "ERROR in the process to read from shell\n");
            	exit(1);
            } 

    		for ( int i=0; i<bytes_read; ++i )
    		{
	            temp_buf = buffer[i];
				switch (temp_buf)
				{
					case (NEWLINE):
						write(1, &crlf, 2*sizeof(char));
						continue;
					default:
						write(1, &temp_buf, sizeof(char));
				}
            }
            memset(buffer, 0, bytes_read);
        }

    	if ((POLLERR | POLLHUP) & (poll_list[1].revents)) 
    	{
    	    exit(0);
    	}
    }
}


void copy()
{
	char buffer[256];
	int bytes_read = read(0,buffer, sizeof(char)*256);
	while (bytes_read)
	{
		for (int i=0 ;i<bytes_read;i++)
		{
			switch (buffer[i]) 
			{
				case (CTRL_D):
					exit(0);
					break;
				case (CTRL_C):
				case (NEWLINE):
					write(1, &crlf, 2*sizeof(char));
					continue;
				default:
					write(1, &buffer[i] , sizeof(char));
			}
		}
		//clear buffer 
		memset(buffer, 0, bytes_read);
		bytes_read = read(0, buffer, sizeof(char)*256);
	}
}

int main(int argc, char **argv) {
	//define CL option 'shell'
	
	//var to store index of command that getopt finds

	// only for  "./lab1a"
    if (argc == 1)
    {
    	flag = 1;
    }
    // for "./lab1a --shell="cmd" or other not valid commands
    else if (argc == 2) 
    {
		int opt_index;
		int c;
		while (1)
		{
			c = getopt_long(argc, argv, "", longopts, &opt_index);
			if (c == 's')
			{
				flag = 2;
				break;
			}
			else
			{
				fprintf(stderr, "error: invalid argument\n");
				exit(1);
			}
		}
    }
    else{
    	fprintf(stderr, "error: invalid argument\n");
		exit(1);
    }
	
	//set terminal input mode
	tcgetattr(0, &termios_p);
	atexit(fnExit);
	
	//get working copy of attr
	struct termios attr;
	tcgetattr(0, &attr); 

	attr.c_iflag = ISTRIP;	    /* only lower 7 bits	*/
	attr.c_oflag = 0;			/* no processing		*/
	attr.c_lflag = 0;			/* no processing		*/
	
	int check = tcsetattr(0, TCSANOW, &attr);
	if (check < 0)
	{
		fprintf(stderr, "ERROR in the process of termios");
		exit(1);
	}

	int C_check = pipe(Child_Pipe);
	int P_check = pipe(Parent_Pipe);

	if(C_check < 0 || P_check < 0)
	{
		fprintf(stderr, "ERROR in creating pipes: child and parent");
		exit(1);
	}


/***************************** code about flag below ****************************/
	if (flag == 1)
	{
		pid = fork(); //fork new process
		if (pid < -1)
		{
			fprintf(stderr, "ERROR : %s", strerror(errno));
			exit(1);
		}
		else if (pid == 0)
		{	//child process will redirect to shell
			run_CHILD();
		}
		else
		{	//parent process will do writing
			// Parent I/O
			close(Child_Pipe[0]);
			close(Parent_Pipe[1]);
			Shell_Copy();
		}
		copy();
		exit(0);
	} 
	else if (flag == 2)
	{
		pid = fork(); //fork new process
		if (pid < -1){
			fprintf(stderr, "ERROR : %s", strerror(errno));
			exit(1);
		}
		else if (pid == 0){ //child process will redirect to shell
			run_CHILD2(optarg);
		}
		else{
			// Parent I/O
			close(Child_Pipe[0]);
			close(Parent_Pipe[1]);
			Shell_Copy();
		}
		copy();
		exit(0);
	}
	else
	{
		fprintf(stderr, "ERROR in flag");
		exit(1);
	}
	
	
	return 1;
}
