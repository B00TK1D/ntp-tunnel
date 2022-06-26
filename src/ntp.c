#include "ntp.h"
#include <time.h>

char* ntp_time_content() {
    char content[NTP_CONTENT_SIZE];
    time_t t = time(NULL);
    content[NTP_CONTENT_SIZE - (sizeof(time_t))] = time(NULL);
    return content;
}