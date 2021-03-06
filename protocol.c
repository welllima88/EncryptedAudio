#include "protocol.h"

void senderHello(field_t *restrict sendPacket, digit_t *restrict senderModExp, digit_t *restrict senderSecret) {
    digit_t modExpResult[ENC_PRIVATE_KEY_DIGITS];

    // Generate x
    getRandomDigit(senderSecret);

	// Calculate alpha^x mod p = generator^senderSecret mod prime
    mpModExp(modExpResult, Enc_GeneratorDigits, senderSecret, Enc_PrimeDigits, ENC_PRIVATE_KEY_DIGITS);
    sendPacket[0] = 0x00;
    memcpy(sendPacket+1, modExpResult, ENC_PRIVATE_KEY_CHARS);
    memcpy(senderModExp, modExpResult, ENC_PRIVATE_KEY_DIGITS*sizeof(digit_t));
}

int receiverHello(field_t *restrict sendPacket, digit_t *restrict receiverModExp, field_t *restrict receivedPacket, digit_t *restrict receiverSecret, digit_t *restrict senderModExp, unsigned char *restrict receiverPrivateExp) {
    if (0x00 != receivedPacket[0])
        return ENC_REJECT_PACKET_TAG;

    unsigned char cSignature[ENC_ENCRYPTED_SIGNATURE_CHARS];

    digit_t exponent[ENC_PRIVATE_KEY_DIGITS];
    digit_t signature[ENC_SIGNATURE_DIGITS];
    digit_t signatureMessage[2*ENC_PRIVATE_KEY_DIGITS];

    field_t encryptedSignature[ENC_ENCRYPTED_SIGNATURE_CHARS];

    uint8_t receiverCTRNonce[ENC_CTR_NONCE_CHARS];
    uint8_t receiverAESKey[ENC_AES_KEY_CHARS];

    // Generate y
    getRandomDigit(receiverSecret);

	// Calculate alpha^y mod p = alpha^receiverSecret mod prime
    mpModExp(receiverModExp, Enc_GeneratorDigits, receiverSecret, Enc_PrimeDigits, ENC_PRIVATE_KEY_DIGITS);

	// Concatenate alpha^y | alpha^x
    memcpy(senderModExp, receivedPacket+1, ENC_PRIVATE_KEY_CHARS);
    memcpy(signatureMessage, senderModExp, ENC_PRIVATE_KEY_DIGITS*sizeof(digit_t));
    memcpy(signatureMessage+ENC_PRIVATE_KEY_DIGITS, receiverModExp, ENC_PRIVATE_KEY_DIGITS*sizeof(digit_t));

    // Derive Keys
    receiver_deriveKey(receiverAESKey, receiverCTRNonce, senderModExp);

    // Create Signature
    memset(signature, 0, sizeof(signature));
    mpConvFromOctets(exponent, ENC_SIGNATURE_DIGITS, receiverPrivateExp, ENC_PRIVATE_KEY_CHARS);
    _sign_crt(signature, signatureMessage, exponent, Enc_ReceiverPrimeOneDigits, Enc_ReceiverPrimeTwoDigits);

    #ifndef __ENC_NO_ENCRYPTION_PRINTS__
        printf("---| signature\n");
        mpPrintNL(signature, ENC_SIGNATURE_DIGITS);
    #endif

    // Encrypt Signature
    mpConvToOctets(signature, ENC_SIGNATURE_DIGITS, cSignature, ENC_ENCRYPTED_SIGNATURE_CHARS);
    _encryptData(encryptedSignature, receiverAESKey, receiverCTRNonce, 0, cSignature, ENC_ENCRYPTED_SIGNATURE_CHARS);
    #ifndef __ENC_NO_ENCRYPTION_PRINTS__
        printf("---| encyptedSignature\n");
        mpConvFromOctets(signature, ENC_ENCRYPTED_SIGNATURE_DIGITS, encryptedSignature, ENC_ENCRYPTED_SIGNATURE_CHARS);
        mpPrintNL(signature, ENC_SIGNATURE_DIGITS);
    #endif

    sendPacket[0] = 0x01;
    memcpy(sendPacket+1, receiverModExp, ENC_PRIVATE_KEY_DIGITS*sizeof(digit_t));
    memcpy(sendPacket+ENC_PRIVATE_KEY_CHARS+1, encryptedSignature, ENC_ENCRYPTED_SIGNATURE_CHARS);

    return ENC_ACCEPT_PACKET;
}

