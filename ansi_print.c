#include "ansi_print.h"
#include <ctype.h>

#define NORMAL 0
#define ANSICMD 1
#define NUMBER 2

void ansi_print(char *str, int len, struct ansi_struct *st_cb) {
    int i,cmd_index;
    int j;
    int sta = NORMAL;
    for (i = 0,cmd_index = 0; i < len && i < MAX_CONNECT && cmd_index < MAX_CMD; i++) {
        if (*str == '\033' && *(str++) == '[') {
            sta = ANSICMD;  // into ansi mod
        }
        
        if (sta == ANSICMD) {
            if (isdigit(*str)) {
                sta = NUMBER;
            }
            if (isalpha(*str)) {
                sta = NORMAL;  //into normal mod
            }
        }
        


        str++;
    }

}
