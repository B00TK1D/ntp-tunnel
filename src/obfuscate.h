#include <stdlib.h>
#include <string.h>
#include <time.h>

#define OBFUSCIATE_FOOTER_SIZE 3

void obfuscate(char *data, int size);
void deobfuscate(char *data, int size);