#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <dirent.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <ncurses.h>
#include <pthread.h>
#include "fuction.h"

#define CONFIG_RIGHT_SPACE 20

struct _RXTX_STRUCT {
    int* fd;
    WINDOW* win[2];
    pthread_mutex_t* mutex;
};

void list_all_devices(devices_l* dl) {
    int i = 0;
    struct dirent* entry;
    DIR *dir = opendir("/dev");

    if (dir == NULL) {
        printf("Error:Can't open /dev");
    }
    
    while ((entry = readdir(dir)) != NULL) {
        if(strstr(entry->d_name,"ttyUSB") != NULL && entry-> d_type == DT_CHR) {
            if(dl == NULL) {
                printf("device\033[1m#%d\033[0m\t\033[41m/dev/%s\033[0m\n",i,entry->d_name);
            }
            else {
                strcpy((*dl)[i],entry->d_name);
            }
            i++;
        }
    }
    if(i == 0) {
        printf("\033[1;31mNO USB DEVICES CONNECT\033[0m\n\n");
    }

    closedir(dir);
}

int open_port_file(int id,devices_l* dl) {
    char device[24];
    int fd;
    
    list_all_devices(dl);
    sprintf(device,"/dev/%s",(*dl)[id]);
    fd = open(device,O_RDWR | O_NOCTTY);

    return fd;
}

void close_port_file(int fd) {
    close(fd);
}

speed_t baud_select(int b) {
    switch (b) {
        case 9600:
            return B9600;
        case 19200:
            return B19200;
        case 38400:
            return B38400;
        case 57600:
            return B57600;
        case 115200:
            return B115200;
        case 230400:
            return B230400;
        case 460800:
            return B460800;
        case 500000:
            return B500000;
        case 576000:
            return B576000;
        case 921600:
            return B921600;
        case 1000000:
            return B1000000;
        case 1152000:
            return B1152000;
        case 1500000:
            return B1500000;
        case 2000000:
            return B2000000;
        default:
            return B0;
    }
}

int config_port(int fd,struct Port* port) {
    struct termios options;
    speed_t baud;

    tcgetattr(fd,&options);
    
    //set baud
    baud = baud_select(port->baud);
    cfsetispeed(&options,baud);
    cfsetospeed(&options,baud);

    //set config
    options.c_cflag &= ~CSIZE;
    switch (port->bitsize) {
        case '5':
            options.c_cflag |= CS5;
            break;
        case '6':
            options.c_cflag |= CS6;
            break;
        case '7':
            options.c_cflag |= CS7;
            break;
        default:
            options.c_cflag |= CS8;
            break;
    }
    switch (port->parity_bit) {
        case 'O':
            options.c_cflag |= PARENB;
            options.c_cflag |= PARODD;
            break;
        case 'E':
            options.c_cflag |= PARENB;
            break;
        default:
            options.c_cflag &= ~PARENB;
            options.c_cflag &= ~PARODD;
            break;
    }
    switch (port->stop_bit) {
        case '2':
            options.c_cflag |= CSTOPB;
            break;
        default:
            options.c_cflag &= ~CSTOPB;
            break;
    }
    
    cfmakeraw(&options);
    options.c_cflag |= (CLOCAL | CREAD);

    tcflush(fd,TCIFLUSH);
    tcsetattr(fd,TCSANOW,&options);

    return 0;
}



void* send_thread(void *arg) {
    char input[1024];
    int fd = *(((struct _RXTX_STRUCT*)arg)->fd);
    WINDOW* read_win = ((struct _RXTX_STRUCT*)arg)->win[0];
    WINDOW* send_win = ((struct _RXTX_STRUCT*)arg)->win[1];
    pthread_mutex_t* read_mutex = ((struct _RXTX_STRUCT*)arg)->mutex;

    wprintw(send_win,"SEND>>>");
    wrefresh(send_win);
    for(;;) {
        wscanw(send_win,"%[^\n]",input);
        wrefresh(send_win);
        int len = write(fd,input,strlen(input));
        if (len < 0) {
           perror("ERROR:Failed to write"); 
        }
        else {
            pthread_mutex_lock(read_mutex);
            wprintw(read_win,"[SEND] %s\n",input);
            wrefresh(read_win);
            pthread_mutex_unlock(read_mutex);
        }
    }

    return NULL;
}

