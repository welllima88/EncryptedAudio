#ifndef __ENC_PROTOCOL_H__
#define __ENC_PROTOCOL_H__

#include <stdint.h>

#include <stdio.h>
#include <stdlib.h>

#include "bigdigits.h"

typedef DIGIT_T digit_t;
typedef uint8_t field_t;

// Key sizes
#define ENC_KEY_BITS          1248
#define ENC_KEY_CHARS         156
#define ENC_KEY_DIGITS        39

// Packet Sizes
#define ENC_KEY_PACKET_CHARS  313
#define ENC_DATA_PACKET_CHARS 1500

// Diffie-Hellman Secrets Size
#define ENC_DH_SECRET_DIGITS  32

// Hash Sizes
#define ENC_HASH_CHARS        20
#define ENC_HASH_DIGITS       5

#define NO_ALLOCS

void senderHello(field_t *sendPacket, int *senderSecret);
void receiverHello(field_t *sendPacket, field_t *receivedPacket, int *receiverSecret, unsigned char *receiverModulus, unsigned char *receiverPrivateExp);
void senderAcknowledge(field_t *sendPacket, field_t *receivedPacket, int *senderSecret, unsigned char *senderModulus, unsigned char *senderPrivateExp);

void sendData(field_t *sendPacket);

#endif