/* bitmap structures */

#define OFFSET_BMP_HEADER 0
#define OFFSET_BMP_INFO_HEADER (sizeof(struct bmpHeader))
#define NUM_PALETTE_8BPP 256
#define OFFSET_BMP_DATA_16BPP (sizeof(struct bmpHeader) + sizeof(struct bmpInfoHeader))

struct bmpHeader {
	unsigned short bfType;
	unsigned int   bfSize;
	unsigned short bfReserved1;
	unsigned short bfReserved2;
	unsigned int   bfOffBits;
} __attribute__((packed));

struct bmpInfoHeader {
	unsigned int   biSize;
    int            biWidth;
    int            biHeight;
    unsigned short biPlanes;
    unsigned short biBitCount;
    unsigned int   biCompression;
    unsigned int   biSizeImage;
    long           biXPixPerMeter;
    long           biYPixPerMeter;
    unsigned int   biClrUsed;
    unsigned int   biClrImportant;
} __attribute__((packed));

struct rgbQuadPalette {
	unsigned char rgbBlue;
    unsigned char rgbGreen;
    unsigned char rgbRed;
    unsigned char rgbReserved;
} __attribute__((packed));
