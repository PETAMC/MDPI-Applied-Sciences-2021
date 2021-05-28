/*
 * SDFLib.c
 *
 *  Created on: 01.12.2015
 *      Author: Christof Schlaak
 *      Refactored: Ralf Stemmer on 16.05.2018
 */

#include "sdf.h"


/*
 * Channel Functions
 */

void InitChannel(heap_t *heap, channel_t *channel, size_t size, unsigned int producerate, unsigned int consumerate, unsigned int delay)
{
    if(delay > size)
        delay = size;

    channel->size = size;
    channel->producerate = producerate;
    channel->consumerate = consumerate;

    channel->availabletokens    = (unsigned int*)AllocMemory(heap, sizeof(unsigned int));
    *(channel->availabletokens) = 0;
    channel->firsttokenpos      = (unsigned int*)AllocMemory(heap, sizeof(unsigned int));
    *(channel->firsttokenpos)   = 0;
    channel->tokens             = (token_t*)AllocMemory(heap, sizeof(token_t)*size);

    // init channel with 0
    token_t *ptr = channel->tokens;
    int i;
    for(int i=0; i<size; i++)
    {
        *ptr = 0;
        ptr++;
    }

    *(channel->availabletokens) = delay;
}

void WriteTokens(channel_t *channel, token_t inputtokens[])
{
    // 1.: Check, if there is enough space for the new tokens
    unsigned int usage;
    unsigned int free;
    do
    {
        usage = *(channel->availabletokens);
#ifdef ENABLE_RINGBUFFER
        free  = channel->size - usage;

        if(free >= channel->producerate)
            break;
#else
        if(usage == 0)
            break;
#endif
        
#ifdef ENABLE_POLLING
        pollingWait();
#endif
    }
    while(true);

    // 2.: Calculate the index of the first free place for a new token
    unsigned int index;
    unsigned int firsttokenpos;
#ifdef ENABLE_RINGBUFFER
    firsttokenpos = *(channel->firsttokenpos);
    index         = (firsttokenpos + usage);    // usage is still up-to-date from section 1 of this function
    index         = index % channel->size;
#else
    index         = 0;  // no offset possible when not in ring-buffer-mode
#endif

    // 3.: Write token into channel
    for(unsigned int inputindex = 0; inputindex < channel->producerate; inputindex++)
    {
        token_t *address;
        address   = channel->tokens + index;  // in bytes, it is index*sizeof(token_t) -- this is done by C implicit
        *address  = inputtokens[inputindex];
        index    += 1;
#ifdef ENABLE_RINGBUFFER
        index = index % channel->size;
#endif
    }

    // 4.: Update channel usage
#ifdef ENABLE_RINGBUFFER
    *(channel->availabletokens) += channel->producerate;
#else
    *(channel->availabletokens) = 1;    // in none ring-buffer-mode, only signal that the buffer is full now
#endif
}

void ReadTokens(channel_t *channel, token_t outputtokens[])
{
    // 1.: Check if there are enough tokens in the channel to read
    unsigned int usage;
    do
    {
        usage = *(channel->availabletokens);
#if ENABLE_RINGBUFFER
        if(usage >= channel->consumerate)
            break;
#else
        if(usage != 0)
            break;
#endif

#ifdef ENABLE_POLLING
        pollingWait();
#endif
    }
    while(true);

    // 2.: Get first token position
    unsigned int index;
#ifdef ENABLE_RINGBUFFER
    index = *(channel->firsttokenpos);
#else
    index = 0;
#endif

    // 3.: Read all tokens from channel
    for(int outputindex = 0; outputindex < channel->consumerate; outputindex++)
    {
        token_t *address;
        address   = channel->tokens + index;
        outputtokens[outputindex] = *address;
        index    += 1;
#ifdef ENABLE_RINGBUFFER
        index = index % channel->size;
#endif
    }

    // 4.: Update channel meta data
#ifdef ENABLE_RINGBUFFER
    *(channel->firsttokenpos) = index;
    *(channel->availabletokens) -= channel->consumerate;
#else
    *(channel->availabletokens)  = 0;
#endif
}

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

