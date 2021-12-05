#ifndef G711_ENCODE_H
#define G711_ENCODE_H

#ifdef __cplusplus
extern "C" {
#endif
void g711_encode(const char *bufferPcm, int bufferPcmSize, char **bufferG711, int *bufferG711Size);
void g711_decode(const char *bufferG711, int bufferG711Size, char **bufferPcm, int *bufferPcmSize);
#ifdef __cplusplus
}
#endif

#endif

