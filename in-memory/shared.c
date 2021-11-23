#include <stdio.h>

void __attribute__ ((constructor)) alert_init(void);

void alert_init(void) {
    fprintf(stderr,"[+] Module was loaded correctly\n");
}
