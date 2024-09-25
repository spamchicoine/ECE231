#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/epoll.h>
#include <signal.h>
#include <string.h>

const char AIN_DEV[] = "/sys/bus/iio/devices/iio:device0/in_voltage1_raw";

struct timespec stamp;

void configure_gpio_input(int gpio_number){
    // converting gpio number from integer to string
    char gpio_num[10];
    sprintf(gpio_num, "export%d", gpio_number);
    const char* GPIOExport="/sys/class/gpio/export";
    // exporting the GPIO to user space
    FILE* fp = fopen(GPIOExport, "w");
    fwrite(gpio_num, sizeof(char), sizeof(gpio_num), fp);
    fclose(fp);
    // setting gpio direction as input
    char GPIODirection[40];
    sprintf(GPIODirection, "/sys/class/gpio/gpio%d/direction", gpio_number);
    // setting GPIO as input
    fp = fopen(GPIODirection, "w");
    fwrite("in", sizeof(char), 2, fp);
    fclose(fp);
}

void event_handler_0(){
    char pin_number[32] = "P8_13";
    char pwmchip[32] = "pwmchip7";
    char channel[32] = "1";
    char period[32] = "1000000";
    char duty_cycle[32] = "500000";


    printf("PWM Frequency 1kHz.\n");
    start_pwm(pin_number, pwmchip, channel, period, duty_cycle); // start 
            
    sleep(1);
    stop_pwm(pin_number, pwmchip, channel);
}

void event_handler_1(){
    char pin_number[32] = "P8_13";
    char pwmchip[32] = "pwmchip7";
    char channel[32] = "1";
    char period[32] = "100000";
    char duty_cycle[32] = "50000";


    printf("PWM Frequency 10kHz.\n");
    start_pwm(pin_number, pwmchip, channel, period, duty_cycle); // start 
            
    sleep(1);
    stop_pwm(pin_number, pwmchip, channel);
}

void event_callback(void handler()){
    handler();
}

double CtoF(double c){
    return (c * 9.0 / 5.0) + 32.0;
}

double temperature(char *string){
    int value = atoi(string);
    // Convert the digital value to millivolts.
    double millivolts = (value / 4096.0) * 1800;
    // Convert the millivolts to Celsius temperature.
    double temperature = (millivolts - 500.0) / 10.0;
    return temperature;
}

void* gettemp(void* input){
    long tmic;
    int fd = open(AIN_DEV, O_RDONLY);
    while (1){
    char buffer[1024];
        // Read the temperature sensor data
        int ret = read(fd, buffer, sizeof(buffer));
        if (ret != -1){
            buffer[ret] = '\0';
            double celsius = temperature(buffer);
            double fahrenheit = CtoF(celsius);
            clock_gettime(CLOCK_MONOTONIC, &stamp);
            tmic = stamp.tv_sec*1000;
            printf("digital value: %s celsius: %f fahrenheit: %f time: %ld\n", buffer, celsius,fahrenheit,tmic);
            lseek(fd, 0, 0);
        }
        sleep(1);
    }
    close(fd);
    
    pthread_exit(NULL);
}

void* playspeaker(void* input){
    // configure pin P8_8 as input with internal pull-up enabled [Button 0]
    int gpio_number_0 = 67;
    configure_gpio_input(gpio_number_0);
    // file path to read button status
    char valuePath_0[40];
    sprintf(valuePath_0, "/sys/class/gpio/gpio%d/value", gpio_number_0);
    // configure pin P8_9 as input with internal pull-up enabled [Button 1]
    int gpio_number_1 = 69;
    configure_gpio_input(gpio_number_1);
    // file path to read button status
    char valuePath_1[40];
    sprintf(valuePath_1, "/sys/class/gpio/gpio%d/value", gpio_number_1);
    // wait before first readings to avoid faulty readings
    sleep(1);
    int state[2];
    FILE *fp_0, *fp_1;
    // loop to monitor events
    char pin_number[32] = "P8_13";
    char pwmchip[32] = "pwmchip7";
    char channel[32] = "1";

    while(1){
        fp_0 = fopen(valuePath_0, "r");
        fp_1 = fopen(valuePath_1, "r");
        // default state is 1 since buttons are configured with
        // internal pull-up resistors. So, reading 0 means button
        // is pressed
        fscanf(fp_0, "%d", &(state[0]));
        fscanf(fp_1, "%d", &(state[1]));
        fclose(fp_0);
        fclose(fp_1);
        // event 0 detected
        if( state[0] == 0 ){
            stop_pwm(pin_number, pwmchip, channel);

            event_callback(&event_handler_0);
        }
        // event 1 detected
        if( state[1] == 0 ){

            stop_pwm(pin_number, pwmchip, channel); // Make sure the pwm pin and channel are cleared first

            event_callback(&event_handler_1);
        }
    }
    pthread_exit(NULL);
}

