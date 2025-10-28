#define _SIZE_DEVICES_ 16


struct Port {
    char terminal_name[16];
    int baud;
    int usb_id;
    char bitsize;
    char parity_bit;
    char stop_bit;
};

typedef char devices_l[_SIZE_DEVICES_][_SIZE_DEVICES_];

void list_all_devices(devices_l* dl);
int open_com(struct Port* port);
