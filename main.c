#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "fuction.h"

void noOptionHelp(const char* o) {
    printf("Undefine Option:%s\nFor more information,try '\033[1m-h\033[0m'\n",o);
}

void noCMDHelp(const char* c) {
    printf("Undefine Command:%s\nFor more information,try '\033[1m-h\033[0m'\n",c);
}

void Help() {
    printf("A command-line for connect between USB and Serial Port\n\n" \
        "\033[1;4mUsage:\033[0m \033[1mfastcom\033[0m [COMMAND] [OPTIONS]\n"    \
        "\n"                                                                    \
        "\033[1;4mCommands:\033[0m\n"                                           \
        "\033[1m list\033[0m\tList all USB devices has connected\n"             \
        "\033[1m open <id>\033[0m\tOpen the port port of USB<id>\n"             \
        "\033[1m help\033[0m\tPrint help\n"                                     \
        "\033[1m version\033[0m\tPrint version\n"                               \
        "\n"                                                                    \
        "\033[1;4mOptions:\033[0m\n"                                            \
        "\033[1m -v,--version\033[0m\tPrint version\n"                          \
        "\033[1m -h,--help\033[0m\tPrint help\n"                                \
        "\033[1m -n <new_name>\033[0m\tName the opened terminal\n"              \
        "\033[1m -b <baud_rate>\033[0m\tSet the baud rate\n"                    \
        "\033[1m -c <port_config>\033[0m\tSet the port config\n");
}

int set_option(char* argv[],int* i,struct Port* port) {
    int index = *i;

    if(strcmp(argv[index],"-v") == 0) {
        //version
        printf("version 2.0.0\n");
        return 1;
    }
    else if(strcmp(argv[index],"-h") == 0) {
        //use help
        Help();
        return 1;
    }
    else if(strcmp(argv[index],"-n") == 0) {
        //named the terminal
        (*i)++;
        if(argv[index + 1] == NULL) {
            printf("A value is required for '\033[31m-n <new name>\033[0m',"\
                    "but none was supplied\n[e.g fastcom open 0 -n terminal1]\n\n"  \
                    "For more information,try '\033[1m-h\033[0m'\n");
        return 0;
        }
        strcpy(port->terminal_name,argv[index+1]);
        return 1;
    }
    else if(strcmp(argv[index],"-b") == 0) {
        //set the baud rate
        (*i)++;
        if(argv[index + 1] == NULL) {
                printf("A value is required for '\033[31m-b <baud rate>\033[0m'," \
                        "but none was supplied\n[e.g fastcom open 0 -b 9600]\n\n"\
                        "For more information,try '\033[1m-h\033[0m'\n");
            return 0;
        }
        port->baud = atoi(argv[index+1]);
        return 1;
    }
    else if(strcmp(argv[index],"-c") == 0) {
        //set port config
        (*i)++;
         if(argv[index + 1] == NULL || strlen(argv[index + 1]) != 3) {
                printf("A value is required for '\033[31m-c <port_config>\033[0m'," \
                        "but none was supplied\n[e.g fastcom open 0 -c N81]\n\n" \
                        "For more information,try '\033[1m-h\033[0m'\n");
            return 0;
        }

        //set parity bit
        if(argv[index+1][0] != 'N' && argv[index+1][0] != 'O' && argv[index+1][0] != 'E') {
            printf("\033[31merror:\033[0mThe parity bit must be:N(NONE),O(ODD),E(EVEN)\n\n");
            return 0;
        }
        port->parity_bit = argv[index+1][0];

        //set bit size
        if(argv[index+1][1] != '8' && argv[index+1][1] != '7' && argv[index+1][1] != '6' && argv[index+1][1] != '5') {
            printf("\033[31merror:\033[0mThe bit size must be:5,6,7,8\n\n");
            return 0;
        }
        port->bitsize = argv[index+1][1];

        //set stop bit
        if(argv[index+1][2] != '1' && argv[index+1][2] != '2') {
            printf("\033[31merror:\033[0mThe stop bit must be:1(1 bit),2(2 bit)\n\n");
            return 0;
        }
        port->stop_bit = argv[index+1][2];

    }
    else {
        noOptionHelp(argv[index]);
        return 0;
    }
}


int main(int argc, char* argv[]) {
    int i;
    int j;

    struct Port port = {
        .usb_id = 0,
        .baud = 115200,
        .bitsize = '8',
        .parity_bit = 'N',
        .stop_bit = '1' 
    };

    for(i = 1; i < argc; i++) {
        if(argv[i][0] == '-') { 
            //this is an option
            if(!set_option(argv,&i,&port)) {
                return 0;
            }
        }
        else {
            //this is a command
            if(strcmp(argv[i],"open") == 0) {
                //open terminal
                i++;
                if(argv[i] == NULL) {
                    printf("A value is required for '\033[31mopen <id>\033[0m'," \
                            "but none was supplied\n[e.g fastcom open 0]\n\n" \
                            "For more information,try '\033[1m-h\033[0m'\n");
                    break;
                }
                port.usb_id = atoi(argv[i]);
                for(j = i + 1; j < argc; j++) {
                    if(!set_option(argv,&j,&port)) {
                        return 0;
                    }
                }
                printf("CONNECT READY");
                open_com(&port);
                break;
            }
            else if(strcmp(argv[i],"list") == 0) {
                //list all USB device
                list_all_devices(NULL);
            }
            else if(strcmp(argv[i],"help") == 0) {
                Help();
                break;
            }
            else if(strcmp(argv[i],"version") == 0) {
                printf("version 2.0.0\n");
                break;
            }
            else {
                noCMDHelp(argv[i]);
                break;
            }
        }
    }
    return 0;
}