void config_pin(char* pin_number, char* mode){
    // creates environment to execute shell command
    if(!vfork()){
        // execute shell command for pin configuration
        int ret = execl("/usr/bin/config-pin", "config-pin", pin_number, mode, NULL);
        if (ret < 0){
            printf("Failed to configure pin in PWM mode.\n");
            exit(-1);
        }
    }
}

void set_pwm_duty_cycle(char* pwmchip, char* channel, char* duty_cycle){
    // export file path
    char PWMDutyCycle[60];
    sprintf(PWMDutyCycle, "/sys/class/pwm/%s/pwm-7:%s/duty_cycle", pwmchip, channel);
    // configure PWM device
    FILE* fp = fopen(PWMDutyCycle, "w");
    fwrite(duty_cycle, sizeof(char), strlen(duty_cycle), fp);
    fclose(fp);
}

void set_pwm_period(char* pwmchip, char* channel, char* period){
    long duty_cycle_int, period_int;
    // before setting up the period read old duty cycle
    char PWMDutyCycle[60], duty_cycle_str[20];
    sprintf(PWMDutyCycle, "/sys/class/pwm/%s/pwm-7:%s/duty_cycle", pwmchip, channel);
    FILE* fp = fopen(PWMDutyCycle, "r");
    fscanf(fp, "%ld", &duty_cycle_int);
    fclose(fp);
    period_int = atol(period);
    // If the old duty_cycle value is greater than the new period
    // update the dummy_duty_cycle first to avoid errors with setting up
    // the period
    if( duty_cycle_int >= period_int){
        duty_cycle_int = period_int/2;
        // converting long to char data type
        sprintf(duty_cycle_str, "%ld", duty_cycle_int);
        // setup dummy duty cycle
        set_pwm_duty_cycle(pwmchip, channel, duty_cycle_str);
    }
    // export file path
    char PWMPeriod[60];
    sprintf(PWMPeriod, "/sys/class/pwm/%s/pwm-7:%s/period", pwmchip, channel);
    fp = fopen(PWMPeriod, "w");
    fwrite(period, sizeof(char), strlen(period), fp);
    fclose(fp);
}

void stop_pwm(char* pin_number, char* pwmchip, char* channel){
    char PWMDisable[40];
    sprintf(PWMDisable, "/sys/class/pwm/%s/pwm-7:%s/enable", pwmchip, channel);
    // stop generating PWM
    FILE* fp = fopen(PWMDisable, "w");
    fwrite("0", sizeof(char), 1, fp);
    fclose(fp);
}

void start_pwm(char* pin_number, char* pwmchip, char* channel, char* period, char* duty_cycle){
    /* Input:
    pin_number: pin_number to generate PWM on
    pwmchip: the device folder to generate PWM
    channel: pwm device channel
    perod: pwm period
    duty_cycle: pwm duty cyle
    */
    // configure the pin in PWM mode
    config_pin(pin_number, "pwm");
    // export PWM device
    FILE* fp;
    char PWMExport[40];
    sprintf(PWMExport, "/sys/class/pwm/%s/export", pwmchip);
    fp = fopen(PWMExport, "w");
    fwrite(channel, sizeof(char), sizeof(channel), fp);
    fclose(fp);
    // configure PWM Period
    set_pwm_period(pwmchip, channel, period);
    // configure PWM Duty Cycle
    set_pwm_duty_cycle(pwmchip, channel, duty_cycle);
    // enable PWM
    char PWMEnable[40];
    sprintf(PWMEnable, "/sys/class/pwm/%s/pwm-7:%s/enable", pwmchip, channel);
    // configure generating PWM
    fp = fopen(PWMEnable, "w");
    fwrite("1", sizeof(char), 1, fp);
    fclose(fp);
}

int main(){
    pthread_t thread_id[2];

    pthread_create(&(thread_id[0]), NULL, gettemp, NULL );
    pthread_create(&(thread_id[1]), NULL, playspeaker, NULL );

    pthread_join(thread_id[0], NULL);
    pthread_join(thread_id[1], NULL);

    return 0;
}