#ifndef _ANSI_PRINT_H_
#define _ANSI_PRINT_H_

#define MAX_CMD 16
#define MAX_CONNECT 256

struct ansi_struct {
    int ansi_cmd[MAX_CMD];
    char connect[MAX_CONNECT];
};

void ansi_print(char* str,int len,struct ansi_struct* st_cb);

#endif //_ANSI_PRINT_H_
