// Table G.7: Bit assignment for SIGNAL field
//  0   1   RATE:R1         8   0   -               16  0   LENGTH (MSB)
//  1   0   RATE:R1         9   0   -               17  0   PARITY
//  2   1   RATE:R1         10  1   -               18  0   SIGNAL TAIL
//  3   1   RATE:R1         11  1   -               19  0   SIGNAL TAIL
//  4   0   [reserved]      12  0   -               20  0   SIGNAL TAIL
//  5   0   LENGTH(LSB)     13  0   -               21  0   SIGNAL TAIL
//  6   0   -               14  0   -               22  0   SIGNAL TAIL
//  7   1   -               15  0   -               23  0   SIGNAL TAIL
//      1011 0001               0011 0000               0000 0000
unsigned char annexg_G7[3] = {0xb1, 0x30, 0x00};