int senderAcknowledge(field_t *restrict sendPacket, field_t *restrict receivedPacket, digit_t *restrict senderSecret, digit_t *restrict receiverModExp, digit_t *restrict senderModExp, unsigned char *restrict senderPrivateExp) {
    if (0x01 != receivedPacket[0])
        return ENC_REJECT_PACKET_TAG;

    unsigned char cSignature[ENC_ENCRYPTED_SIGNATURE_CHARS];

    digit_t exponent[ENC_PRIVATE_KEY_DIGITS];

    digit_t signature[ENC_SIGN_MODULUS_DIGITS];
    digit_t signatureMessageDigits[2*ENC_PRIVATE_KEY_DIGITS];

    field_t encryptedSignature[ENC_ENCRYPTED_SIGNATURE_CHARS];

    uint8_t signatureMessage[2*ENC_PRIVATE_KEY_CHARS];
    uint8_t senderCTRNonce[ENC_CTR_NONCE_CHARS];
    uint8_t senderAESKey[ENC_AES_KEY_CHARS];

    mpSetZero(signature, ENC_SIGN_MODULUS_DIGITS);

    // Concatenate alpha^y | alpha^x
    memcpy(receiverModExp, receivedPacket+1, ENC_PRIVATE_KEY_CHARS);
    
    memcpy(signatureMessageDigits, senderModExp, ENC_PRIVATE_KEY_DIGITS*sizeof(digit_t));
    memcpy(signatureMessageDigits+ENC_PRIVATE_KEY_DIGITS, receiverModExp, ENC_PRIVATE_KEY_DIGITS*sizeof(digit_t));

    //deriveKey from receiverModExp
    sender_deriveKey(senderAESKey, senderCTRNonce, receiverModExp);

    // Decrypt signature
    memcpy(encryptedSignature, receivedPacket+ENC_PRIVATE_KEY_CHARS+1, ENC_ENCRYPTED_SIGNATURE_CHARS);
    #ifndef __ENC_NO_ENCRYPTION_PRINTS__
        printf("---| encryptedSignature\n");
        mpConvFromOctets(signature, ENC_ENCRYPTED_SIGNATURE_DIGITS, encryptedSignature, ENC_ENCRYPTED_SIGNATURE_CHARS);
        mpPrintNL(signature, ENC_SIGNATURE_DIGITS);
    #endif

    _decryptData(cSignature, senderAESKey, senderCTRNonce, 0, (unsigned char *) encryptedSignature, ENC_ENCRYPTED_SIGNATURE_CHARS);

    // Verify signature
    mpConvFromOctets(signature, ENC_ENCRYPTED_SIGNATURE_DIGITS, cSignature, ENC_ENCRYPTED_SIGNATURE_CHARS);

    #ifndef __ENC_NO_ENCRYPTION_PRINTS__
        printf("---| signature\n");
        mpPrintNL(signature, ENC_SIGN_MODULUS_DIGITS);
    #endif

    mpConvToOctets(signatureMessageDigits, ENC_PRIVATE_KEY_DIGITS*2, signatureMessage, ENC_PRIVATE_KEY_CHARS*2);
    if (!_verify(signature, signatureMessage, Enc_PublicExpDigits, Enc_ReceiverModulusDigits))
        return ENC_REJECT_PACKET_SIGNATURE;

    // Concatenate alpha^x | alpha^y
    memcpy(signatureMessageDigits, receiverModExp, ENC_PRIVATE_KEY_CHARS);
    memcpy(signatureMessageDigits+ENC_PRIVATE_KEY_DIGITS, senderModExp, ENC_PRIVATE_KEY_CHARS);

    // Create Signature
    mpConvFromOctets(exponent, ENC_SIGNATURE_DIGITS, senderPrivateExp, ENC_PRIVATE_KEY_CHARS);
    _sign_crt(signature, signatureMessageDigits, exponent, Enc_SenderPrimeOneDigits, Enc_SenderPrimeTwoDigits);

    // Encrypt signature
    memset(cSignature, 0, sizeof(cSignature));
    mpConvToOctets(signature, ENC_SIGNATURE_DIGITS, cSignature, ENC_ENCRYPTED_SIGNATURE_CHARS);
    _encryptData(encryptedSignature, senderAESKey, senderCTRNonce, 0, cSignature, ENC_ENCRYPTED_SIGNATURE_CHARS);

    sendPacket[0] = 0x01;
    mpConvFromOctets(signature, ENC_SIGNATURE_DIGITS, encryptedSignature, ENC_ENCRYPTED_SIGNATURE_CHARS);
    memcpy(sendPacket+1, encryptedSignature, ENC_ENCRYPTED_SIGNATURE_CHARS);

    return ENC_ACCEPT_PACKET;
}

int increaseCounter(uint32_t *counter) {
	uint32_t nextValue = *counter + 1;

	if (*counter > nextValue) {
		*counter = nextValue;
		return ENC_COUNTER_WRAPAROUND;
	} else {
		*counter = nextValue;
		return ENC_ACCEPT_PACKET;
	}
}
