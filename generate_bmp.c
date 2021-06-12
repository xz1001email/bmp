#include <stdio.h>
#include <stdint.h>


/* 
 * this is a example to display gray image using bmp
 * cap_raw_raw_1600x1296_3200.raw file is a bit10 gray data,
 * should convert to bit8 when using bmp protocol
 *
 * */

typedef struct tagFileHeader {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfResv1;
    uint16_t bfResv2;
    uint32_t bfOffBits;
}__attribute__ ((packed)) FileHeader;

typedef struct tagInodeHeader {
    uint32_t biSize;
    uint32_t biWith;
    uint32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    uint32_t biXPelsPerMeter;
    uint32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
} __attribute__ ((packed)) InodeHeader;


typedef struct tagPalette {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t resv;
}__attribute__ ((packed)) PaletteT;

PaletteT g_palette[256];

char buf[1600*1296*2];
char gray[1600*1296];
#define GRAY_RAW_FILE "./cap_raw_raw_1600x1296_3200.raw"

void palette_init(void)
{
    for (int i=0; i<256; i++) {
        g_palette[i].red = i;
        g_palette[i].green = i;
        g_palette[i].blue = i;
    }
    printf("palette inited\n");
}


int main(void)
{
    FILE *fp = fopen(GRAY_RAW_FILE, "r");
    if (!fp) {
        printf("open file %s fail\n", GRAY_RAW_FILE);
        return -1;
    }

    fread(buf, 1, 1600*1296*2, fp);
    fclose(fp);

    FILE *fw = fopen("./out.bmp", "w+");
    if (!fw) {
        printf("open file fail\n");
        return -1;
    }


    uint16_t *p = (uint16_t *)buf;

    // gray bit10 to gray bit8
    for (int i=0; i<1600*1296; i++) {
        //gray[i] = p[i] >> 6;
        gray[i] = (p[i] >> 8) & 0xFF;
    }

    palette_init();

    FileHeader head;
    head.bfType = 0x4D42; //BM

    //all data size
    head.bfSize = sizeof(FileHeader) + sizeof(InodeHeader) + sizeof(g_palette) + sizeof(gray);
    head.bfResv1 = 0;
    head.bfResv2 = 0;

    // image offset
    head.bfOffBits = sizeof(FileHeader) + sizeof(InodeHeader) + sizeof(g_palette);


    InodeHeader node;
    node.biSize = sizeof(InodeHeader);
    node.biWith = 1600;
    node.biHeight = 1296;
    node.biPlanes = 1; //must set 1
    //node.biBitCount = 16;
    node.biBitCount = 8; // bit8 256 level gray
    node.biCompression = 0;

    //image data size
    node.biSizeImage = sizeof(gray);
    node.biXPelsPerMeter = 0x0EC4;
    node.biYPelsPerMeter = 0x0EC4;
    node.biClrUsed = 0;
    node.biClrImportant = 0;

    int ret = 0;
    ret += fwrite(&head, 1, sizeof(FileHeader), fw);
    printf("write head %d\n", ret);

    ret += fwrite(&node, 1, sizeof(InodeHeader), fw);
    printf("write node %d\n", ret);

    //write palette
    ret += fwrite(&g_palette[0], 1, sizeof(g_palette), fw);
    printf("write palette %d\n", ret);

    ret += fwrite(gray, 1, sizeof(gray), fw);
    printf("write image data %d\n", ret);
    fclose(fw);
    

    return 0;
}