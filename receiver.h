#ifndef __ENC_RECEIVER_H__
#define __ENC_RECEIVER_H__

#include <string.h>

#include "buffer.h"
#include "channel.h"
#include "protocol.h"

void receiver_construct();

int receiver_receiverHello();
void receiver_deriveKey();
int receiver_receiveData();

#endif
