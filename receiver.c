#include "channel.h"
#include "receiver.h"
#include "protocol.h"

int receiver_checkHmac(field_t *dataPacket);

// RSA
const unsigned char Enc_ReceiverPrivateExp[ENC_PRIVATE_KEY_CHARS] =
    "\x3c\x13\xf0\x04\xd1\x24\xdd\x01\x03\xd2\xb0\x71\x42\xa2\xf6"
    "\x10\xc6\x35\x96\x9c\xd4\x4c\x46\xd9\xaf\x91\xbe\xcc\xad\xc9"
    "\xc5\x51\xdb\x4d\x5b\x81\xd9\x89\x13\xd4\xc4\xb0\x30\x30\xf7"
    "\x00\xe6\xc5\x50\x5d\x10\xa4\x9f\xf4\xec\xc0\x73\xd8\xfd\x36"
    "\x9e\x60\xe1\x42\x4f\x73\xae\xf7\x28\x0d\x15\xcd\x92\xb0\x0d"
    "\x5f\x04\xeb\xf9\xd0\xc0\xe7\x38\xff\x9e\xc3\x2c\xab\x22\xcd"
    "\x4b\xda\xcb\xc7\xbd\x24\xd4\x51\x71\xe5\x99\x01\xcb\xcb\x78"
    "\xc0\xbd\x3f\xc2\xc5\xd9\x02\x46\x68\xa9\xbc\xfd\x11\x3f\xc5"
    "\xdc\xc3\x6a\x55\xad\x3f\x5f\xfb\x08\xad\x01\x5a\xac\x02\xbc"
    "\xa1\xaa\xb7\x88\x01\x2b\xbd\x60\x97\x41\xbf\x5b\x6e\x06\x55"
    "\xf9\x03\xb5\xd1\xd3\xc1";

// Memory Pointers
digit_t *receiverSecret;
digit_t *senderModExp;

uint8_t *receiverAESKey;
uint8_t *receiverHashKey;
uint8_t *receiverCTRNonce;

uint32_t *receiverPacketCounter;

void receiver_construct() {
    receiverSecret = calloc(ENC_PRIVATE_KEY_DIGITS, sizeof(digit_t));
    senderModExp = calloc(ENC_PRIVATE_KEY_DIGITS, sizeof(digit_t));

	receiverAESKey = calloc(ENC_AES_KEY_CHARS, sizeof(uint8_t));
    receiverHashKey = calloc(ENC_HMAC_KEY_CHARS, sizeof(uint8_t));
	receiverCTRNonce = calloc(ENC_CTR_NONCE_CHARS, sizeof(uint8_t));

    receiverPacketCounter = calloc(1, sizeof(uint32_t));
}

int receiver_receiverHello() {
    int returnStatus;

    field_t receivedPacket[ENC_KEY_PACKET_CHARS];
    field_t sendPacket[ENC_KEY_PACKET_CHARS];

    channel_read(receivedPacket, ENC_KEY_PACKET_CHARS);

    printf("--> receiver_receiverHello\n");
    returnStatus = receiverHello(sendPacket, receivedPacket, receiverSecret, senderModExp, (unsigned char *) Enc_ReceiverPrivateExp);

    channel_write(sendPacket, ENC_KEY_PACKET_CHARS);

    return returnStatus;
}

void receiver_deriveKey() {
	digit_t symmetricKey[ENC_PRIVATE_KEY_DIGITS];

    printf("--> receiver_deriveKey\n");

	_calculateSymmetricKey(symmetricKey, senderModExp, receiverSecret);
	_deriveKeys(receiverAESKey, receiverHashKey, receiverCTRNonce, symmetricKey);
}

int receiver_receiveData() {
    unsigned short i;
    unsigned char encryptedData[ENC_DATA_SIZE_CHARS];

    field_t dataPacket[ENC_DATA_PACKET_CHARS];

    uint32_t receivedPacketCounter;

    printf("\n# Receiver\n");
    printf("--------\n");

    receiver_deriveKey();
    channel_read(dataPacket, ENC_DATA_PACKET_CHARS);

    for (i = 0; i < ENC_DATA_PACKET_CHARS; i++)
        printf("%x", dataPacket[i]);
    printf("\n");

    for (i = 0; i < sizeof(uint32_t); i++)
        receivedPacketCounter = (receivedPacketCounter << 8) + dataPacket[i+1];

    if (receiver_checkHmac(dataPacket) == ENC_HMAC_REJECTED) {
        return ENC_HMAC_REJECTED;
    } else if (dataPacket[0] != 0x03) {
        return ENC_REJECT_PACKET_TAG;
    } else if (*receiverPacketCounter > receivedPacketCounter) {
        return ENC_LOST_PACKET;
    } else {
        while (*receiverPacketCounter != receivedPacketCounter) {
            if (increaseCounter(receiverPacketCounter) == ENC_COUNTER_WRAPAROUND)
                return ENC_COUNTER_WRAPAROUND;
        }

        printf("--| receiverPacketCounter: %d\n", *receiverPacketCounter);

        for (i = 0; i < ENC_DATA_SIZE_CHARS; i++)
            encryptedData[i] = dataPacket[i+5];

        return ENC_ACCEPT_PACKET;
    }
}

int receiver_checkHmac(field_t *dataPacket) {
    unsigned char i;
    uint8_t hmac[ENC_HMAC_CHARS];

    _hmac(hmac, dataPacket, receiverHashKey, ENC_HMAC_CHARS, 5+ENC_DATA_SIZE_CHARS, ENC_HASH_CHARS/2);

    for (i = 0; i < ENC_HMAC_CHARS; i++) {
        if (hmac[i] != dataPacket[5+ENC_DATA_SIZE_CHARS+i])
            return ENC_HMAC_REJECTED;
    }

    return ENC_HMAC_ACCEPTED;
}

void receiver_destruct() {
	free(senderModExp);
    free(receiverSecret);
    free(receiverAESKey);
	free(receiverHashKey);
	free(receiverCTRNonce);
    free(receiverPacketCounter);
}
