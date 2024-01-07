# Authenticated switch


packet size:
- transmit 16 bytes at a time + Header
- STS protocol: ECC_PUB_KEY_SIZE = 48 bytes
- 2*BLOCKSIZE = 32 bytes
- ----> 48 bytes
- max radio packet size is 32 bytes
- nonce = 8 bytes -> top half, bottom half is counter (not transmitted, fixed in the protocol to 0)
    - pad with 8 bytes


k = preshared key (authentication)
s = session key (forward secrecy)

                                    pubR **48 bytes**
Remote          ------------------------------------------------------>          Switch
Generate (pubR, privR)

                   pubS | nonce | PAD8 | E_s(MAC_k(pubS | pubR))  **80 bytes** ctr = 0
Remote          <------------------------------------------------------          Switch
                                                                        Generate (pubS, privS)
                                                                        Compute shared secret s = ecdh(privS, pubR)
                                                                        Compute MAC(pubS | pubR) with preshared key k (NOTE THE MAC ORDER). The MAC with k is a "signature".
                                                                        Encrypt MAC

                     E_s(MAC_k(pubR | pubS)) **16 bytes** ctr = 1
Remote          ------------------------------------------------------>          Switch
Compute shared secret s = ecdh(privR, pubS)
Compute MAC(pubR | pubS) with preshared key k
Encrypt MAC
At this point R and S know that only they possess the shared key s.
Now that STS is done, use s for both encryption and mac.

                    E_s(CMD) | MAC_s(E_s(CMD)) **32 bytes** ctr = 2
Remote          ------------------------------------------------------>          Switch

                    E_s(ACK) | MAC_s(E_s(ACK)) **32 bytes** ctr = 3
Remote          <------------------------------------------------------          Switch
