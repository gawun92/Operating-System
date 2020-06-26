/* 
NAME: Gawun Kim
EMAIL: gawun92@g.ucla.edu
ID : 305186572
*/
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <math.h>
#include <mraa.h>
#include <poll.h>
#include <errno.h>
 


static struct option long_options[] = {
   {"period", required_argument, NULL, 'p'},
   {"scale", required_argument,  NULL, 's'},
   {"log", required_argument,    NULL, 'l'},
   {0,     0,                    NULL,  0 }
};
FILE *log_File = NULL;
int period = 1;           // initial period
int temperature_Flag = 0; // 0 is F    1 is C
int stop_Flag = 0;
time_t prev_time, curr_time;
mraa_aio_context sensor;
mraa_gpio_context button;
char buffer[256];

void usage_handler()
{
    fprintf(stderr, "invalid input\n");
    fprintf(stderr, "  --period=#\n");
    fprintf(stderr, "  --scale={C,F}\n");
    fprintf(stderr, "  --log=filename\n");
    exit(1);
}


void report_Data(float Temp, int index) {
    time_t raw;
    time(&raw);
    struct tm* tm = localtime(&raw);
    if (index == 0){
        fprintf(stdout, "%.2d:%.2d:%.2d %.1f\n", tm->tm_hour, tm->tm_min, tm->tm_sec, Temp);
        if (stop_Flag == 0 && log_File != NULL) {
            fprintf(log_File, "%.2d:%.2d:%.2d %.1f\n", tm->tm_hour, tm->tm_min, tm->tm_sec, Temp);
        }  
    }else{
        fprintf(stdout, "%.2d:%.2d:%.2d SHUTDOWN\n", tm->tm_hour, tm->tm_min, tm->tm_sec);
        if (log_File != NULL) {
            fprintf(log_File, "%.2d:%.2d:%.2d SHUTDOWN\n", tm->tm_hour, tm->tm_min, tm->tm_sec);
        }
        exit(0);
    }
    
}

void init_device(){
    sensor = mraa_aio_init(1);
    if(!sensor){
        fprintf(stderr, "ERROR[initialize sensor]: %s\n", strerror(errno));
        exit(1);
    }
    button = mraa_gpio_init(60);
    mraa_gpio_dir(button, MRAA_GPIO_IN);
    if(!button){
        fprintf(stderr, "ERROR[initialize button]: %s\n",strerror(errno));
        exit(1);
    }
    
}

// From Diyu slides
float getTemperature(int temp)
{
    float R = 100000.0 * (1023.0 / ((float)temp) - 1.0);
    int B = 4275;
    float C = 1.0 / (log(R / 100000.0) / B + 1 / 298.15) - 273.15;
    if (temperature_Flag == 0)
        return (C * 1.8) + 32;
    return C;
}

void exit_device(){
    mraa_aio_close(sensor);
    mraa_gpio_close(button);
    if (fclose(log_File) != 0){
        fprintf(stderr, "ERROR[close log file]: %s\n", strerror(errno));
        exit(1);
    }
    exit(0);
}

void setupPollandTime(){
    struct pollfd poll_fd;
    poll_fd.fd = 0;
    poll_fd.events = POLLIN;

    char temp_buf[256];
    memset(buffer, 0, 256);
    memset(temp_buf, 0, 256);
    int i, cp_index = 0;
    
    while (1){
        float tempValue = getTemperature(mraa_aio_read(sensor));
        if(!stop_Flag){
            report_Data(tempValue, 0);
        }
        time(&prev_time);
        time(&curr_time); 
        while((curr_time-prev_time) < period){
            if(mraa_gpio_read(button)){
                report_Data(0, 1);
            }
            if(poll(&poll_fd, 1, 0) < 0){
                fprintf(stderr, "ERROR[poll]: %s\n", strerror(errno));
                exit(1);
            }
            if(poll_fd.revents & POLLIN){
                int num = read(STDIN_FILENO, buffer, 256);
                if(num < 0){
                    fprintf(stderr, "ERROR[read from stdin]: %s\n", strerror(errno));
                    exit(1);
                }
                
                for(i = 0; i < num; ++i){
                    if (buffer[i] =='\n'){
                        if(strncmp(temp_buf, "PERIOD=", 7) == 0){
                            int newPeriod = atoi(&temp_buf[7]);
                            period = newPeriod;
                            if(stop_Flag == 0 && log_File != NULL){
                                fprintf(log_File, "PERIOD=%d\n", period);
                            }
                        }
                        else if(strcmp(temp_buf, "SCALE=F") == 0){
                            temperature_Flag = 0;
                            if(stop_Flag == 0 && log_File != NULL){
                                fprintf(log_File, "SCALE=F\n");
                            }
                        }
                        else if(strcmp(temp_buf, "SCALE=C") == 0){
                            temperature_Flag = 1;
                            if(stop_Flag == 0 && log_File != NULL){
                                fprintf(log_File, "SCALE=C\n");
                            }
                        }
                        else if((strncmp(temp_buf, "LOG", 3) == 0)){
                            if(log_File != NULL){
                                fprintf(log_File, "%s\n", temp_buf);
                            }
                        }
                        else if(strcmp(temp_buf, "OFF") == 0){
                            if(log_File != NULL){
                                fprintf(log_File, "OFF\n");
                            }
                            report_Data(0, 1);
                        }
                        else if(strcmp(temp_buf, "STOP") == 0){
                            stop_Flag = 1;
                            if(log_File != NULL){
                                fprintf(log_File, "STOP\n");
                            }
                        }
                        else if(strcmp(temp_buf, "START") == 0){
                            stop_Flag = 0;
                            if(log_File != NULL){
                                fprintf(log_File, "START\n");
                            }
                        }
                        else {
                            fprintf(stdout, "Command not recognized\n");
                            exit(1);
                        }
                        cp_index = 0;
                        memset(temp_buf, 0, 256);  
                        
                    }
                    else {
                        temp_buf[cp_index] = buffer[i];
                        cp_index++;
                    }
                    time(&curr_time);
                }
                
            }
            
        }
    }
}
                
int main(int argc, char **argv){
    int option = 0;
    
    while((option = getopt_long(argc, argv, "p:s:l", long_options, NULL)) != -1){
        switch(option){
            case 'p':
                period = atoi(optarg);
                if(period < 0){
                    fprintf(stderr, "ERROR[Incorrect value]: %s\n", strerror(errno));
                    exit(1);
                }
                break;
            case 's':
                if (strlen(optarg) != 1)
                    usage_handler();
                else{
                    if (optarg[0] == 'C'){
                        temperature_Flag = 1;
                    }
                    else if (optarg[0] == 'F'){
                        temperature_Flag = 0;
                    }
                    else{
                        usage_handler();
                    }
                }
                break;
        case 'l':
            log_File = fopen(optarg, "a+");  
            if (log_File == NULL){
                fprintf(stderr, "ERROR[opening the log file]: %s\n", strerror(errno));
                exit(1);
            }
            break;
        default:
            usage_handler();
            break;
        }
    }
    init_device();
    setupPollandTime();
    exit_device();
    return 1;
}