void* read_thread(void *arg) {
    char buffer[1024];
    int fd = *(((struct _RXTX_STRUCT*)arg)->fd);
    WINDOW* show_win = ((struct _RXTX_STRUCT*)arg)->win[0];
    pthread_mutex_t* read_mutex = ((struct _RXTX_STRUCT*)arg)->mutex;

    for(;;) {
        memset(buffer, 0, sizeof(buffer));
        int len = read(fd,buffer,sizeof(buffer) - 1);
        if (len > 0) {
            buffer[len] = '\0';
            pthread_mutex_lock(read_mutex);
            wprintw(show_win,"[RECEIVE] ");
            for(int i = 0; i < len; i++) {
                unsigned char c = buffer[i];
                if (isprint(c)) {
                    wprintw(show_win,"%c",c);
                }
                else if (c == '\n'){
                    wprintw(show_win,"\n");
                }
                else if (c == '\t') {
                    wprintw(show_win,"\t");
                }
                else {
                    wprintw(show_win,"<%02X>",c);
                }
            }
            wrefresh(show_win);
            pthread_mutex_unlock(read_mutex);
        }
        else if (len == 0) {
            continue;
        }
        else {
            perror("ERROR:Failed to read");
        }
        usleep(100000);
    }

    return NULL;
}

void show_config(struct Port* port,const char* d,int scrW) {
    const char* _parity[3] = {"NONE","ODD","EVENT"};
    int _parity_i;
    switch (port->parity_bit) {
        case 'O':_parity_i = 1;break;
        case 'E':_parity_i = 2;break;
        default:_parity_i = 0;
    }
    move(1,scrW - CONFIG_RIGHT_SPACE);
    attron(A_BOLD | A_UNDERLINE);
    printw("CONFIG\n");
    attroff(A_BOLD | A_UNDERLINE);
    move(2,scrW - CONFIG_RIGHT_SPACE);
    printw("DEVICE:\t%s\n",d);
    move(3,scrW - CONFIG_RIGHT_SPACE);
    printw("BAUD:\t%d\n",port->baud);
    move(4,scrW - CONFIG_RIGHT_SPACE);
    printw("SIZE:\t%c bit\n",port->bitsize);
    move(5,scrW - CONFIG_RIGHT_SPACE);
    printw("PARITY:\t%s\n",_parity[_parity_i]);
    move(6,scrW - CONFIG_RIGHT_SPACE);
    printw("STOP:\t%c bit\n",port->stop_bit);
    refresh();
}

int open_com(struct Port* port) {

    devices_l dl;
    int fd = open_port_file(port->usb_id,&dl);
    int scrH,scrW;

    if (fd == -1) {
        perror("ERROR:Failed to open port port");
        return -1;
    }

    config_port(fd,port);

    WINDOW* stdscr = initscr();
    echo();
    cbreak();
    getmaxyx(stdscr,scrH,scrW);
    show_config(port,dl[port->usb_id],scrW); 
    
    WINDOW* read_win = newwin(scrH - 1, scrW - CONFIG_RIGHT_SPACE - 4, 0, 0);
    scrollok(read_win,TRUE);
    WINDOW* send_win = newwin(1,scrW,scrH,0);

    pthread_mutex_t read_win_mutex = PTHREAD_MUTEX_INITIALIZER;
    struct _RXTX_STRUCT _struct = {
        .fd = &fd,
        .win = {read_win,send_win},
        .mutex = &read_win_mutex
    };

    pthread_t read_p,send_p;
    pthread_create(&read_p,NULL,read_thread,&_struct);
    pthread_create(&send_p,NULL,send_thread,&_struct);

    pthread_join(read_p,NULL);
    pthread_join(send_p,NULL);

    delwin(read_win);
    delwin(send_win);
    endwin();

    close_port_file(fd);
    
    return 0;
}






