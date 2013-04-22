#include "buffer.h"

void _printBuffer();

field_t buffer[ENC_BUFFER_CHARS];
bool modified;

void buffer_construct() {
    memset(buffer, 0, ENC_BUFFER_CHARS*sizeof(field_t));
    modified = false;
}

void buffer_write(field_t *data, size_t length) {
    unsigned short i;

    if (!modified) {
        for (i = 0; i < length; i++)
            buffer[i] = data[i];

        #ifndef __ENC_NO_PRINTS__
            _printBuffer();
        #endif
    }

    modified = true;
}

void buffer_read(field_t *data, size_t length) {
    unsigned int i;

    for (i = 0; i < length; i++)
        data[i] = buffer[i];

    modified = false;
}

bool buffer_isModified() {
    return modified;
}

void _printBuffer() {
    unsigned short i;

    printf("\n# Buffer\n");
    printf("--------\n\n");

    for (i = 0; i < ENC_BUFFER_CHARS; i++)
        printf("%x", buffer[i]);

    printf("\n\n");
}