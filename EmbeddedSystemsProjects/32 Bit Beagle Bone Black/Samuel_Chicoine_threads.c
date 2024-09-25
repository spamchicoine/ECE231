#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/epoll.h>

int datavalues = 0;
char buffer[10];
int bsize  = 0;
int pindex = 0;
int gindex = 0;

struct timespec start, end;
pthread_mutex_t lock;


void* readtime(void* input){
    int gpio_number = 67;
    configure_interrupt(gpio_number); // enable interrupt
        // the following code can be used to receive interrupts on the registered pin
    char InterruptPath[40];
    sprintf(InterruptPath, "/sys/class/gpio/gpio%d/value",gpio_number);
    int epfd;
    struct epoll_event events, ev;
        // (STEP 1) open the interrupt file
        // file pointer (C abstraction to manipulate files)
    FILE* fp = fopen(InterruptPath, "r");
        // file descriptor (Unix\Linux file identifier used by system calls)
    int fd = fileno(fp);
        // (STEP 2) create epoll instance to monitor I/O events oninterrupt file
    epfd = epoll_create(1);
        // (STEP 3) register events that will be monitored
        // detects whenever a new data is available for read (EPOLLIN)
        // signals the read events when the available read valuehas changed (EPOLLET)
    ev.events = EPOLLIN | EPOLLET;
        // (STEP 4) register interrupt file with epoll interface for monitoring
    ev.data.fd = fd;

    epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);
    int interrupt;
    time_t seconds;

    while (datavalues < 100){
        
        // Begin critical section lock thread
        pthread_mutex_lock(&lock);

        // Check if buffer is full
        while (bsize == 9){
            pthread_mutex_unlock(&lock);
            pthread_mutex_lock(&lock);
            pindex = 0;
        }
        
        // Listen for event
        interrupt = epoll_wait(epfd, &events, 1, -1);
        clock_gettime(CLOCK_MONOTONIC, &start);

        interrupt = epoll_wait(epfd, &events, 1, -1);
        clock_gettime(CLOCK_MONOTONIC, &end);

        // Write time to buffer
        buffer[pindex] = (start.tv_sec - end.tv_sec);
        
        // Move to next index is buffer, increment buffer size by 1
        pindex++; bsize++;

        // Unlock thread
        pthread_mutex_unlock(&lock);
    }

    close(epfd);
    return NULL;
    
}

void* writetime(void* input){

    float mean = 0

    FILE *fptr;
    fptr = fopen("timefile.txt","w");

    while (datavalues < 100){

        pthread_mutex_lock(&lock);

        while (bsize == 0){
            pthread_mutex_unlock(&lock);
            pthread_mutex_lock(&lock);
        }
        for (int i=0,i<10){
            mean = mean + buffer[gindex];
            gindex ++;
        }
        mean = mean/10;

        fprintf(fptr,"%ld,\n", mean);

        datavalues++; mean = 0; gindex = 0;

        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

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
// sets up an interrupt on the given GPIO pin
void configure_interrupt(int gpio_number){
    configure_gpio_input(gpio_number); // set gpio as input
        // configuring interrupts on rising, falling or both edges
    char InterruptEdge[40];
    sprintf(InterruptEdge, "/sys/class/gpio/gpio%d/edge", gpio_number);
        // configures interrupt on falling edge
    FILE* fp = fopen(InterruptEdge, "w");
    fwrite("rising", sizeof(char), 7, fp);
        // configures interrupt on both edges
        // fwrite("both", sizeof(char), 4, fp);
    fclose(fp);
}

int main() {
    int data = 6;
    pthread_t thread_id[2]; // instantiating argument required for thread creation

    // initialize lock
    if (pthread_mutex_init(&lock, NULL) != 0) {
    printf("\n mutex init has failed\n");
    return 1;
    }
    
    pthread_create(&(thread_id[0]), NULL, readtime, (void*)(&data) );
    pthread_create(&(thread_id[1]), NULL, writetime, (void*)(&data) );

    pthread_join(thread_id[0], NULL);
    pthread_join(thread_id[1], NULL);

    pthread_mutex_destroy(&lock);
    pthread_exit(0);

    return 0;
}