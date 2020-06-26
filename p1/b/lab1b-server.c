/*
NAME: Gawun Kim
EMAIL: gawun92@g.ucla.edu
ID: 305186572
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <poll.h>
#include <errno.h>
#include <netinet/in.h>
#include <mcrypt.h>
#include <sys/stat.h>
#include <signal.h>

#define CTRL_C          '\x03'
#define CTRL_D          '\x04'
#define CARRIAGE_RETURN '\r'
#define NEWLINE         '\n'
int i;

static struct sockaddr_in serv_addr, cli_addr;
static struct pollfd poll_list[2];
static struct option longopts[] = {
    {"port", required_argument, 0, 'p'},
    {"encrypt", required_argument, 0, 'e'},
    {0,0,0,0}
};
static int Child_Pipe[2];
static int Parent_Pipe[2];

static char buffer[1024];
char temp_buf;
int temp;

int port_Flag = 0;
int encrypt_Flag = 0;

int portNum;

int encrypt_fd;


int listenfd, new_socket_fd;
int pid;


MCRYPT c_td, d_td;
char *en_key;
int key_len;





void server_connect(unsigned int portNum)
{
    unsigned int cli_len = sizeof(struct sockaddr_in); 
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0)
    {
        fprintf(stderr, "ERROR : CREATING SOCKET : %s\n", strerror(errno));
        exit(1);
    }

    memset(&serv_addr, 0, sizeof(struct sockaddr_in));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portNum);

    if (bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        fprintf(stderr, "ERROR : BINDING SOCKET: %s\n", strerror(errno));
        exit(1);
    }

    listen(listenfd, 5);
    new_socket_fd = accept(listenfd, (struct sockaddr *) &cli_addr, &cli_len);
    if(new_socket_fd < 0){
        fprintf(stderr, "ERROR in the process of accept\n");
        exit(1);
    }
}


void signal_handler(int sig_Num)
{
    if(sig_Num == SIGINT)
    {
        temp = kill(pid, SIGINT);
        if(temp < 0)
        {
            fprintf(stderr, "ERROR in the process of kill\n");
            exit(1);
        }
    }
    if (sig_Num == SIGPIPE)
    {
        fprintf(stderr, " SIGPIPE has been received\n");
        exit(0);
    }
}

void encrypt(char* buffer, int crpt_len)
{
    if (mcrypt_generic(c_td, buffer, crpt_len) != 0)
    {
        fprintf(stderr, "ERROR : the main encryption function\n");
        exit(1);
    }
}

void decrypt(char* buffer, int decrpt_len)
{

    if (mdecrypt_generic(c_td, buffer, decrpt_len) != 0)
    {
        fprintf(stderr, "ERROR : the main decryption function\n");
        exit(1);
    }
}


void setting_Encrypt(char* key, int len)
{
    c_td = mcrypt_module_open("blowfish", NULL, "ofb", NULL);
    if (c_td == MCRYPT_FAILED)
    {
        fprintf(stderr, "ERROR in the process of encriptor module\n");
        exit(1);
    }
    temp = mcrypt_generic_init(c_td, key, len, NULL);
    if (temp < 0)
    {
        fprintf(stderr, "ERRPR in the process of initializing MCRYPT\n");
        exit(1);
    }

    d_td = mcrypt_module_open("blowfish", NULL, "ofb", NULL);
    if (d_td == MCRYPT_FAILED)
    {
        fprintf(stderr, "ERROR in the process of descriptor module\n");
        exit(1);
    }
    temp = mcrypt_generic_init(d_td, key, len, NULL);
    if (temp < 0)
    {
        fprintf(stderr, "ERRPR in the process of initializing MCRYPT\n");
        exit(1);
    }
}



void FnExit()
{
   int status;
    temp = waitpid(pid, &status, 0);
    if(temp < 0){
        fprintf(stderr, "ERROR in waitpid\n");
        exit(1);
    }
    else if (WIFEXITED(status))
    {
        int SIGNAL = WTERMSIG(status);
        int STATUS = WEXITSTATUS(status);
        fprintf(stderr, "SHELL EXIT SIGNAL=%d, STATUS=%d\n", SIGNAL, STATUS);
        close(listenfd);
        close(new_socket_fd);
        exit(0);
    }
}


void buff_dealing(int bytes_read, int socketfd)
{

    for ( i = 0; i < bytes_read; ++i)
    {
        temp_buf = buffer[i];
        switch (temp_buf)
        {
            case (CARRIAGE_RETURN):
            case (NEWLINE):
                if(socketfd == Child_Pipe[1]){
                    temp = write(socketfd, "\n", sizeof(char));
                    if(temp < 0){
                       fprintf(stderr, "ERROR in the process of write to STDOUT\n");
                        exit(1); 
                    }
                }
                else{
                    temp = write(socketfd, &temp_buf, sizeof(char));
                    if(temp < 0){
                       fprintf(stderr, "ERROR in the process of write to STDOUT\n");
                        exit(1); 
                    }
                }
                
                
                break;
            case (CTRL_D):
                if (socketfd == Child_Pipe[1]){
                    close(Child_Pipe[1]);
                }
                else{
                    temp = write(socketfd, &temp_buf, sizeof(char));
                    if(temp < 0){
                        fprintf(stderr, "ERROR in the process of write to STDOUT\n");
                        exit(1); 
                    }
                }
                break;
            case (CTRL_C):
                if (socketfd == Child_Pipe[1]){
                    temp = kill(pid, SIGINT);
                    if(temp < 0){
                        fprintf(stderr, "ERROR in the process of kill\n");
                        exit(1); 
                    }
                }
                else{
                    temp = write(socketfd, &temp_buf, sizeof(char));
                    if(temp < 0){
                        fprintf(stderr, "ERROR in the process of write to STDOUT\n");
                        exit(1); 
                    }
                }
                break;
            default:
                temp = write(socketfd, &temp_buf, sizeof(char));
                if(temp < 0){
                    fprintf(stderr, "ERROR in the process of write to STDOUT\n");
                    exit(1); 
                }
                break;
        }
    }
}

void Shell_Copy()
{

    poll_list[0].fd = new_socket_fd;
    poll_list[1].fd = Parent_Pipe[0]; //read from shell
    poll_list[0].events = poll_list[1].events = POLLIN | POLLHUP | POLLERR;

    if (encrypt_Flag == 1)
        setting_Encrypt(en_key, key_len);

    while (1)
    {
        int retval = poll(poll_list, 2, 0);
        int bytes_read;

        if (retval < 0)
        {
            fprintf(stderr, "ERROR in the process of poll\n");
            exit(1);
        }

        if (poll_list[0].revents & POLLIN)
        {
            bytes_read = read(new_socket_fd, buffer, 1024);
            if(bytes_read < 0)
            {
                fprintf(stderr, "ERROR in the process of reading from keyboard\n");
                exit(1);
            }
            if (encrypt_Flag == 1)
            {
                for ( i = 0; i < bytes_read; i++)
                    decrypt(&buffer[i], 1);
            }
            buff_dealing(bytes_read, Child_Pipe[1]);
        }
        if (poll_list[1].revents & POLLIN)
        {
            bytes_read = read(Parent_Pipe[0], buffer, 1024);
            if(bytes_read < 0){
                fprintf(stderr, "ERROR in the process of reading from keyboard\n");
                exit(1); 
            }

            if (encrypt_Flag == 1)
            {
                for ( i = 0; i < bytes_read; i++)
                    encrypt(&buffer[i], 1);
            }

            buff_dealing(bytes_read, new_socket_fd);
        }

        if ((poll_list[0].revents | poll_list[1].revents) & (POLLHUP | POLLERR))
        {
            fprintf(stderr, "The server has been shut down!\n");
            exit(1);
        }
    }
}


void Forking()
{

    signal(SIGPIPE, signal_handler);
    signal(SIGINT, signal_handler);
    atexit(FnExit);

    
    temp = pipe(Child_Pipe);
    if(temp < 0)
    {
        fprintf(stderr, "ERROR in the process to create Child pipe\n");
        exit(1);
    }
    temp = pipe(Parent_Pipe);
    if(temp < 0)
    {
        fprintf(stderr, "ERROR in the process to create Parent pipe\n");
        exit(1);
    }
    pid = fork();

    if(pid < 0)
    {
        fprintf(stderr, "ERROR in the process of fork\n");
        exit(1);
    }

    if(pid == 0)
    {
        temp = close(Child_Pipe[1]);
        if (temp < 0){
            fprintf(stderr, "ERROR in close for unused child pipe\n");
            exit(1);
        }
        temp = close(Parent_Pipe[0]);  
        if (temp < 0){
            fprintf(stderr, "ERROR in close for unused parent pipe\n");
            exit(1);
        } 
        temp = dup2(Child_Pipe[0], STDIN_FILENO);
        if (temp < 0){
            fprintf(stderr, "ERROR in duplicate for STDIN\n");
            exit(1);
        } 
        temp = dup2(Parent_Pipe[1], STDOUT_FILENO);
        if (temp < 0){
            fprintf(stderr, "ERROR in duplicate for STDOUT\n");
            exit(1);
        } 
        temp = dup2(Parent_Pipe[1], STDERR_FILENO);
        if (temp < 0){
            fprintf(stderr, "ERROR in duplicate for STDERROR\n");
            exit(1);
        } 
        temp = close(Child_Pipe[0]);
        if (temp < 0){
            fprintf(stderr, "ERROR in close for  child pipe\n");
            exit(1);
        }
        temp = close(Parent_Pipe[1]);
        if (temp < 0){
            fprintf(stderr, "ERROR in close for  parent pipe\n");
            exit(1);
        } 
        char path[] = "/bin/bash";
        char* arg[2];
        arg[0] = path; 
        arg[1] = NULL;
        int c = execvp(path, arg);
        if(c == -1)
        {
            fprintf(stderr, "ERROR : %s", strerror(errno));
            exit(1);
        }

    }
    else
    {
        temp = close(Child_Pipe[0]);
        if( temp < 0){
            fprintf(stderr, "ERROR in close for  child pipe\n");
            exit(1); 
        }
        temp = close(Parent_Pipe[1]);
        if( temp < 0){
            fprintf(stderr, "ERROR in close for  parent pipe\n");
            exit(1); 
        }
        Shell_Copy();
    }
}





int main(int argc, char **argv)
{

    int option = 1;


    while ((option = getopt_long(argc, argv, "p:e", longopts, NULL)) != -1)
    {
        switch (option)
        {
        case 'p':
            port_Flag = 1;
            portNum = atoi(optarg);
            break;
        case 'e':
            encrypt_Flag = 1;
            encrypt_fd = open(optarg, O_RDONLY);
            if (encrypt_fd < 0)
            {
                fprintf(stderr, "ERROR in the process of encrypt\n");
                exit(1);
            }
            struct stat keyStat;
            temp = fstat(encrypt_fd, &keyStat);
            if (temp < 0)
            {
                fprintf(stderr, "ERROR : fstat for encrpt\n");
                exit(1);
            }
            if ((en_key = malloc(keyStat.st_size * sizeof(char))) == NULL)
            {
                fprintf(stderr, "ERROR : Memory allocation for encrypt");
                exit(1);
            }
            key_len = read(encrypt_fd, en_key, keyStat.st_size * sizeof(char));
            if (key_len < 0)
            {
                fprintf(stderr, "ERROR in the process of read for encrypt\n");
                exit(1);
            }
            break;
        default:
            fprintf(stderr, "ERROR: invalid argument\n");
            fprintf(stderr, "---------USAGE---------\n");
            fprintf(stderr, "./lab1b --port=portnumber\n");
            fprintf(stderr, "        --log=filename\n");
            fprintf(stderr, "        --encrypt=my.key\n");
            exit(1);
        }
    }

    if (port_Flag != 1)
    {
                fprintf(stderr, "The port_number should be contained always\n");
        fprintf(stderr, "---------USAGE---------\n");
        fprintf(stderr, "./lab1b --port=portnumber\n");
        fprintf(stderr, "        --log=filename\n");
        fprintf(stderr, "        --encrypt=my.key\n");
        exit(1);
    }


    server_connect(portNum);
    Forking();
    /* Deinit the encryption thread, and unload the module */
    /* deinitialize the encryption thread */
    if (encrypt_Flag == 1)
    {
        free(en_key);                   // in line 377 use dynamic allocated so make it free        
        mcrypt_generic_deinit (c_td);   // deinitializeing encrypt
        mcrypt_generic_deinit (d_td);   // deinitializeing decrypt
        mcrypt_generic_end(c_td);       //ending    encrypt
        mcrypt_generic_end(d_td);       // ending   decrypt
    }
    exit(0);
    return 1;
}
