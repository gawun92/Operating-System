// Name : Gawun Kim
// ID : 305186572

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>


// it allows receiving multiple commands
// in other words, ./lab0 --segfault --input
// is supposed to be not just segfault but showing improper input format
// so the flag is allowing not to terminate(by segfault) the option loop directly
// it is holding the flag during the loop and woking it outside of the loop
int Seg_flag = 0;	


void SegFunc()
{
	char *temp = NULL;
	*temp = 'q';
}


void signal_handle(int sig_num)
{
	fprintf(stderr, "Singal Num : %d\n", sig_num);
	exit(4);
}

// getting optional command line arguments
static struct option const longopts[] = {
    {"input",    required_argument, NULL, 'i'},
    {"output",   required_argument, NULL, 'o'},
    {"segfault", no_argument,       NULL, 's'},
    {"catch",    no_argument,       NULL, 'c'},
    {0, 	0, 		NULL,					0}	// for unrecognized options
};


int main(int argc, char **argv)
{
int check = 0;
int longindex = 0;
int fd0 = -1, fd1 = -1; // For using in input and output 

	while( (check = getopt_long(argc, argv, "i:o:sc", longopts, &longindex)) != -1)
	{
		switch (check)
		{
			// input
			case 'i':	
					fd0 = open(optarg, O_RDONLY);
					if(fd0 >= 0)
					{
						close(0);     // close fd0(open) and it is successful closing -> 0
						int temp = dup(fd0);     // create a copy of the file descriptor oldfd
						if( temp == -1)
						{
							fprintf(stderr,"ERROR happened in duplicating a file descriptor\n");
							exit(2);	// in case of the failure for input process.
						}
						close(fd0);
					}
					// report the failure (on stderr, file descriptor 2) using fprintf(3), 
					// and exit(2) with a return code of 2.
					else	// fb0 is not positive, there is an error in open
					{
						fprintf(stderr, " Error in the process of input file  - Error : %s\n", strerror(2));
						exit(2);  // unable to open input file (failure and exit)
					}
					break;
			// output
			case 'o':
					fd1 = creat(optarg, 0666);  // 0666 is from the spec direction
					if(fd1 >= 0)
					{
						close(1);
						int temp = dup(fd1);
						if( temp == -1)
						{
							fprintf(stderr, "ERROR happened in duplicating a file descriptor\n");
							exit(3);   // in case of the failure for output process.
						}
						close(fd1);
					}
					// the failure (on stderr, file descriptor 2) using fprintf(3), 
					// and exit(2) with a return code of 3.
					else
					{
						fprintf(stderr, " Cannot create the output file  - Error : %s\n", strerror(2));
						exit(3); //with a return code of 3
					}
				break;
			// segfault
			case 's':
					Seg_flag = 1;
					//exit(0);	// I considered and sorted as successful result.  
					break;
			// catch
			case 'c':
					signal(SIGSEGV, signal_handle);  // containing exit(4) in that function
					break;
			default:
					fprintf(stderr, "\nAn unrecognized argument detected\n");
					fprintf(stderr, "******* DIRECTION ***********\n" );
					fprintf(stderr, "  ./lab0 --input=[filename]\n" );
					fprintf(stderr, "  ./lab0 --output=[filename]\n" );
					fprintf(stderr, "  ./lab0 --segfault\n" );
					fprintf(stderr, "  ./lab0 --catch\n" );
					fprintf(stderr, "*****************************\n" );
					exit(1);		// return code 1 -> unrecognized arugment
		}	
	}


	if( Seg_flag == 1 )
		SegFunc();  // it results in segfault  


	size_t nbytes = 1024;		// I just set is 1024 for the size of buffer
	char buffer[nbytes];
	ssize_t nbytes_read;

	while(1)
	{	// read(2)-ing from file descriptor 0 (until encountering an end of file)
		// write(2)-ing to file descriptor 1
		if( (nbytes_read = read(0, &buffer, nbytes)) > 0 )
		{
		    if(write(1, &buffer, nbytes_read) == -1)
		    {
		    	fprintf(stderr,"ERROR happened in the process of writing\n");
		    	exit(-1); // exit(error of write) since it is error from write
		    }  
		}
		else if( nbytes_read == -1 )
		{
			fprintf(stderr,"ERROR happened in the process of reading\n");
			exit(-1);	// exit(error of read) since it is error from read
		}
		else		// if encountering an end of file (nbytes_read = 0)
			break;
	}

  exit(0);	// it means that reading and writing are successful.
  return 0;
}
