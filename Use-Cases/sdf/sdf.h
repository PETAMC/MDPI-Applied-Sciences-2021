/*
 * SDFLib.h
 *
 *  Created on: 01.12.2015
 *      Author: Christof Schlaak
 *      Refactored: Ralf Stemmer on 16.05.2018
 *
 *  CHANGELOG
 *  2.1.0 - 01.01.19 - This lib now works with -O3 optimization
 *  2.2.0 - 16.02.19 - Polling and Ring-Buffer behaviour can be disabled to improve determinism
 */

#ifndef SDFLIB_H_
#define SDFLIB_H_

#include <stdbool.h>
#include <heap.h>

//#define ENABLE_RINGBUFFER
// if defined, the buffer shows ring-buffer behaviour

//#define ENABLE_POLLING

// waiting time for polling: 251cyc measured -> 249cyc real
#ifdef ENABLE_POLLING
#define POLLING_WAIT 20
#define pollingWait() for(volatile int waitCount = 0; waitCount < POLLING_WAIT; waitCount++);
#endif

// type for all tokens
//#define token_t int
typedef int token_t;

typedef struct {
    size_t size;
    unsigned int producerate;
    unsigned int consumerate;
    volatile unsigned int *availabletokens;  // \_ this needs to point to memory accessible by both actors
    unsigned int *firsttokenpos;             // /
    token_t *tokens;    // This is the memory the tokens are stored at
} channel_t;
// availabletokens  is the number of tokens in the buffer when RINGBUFFER is enabled,
//                      otherwise 0 denotes an empty buffer, an !0 a full buffer.
// firsttokenpos    is only needed for ring-buffer mode.
//                      in other mode, the first token is always at position 0

void InitChannel(heap_t *heap, channel_t *channel, size_t size, unsigned int producerate, unsigned int consumerate, unsigned int delay);
void WriteTokens(channel_t *channel, token_t inputtokens[]);
void ReadTokens(channel_t *channel, token_t outputtokens[]);

#endif /* SDFLIB_H_ */

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

