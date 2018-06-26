#ifndef COMM_H
#define COMM_H

#define COMM_BUFFER_MAX 1024
#define COMM_TIMEOUT 60

#ifdef __cplusplus
extern "C" {
#endif

// Configuration functions
void commInit();
void commSetRate(int baud);

// Read/write functions
int commBytesPending();
int commReadBytes(u_char* buffer, int bytes);
void commWriteBytes(u_char* buffer, int bytes);
void commWriteBytesAsm(u_char* buffer, int bytes);

// Asynchronous read functions
void commReadBytesAsync(u_char* buffer, int bytes);
int commAsyncDone();
int commAsyncReceived();
void commEndAsync();

#ifdef __cplusplus
}
#endif

#endif /* COMM_H */

