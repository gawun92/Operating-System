/*
NAME: Gawun Kim
EMAIL: gawun92@g.ucla.edu
ID: 305186572
*/
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <stdlib.h>
#include <poll.h>
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <sys/stat.h>
#include <mcrypt.h>

int i;

#define CTRL_C          '\x03'
#define CTRL_D          '\x04'
#define CARRIAGE_RETURN '\r'
#define NEWLINE         '\n'

static struct option longopts[] = {
    {"port", required_argument, 0, 'p'},
    {"log", required_argument, 0, 'l'},
    {"encrypt", required_argument, 0, 'e'},
    {0,0,0,0}
};

static char crlf[2] = {CARRIAGE_RETURN, NEWLINE};

static struct pollfd poll_list[2];
static struct sockaddr_in serv_addr;


static struct termios defult_Attrib;
static struct termios new_Attrib;

static struct hostent *server;

static char temp_buf;
static char buffer[1024];


int port_Num;

int log_fd;
int encrypt_fd;
int socketfd;

int encrypt_Flag = 0;
int port_Flag = 0; 
int log_Flag = 0;

int temp;


MCRYPT c_td, d_td;
char *en_key;




// from TA slide
void client_connect(char* host_name, int port_Num)
{
    socketfd = socket(AF_INET, SOCK_STREAM, 0); // Create socket
    if (socketfd < 0)
    {
        fprintf(stderr, "ERROR : CREATING SOCKET : %s\n", strerror(errno));
        exit(1);
    }

    server = gethostbyname(host_name); // Get host IP address etc.
    if (server == NULL)
    {
        fprintf(stderr, "ERROR in the process of host_name\n");
        exit(1);
    }
    memset(&serv_addr, 0, sizeof(serv_addr));                
    serv_addr.sin_family = AF_INET;      
    serv_addr.sin_port = htons(port_Num);
    memcpy( &serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);

    if (connect(socketfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    {
        fprintf(stderr, "ERROR : CONNECTING SOCKET : %s\n", strerror(errno));
        exit(1);
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




void input_Mode()
{

    tcgetattr(0, &new_Attrib);
    new_Attrib.c_iflag = ISTRIP;    /* only lower 7 bits    */
    new_Attrib.c_oflag = 0;         /* no processing        */
    new_Attrib.c_lflag = 0;         /* no processing        */
    int checker = tcsetattr(0, TCSANOW, &new_Attrib);
    if(checker < 0)
    {
        fprintf(stderr, "ERROR in the process of the new attributes.\n");
        exit(1);
    }
}


void reset()
{
    int temp = tcsetattr(STDIN_FILENO, TCSANOW, &defult_Attrib);
    if(temp < 0) 
    {
        fprintf(stderr, "Could not set the attributes\n");
        exit(1);
    }
}


void save_Attrrib()
{
    int result = tcgetattr(0, &defult_Attrib);
    if(result < 0)
    {
        fprintf(stderr, "ERROR in the process of the attributes.\n");
        exit(1);
    }
    atexit(reset);
}


void buff_dealing(int bytes_read, int socketfd)
{
    for (i = 0; i < bytes_read; i++)
    {
        temp_buf = buffer[i];
        switch (temp_buf)
        {
        case (CARRIAGE_RETURN):
        case (NEWLINE):
            if (socketfd == STDOUT_FILENO){
                temp = write(socketfd, &crlf, 2*sizeof(char));
                if(temp < 0)
                {
                    fprintf(stderr, "ERROR in the process of write to STDOUT\n");
                    exit(1);
                }
            }
            else{
                temp = write(socketfd, &temp_buf, sizeof(char));
                if(temp < 0)
                {
                    fprintf(stderr, "ERROR in the process of write to STDOUT\n");
                    exit(1);
                } 
            }
            break;
        case (CTRL_D):
            temp = write(socketfd, &temp_buf, sizeof(char));
            if(temp < 0)
            {
                fprintf(stderr, "ERROR in the process of write to STDOUT\n");
                exit(1);
            }
            break;
        case (CTRL_C):
            temp = write(socketfd, &temp_buf, sizeof(char));
            if(temp < 0)
            {
                fprintf(stderr, "ERROR in the process of write to STDOUT\n");
                exit(1);
            }
            break;
        default:
            temp = write(socketfd, &temp_buf, sizeof(char));
            if(temp < 0)
            {
                fprintf(stderr, "ERROR in the process of write to STDOUT\n");
                exit(1);
            }
            break;
        }
    }
}


void Shell_Copy()
{
    int bytes_read;
    poll_list[0].fd = STDIN_FILENO;
    poll_list[1].fd = socketfd;
    poll_list[0].events = poll_list[1].events = POLLIN | POLLHUP | POLLERR;

    while (1)
    {
        bytes_read = 0;
        if ( poll(poll_list, 2, 0) < 0 )
        {
            fprintf(stderr, "Polling error: %s\n", strerror(errno));
            exit(1);
        }
        if (poll_list[0].revents & POLLIN)
        {

            //Data has been sent in from keyboard, need to read it
            bytes_read = read(STDIN_FILENO, buffer, 1024);
            if (bytes_read < 0)
            {
                fprintf(stderr, "ERROR in the process of read\n");
                exit(1);
            }

            //Data in from actual keyboard, must be echoed to screen
            buff_dealing(bytes_read, STDOUT_FILENO);

            if (encrypt_Flag == 1)
            {
                for (i = 0; i < bytes_read; i++)
                    encrypt(&buffer[i], 1);
            }
                

            if (log_Flag > 0)
            {
                dprintf(log_fd, "SENT %d bytes: ", bytes_read);
                buff_dealing(bytes_read, log_fd);
                dprintf(log_fd, &crlf[1], sizeof(char) );
            }
            buff_dealing(bytes_read, socketfd);
        }

        if (poll_list[1].revents & POLLIN)
        {
            bytes_read = read(socketfd, buffer, 1024);
            if (bytes_read < 0)
            {
                fprintf(stderr, "ERROR in the process of read\n");
                exit(1);
            }
            if (bytes_read == 0)
            {
                temp = close(socketfd);
                if(temp < 0)
                {
                    fprintf(stderr, "ERROR in the process of close socket\n");
                    exit(1);
                }
                exit(0);
            }

            if (log_Flag > 0)
            {
                dprintf(log_fd, "RECEIVED %d bytes: ", bytes_read);
                buff_dealing(bytes_read, log_fd);
                dprintf(log_fd, "\n");
            }

            if (encrypt_Flag == 1)
            {
                for (i = 0; i < bytes_read; i++)
                    decrypt(&buffer[i], 1);
            }
            buff_dealing(bytes_read, STDOUT_FILENO);
        }

        if ((poll_list[0].revents | poll_list[1].revents) & (POLLHUP | POLLERR))
        {
            fprintf(stderr, "The server has been shut down!\n");
            exit(1);
        }
    }
    return;
}

int main(int argc, char **argv)
{
    int key_len;
    int option;
    int checker = isatty(STDIN_FILENO);
    if( checker == 0)
    {
        fprintf(stderr, "ERROR to test whether a file descriptor refers to a terminal.\n");
        exit(1);
    }
    while ((option = getopt_long(argc, argv, "p:e:l", longopts, NULL)) != -1)
    {
        switch (option)
        {
        case 'p':
            port_Flag = 1;
            port_Num = atoi(optarg);
            break;
        case 'l':
            log_Flag = 1;
            log_fd = creat(optarg, S_IRWXU);
            if (log_fd < 0)
            {
                fprintf(stderr, "ERROR in the process of log\n");
                exit(1);
            }
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



    save_Attrrib();
    input_Mode();
    client_connect("localhost", port_Num);

    if (encrypt_Flag == 1)
        setting_Encrypt(en_key, key_len);


    Shell_Copy();
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
