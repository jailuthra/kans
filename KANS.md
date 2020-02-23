KANS File Format
================

Kompress-ANS file header
------------------------

| byte 1 | byte 2 | byte 3             | byte 4-7       | Frequency table    |
| ------ | ------ | ------------------ | -------------- | ------------------ | 
| 0xA1   | 0x05   | no. of symbols (n) | final state x' | n-entries          | 

The hexcode **0xA105** is the indicator that the bitstream will follow
KANS format guidelines.

### Frequency table entry format

| byte 1       |  byte 2-3                |
| ------------ |  ----------------------- |
| symbol ascii |  fs in working ANS range |

KANS bitstream
--------------

Following the header, there is binary data (as 2-byte chunks).
KANS uses streamable rANS with 32-bit precision integers.

The quantization of the probabilites is over 12-bits (aka working range).
The streaming range is: I = {2^16, 2^32 - 1} with chunk size b = 16 bits.
