/*
NAME: Gawun Kim
EMAIL: gawun92@g.ucla.edu
ID : 305186572
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <poll.h>
#include <time.h>
#include <mraa.h>
#include <getopt.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>

static struct option long_options[] = {
    {"id",      required_argument,  NULL,  'i'},
    {"host",    required_argument,  NULL,  'h'},
    {"period",  required_argument,  NULL,  'p'},
    {"scale",   required_argument,  NULL,  's'},
    {"log",     required_argument,  NULL,  'l'},
    {0,         0,                  NULL,   0 }
};


FILE *log_File = NULL;
char temperature_Flag = 'F';
int socketfd, stop_Flag = 0, first_entry = 0;
long period = 1; 
char *id = "";
char *host = "";
mraa_aio_context sensor;


void usage_handler(){
    fprintf(stderr, "invalid input\n");
    fprintf(stderr, "  --period=#\n");
    fprintf(stderr, "  --scale={C,F}\n");
    fprintf(stderr, "  --log=filename\n");
    exit(1);
}

float getTemperature(int temp){
    float R = 100000.0 * (1023.0 / ((float)temp) - 1.0);
    int B = 4275;
    float C = 1.0 / (log(R / 100000.0) / B + 1 / 298.15) - 273.15;
    if (temperature_Flag == 'F')
        return (C * 1.8) + 32;
    return C;
}



void report_Data(float Temp, int index){
    time_t raw;
    time(&raw);
    struct tm* tm = localtime(&raw);
    if (index == 0){
        dprintf(socketfd, "%.2d:%.2d:%.2d %.1f\n", tm->tm_hour, tm->tm_min, tm->tm_sec, Temp);
        if (stop_Flag == 0 && log_File != NULL) {
            fprintf(log_File, "%.2d:%.2d:%.2d %.1f\n", tm->tm_hour, tm->tm_min, tm->tm_sec, Temp);
        }  
    }
    else{
        dprintf(socketfd, "%.2d:%.2d:%.2d SHUTDOWN\n", tm->tm_hour, tm->tm_min, tm->tm_sec);
        if (log_File != NULL) {
            fprintf(log_File, "%.2d:%.2d:%.2d SHUTDOWN\n", tm->tm_hour, tm->tm_min, tm->tm_sec);
        }
        exit(0);
    }
}

//From Diyu slides
void client_connect(char * host_name, unsigned int port){
    struct sockaddr_in serverAddress;
    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    // AF_INET: IPv4, SOCK_STREAM: TCP connection
    if (socketfd < 0){
        fprintf(stderr, "ERROR[socket]: %s\n", strerror(errno));
        exit(2);
    }
    //Initialize server config
    struct hostent *server = gethostbyname(host_name);
    if (server == NULL){
        fprintf(stderr, "ERROR[get host]: %s\n", strerror(errno));
        exit(2);
    }
    // convert host_name to IP addr
    memset(&serverAddress, 0, sizeof(struct sockaddr_in));
    serverAddress.sin_family = AF_INET; //address is Ipv4
    memcpy((char *)&serverAddress.sin_addr.s_addr, (char *)server->h_addr, server->h_length);
    //copy ip address from server to serv_addr
    serverAddress.sin_port = htons(port); //setup the port
    if (connect(socketfd, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0){
        fprintf(stderr, "ERROR[connection of socket]: %s\n", strerror(errno));
        exit(2);
    }
}

void init_device(){
    mraa_init();
    sensor = mraa_aio_init(1);
    if (sensor==0){
        fprintf(stderr, "ERROR[init sensor]: %s\n", strerror(errno));
        exit(2);
    }
}

void exit_device(){
    mraa_aio_close(sensor);
    if (log_File != NULL && fclose(log_File) != 0){
        fprintf(stderr, "ERROR[closing file]: %s\n", strerror(errno));
        exit(2);
    }
    exit(0);
}



void setupPoll_Time(){
    time_t previousTime, currentTime;
    char buffer[128];
    struct pollfd poll_fd;
    poll_fd.fd = socketfd;
    poll_fd.events = POLLIN;

    previousTime = currentTime = time(NULL);
    while (1){
        currentTime = time(NULL);
        const long long temp_prev = previousTime, temp_curr = currentTime;
        if (stop_Flag == 0 && (temp_curr - temp_prev >= period || first_entry == 0)){
            previousTime = time(NULL);
            float currentTemperature = getTemperature(mraa_aio_read(sensor));
            report_Data(currentTemperature,0);
            if (first_entry == 0)
                first_entry = 1;
        }

        if ( poll(&poll_fd, 1, 0) < 0){
            fprintf(stderr, "ERROR[poll]: %s\n", strerror(errno));
            exit(2);
        }


        if (poll_fd.revents & POLLIN){
            int num = read(socketfd, buffer, 128);
            if (num < 0){
                fprintf(stderr, "ERROR[read]: %s\n", strerror(errno));
                exit(2);
            }

            int i = 0, cmd = 0, period_Flag = 0;
            for (; i < num; i++){
                if (buffer[i] == '\n'){
                    buffer[i] = '\0';
                    
                    if (strcmp(&buffer[cmd], "OFF") == 0){
                        if (log_File != NULL){
                            fprintf(log_File, "OFF\n");
                            fflush(log_File);
                        }
                        report_Data(0,1);
                    }
                    else if (strcmp(&buffer[cmd], "STOP") == 0)
                        stop_Flag = 1;
                    else if (strcmp(&buffer[cmd], "START") == 0)
                        stop_Flag = 0;
                    else if (strcmp(&buffer[cmd], "SCALE=F") == 0)
                        temperature_Flag = 'F';
                    else if (strcmp(&buffer[cmd], "SCALE=C") == 0)
                        temperature_Flag = 'C';
                    else if (strncmp(&buffer[cmd], "PERIOD=", 7) == 0){
                        period = atoi(&buffer[cmd + 7]);
                        if ( period <= 0 ){
                            fprintf(stderr, "ERROR[invalid period format]: %s\n",strerror(errno));
                            exit(2);
                        }
                        if (log_File != NULL){
                            fprintf(log_File, "PERIOD=%lu\n", period);
                            fflush(log_File);
                        }
                        period_Flag = 1;
                    }

                    if (log_File != NULL && period_Flag == 0){
                        fprintf(log_File, "%s\n", &buffer[cmd]);
                        fflush(log_File);
                    }
                    period_Flag = 0;
                    cmd = i + 1;
                }
            }
        }
    }
}



int main(int argc, char **argv){
    if (argc < 2){
        fprintf(stderr, "ERROR : Port number should be specified\n");
        exit(1);
    }
    int option = 0;
    while ((option = getopt_long(argc, argv, "s:p:l:i:h:", long_options, NULL)) != -1){
        switch (option){
        case 's':
            if (strlen(optarg) > 1)
                usage_handler();
            else if (optarg[0] == 'C')
                temperature_Flag = 'C';
            else
                usage_handler();
            break;
        case 'p':
            period = atoi(optarg);
            break;
        case 'l':
            log_File = fopen(optarg, "a+");
            if (log_File == NULL){
                fprintf(stderr, "ERROR[opening the log file]: %s\n", strerror(errno));
                exit(1);
            }
            break;
        case 'i':
            if (strlen(optarg) != 9)
                fprintf(stderr, "ID should be 9 numbers\n");
            else
                id = optarg;
            break;
        case 'h':
            host = optarg;
            break;
        default:
            usage_handler();
        }
    }

    init_device();
    if (atoi(argv[optind]) == 0){
        fprintf(stderr, "ERROR : invalid port num\n");
        exit(1);
    }
    client_connect(host,atoi(argv[optind]));
    dprintf(socketfd, "ID=%s\n", id);
    if (log_File != NULL)
        fprintf(log_File, "ID=%s\n", id);
    
    setupPoll_Time();
    exit_device();
    return 1;
}

