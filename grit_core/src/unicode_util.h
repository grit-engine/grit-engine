
int encode_utf8 (unsigned long x, char *&encoded)
{
    if (x < 0x80) {
        encoded[0] = x;
        encoded[1] = 0;
        return 1;
    } else if (x < 0x800) {
        long bytes = 0xC080                                                     | ((x & 0xF7C0) << 2) | (x & 0x3F);
        encoded[0] = (bytes >> 8) & 0xFF;
        encoded[1] = (bytes >> 0) & 0xFF;
        encoded[2] = 0;
        return 2;
    } else if (x < 0x10000) {
        long bytes = 0xE08080                           | ((x & 0x03F000) << 4) | ((x & 0x0FC0) << 2) | (x & 0x3F);
        encoded[0] = (bytes >> 16) & 0xFF;
        encoded[1] = (bytes >>  8) & 0xFF;
        encoded[2] = (bytes >>  0) & 0xFF;
        encoded[3] = 0;
        return 3;
    } else if (x < 0x110000) {
        long bytes = 0xF0808080 | ((x & 0x1C0000) << 6) | ((x & 0x03F000) << 4) | ((x & 0x0FC0) << 2) | (x & 0x3F);
        encoded[0] = (bytes >> 24) & 0xFF;
        encoded[1] = (bytes >> 16) & 0xFF;
        encoded[2] = (bytes >>  8) & 0xFF;
        encoded[3] = (bytes >>  0) & 0xFF;
        encoded[4] = 0;
        return 4;
    } else {
        fprintf(stderr, "Codepoint out of range: %lu\n", x);
        exit(EXIT_FAILURE);
        return 0; // suppress msvc warning
    }
}

