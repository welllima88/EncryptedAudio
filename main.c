#include "protocol.h"

#include "sender.h"
#include "receiver.h"

// Private Declarations
void _handshake();

int main() {
    // Construct
    sender_construct();
    receiver_construct();

    // Handshake
    _handshake();

    // Destruct
    sender_destruct();
    receiver_destruct();

    exit(EXIT_SUCCESS);
}

void _handshake() {
    unsigned short i = 0;

    printf("# Key Exchange\n\n");

    // SenderHello
    field_t senderKeyExchangePacket[ENC_KEY_PACKET_CHARS];
    sender_senderHello(senderKeyExchangePacket);

    printf("-| senderKeyExchangePacket\n");
    for (i = 0; i < ENC_KEY_PACKET_CHARS; i++)
        printf("%x", senderKeyExchangePacket[i]);

    printf("\n\n");

    // ReceiverHello
    field_t receiverKeyExchangePacket[ENC_KEY_PACKET_CHARS];
    receiver_receiverHello(receiverKeyExchangePacket, senderKeyExchangePacket);

    printf("-| receiverKeyExchangePacket\n");
    for (i = 0; i < ENC_KEY_PACKET_CHARS; i++)
        printf("%x", receiverKeyExchangePacket[i]);

    printf("\n\n");
    
    /**
    // SenderAcknowledge
    field_t senderAcknowledgePacket[ENC_KEY_PACKET_CHARS];
    sender_senderAcknowledge(senderKeyExchangePacket, receiverKeyExchangePacket);

    printf("-| senderAcknowledgePacket\n");
    for (i = 0; i < ENC_KEY_PACKET_CHARS; i++)
        printf("%x", receiverKeyExchangePacket[i]);

    printf("\n\n"
    **/
}
