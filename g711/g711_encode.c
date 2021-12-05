#include "g711_encode.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "g711_table.h"

char *allocate_buffer(long buffer_size)
{
    char *buffer;

    /* grab sufficient memory for the 
    buffer to hold the audio */
    buffer = (char*)calloc((unsigned long)buffer_size, sizeof(char));
    /* memory error */
    if(buffer == NULL)
    {
        perror("Error while allocating memory for write buffer.\n");
        exit(EXIT_FAILURE);
    }

    return buffer;
}

void g711_encode(const char *bufferPcm, int bufferPcmSize, char **bufferG711, int *bufferG711Size)
{
    pcm16_ulaw_tableinit();
    *bufferG711Size = bufferPcmSize / 2;
    *bufferG711 = allocate_buffer(*bufferG711Size);
    pcm16_to_ulaw(bufferPcmSize, bufferPcm, *bufferG711);
}

void g711_decode(const char *bufferG711, int bufferG711Size, char **bufferPcm, int *bufferPcmSize)
{
    ulaw_pcm16_tableinit();
    *bufferPcmSize = bufferG711Size * 2;
    *bufferPcm = allocate_buffer(*bufferPcmSize * 5);
    ulaw_to_pcm16(bufferG711Size, bufferG711, *bufferPcm);
}
