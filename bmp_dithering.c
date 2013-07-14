#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "bmp.h"

#define ALIGN(x, a)	(((x) + (a) - 1) & ~((a) - 1))

void exec_dither(char *image, int width, int height)
{
	char e;
	unsigned char f;
	int i, j;
	unsigned char *img = (char *)image;

	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			f = img[i * width + j];
			//printf("0x%02x ", f);
			//if (j % 16 == 0) { printf("\n"); }

			if (f > (0xFF >> 1)){
				img[i * width + j] = 0xFF;
				e = f - 0xFF;
			}
			else {
				img[i * width + j] = 0x00;
				e = f;
			}

			if (j != (width - 1)) {
				img[i * width + j + 1] += (5.0 * e) / 16;
			}
			if (!j && i != (height - 1)) {
				img[(i + 1) * width + j - 1] += (3.0 * e) / 16;
			}
			if (i != (height - 1)) {
				img[(i + 1) * width + j] += (5.0 * e) / 16;
			}
			if (j != (width - 1) && i != (height - 1)) {
				img[(i + 1) * width + j + 1] += (3.0 * e) / 16;
			}
		}
	}
}


int main(int argc, char *argv[])
{

	char *input_file, *y8_output_file, *dither_output_file;
	struct stat input_file_stat;
	unsigned int input_filesize, y8_output_filesize;
	char *input_bmpbuf, *y8_bmpbuf, *dither_bmpbuf;
	FILE *fp_in, *fp_y8, *fp_dither;
	struct bmpHeader input_bmp_header, y8_bmp_header;
	struct bmpInfoHeader input_bmp_info, y8_bmp_info;
	struct rgbQuadPalette palette;
	int width, height, width_8bpp_bmp; /* 8bpp bitmap image width have to be aligned 4byte. */
	int i, j;
	int written_data_cnt;
	short *input_curr_pix;
	char *y8_curr_pix;
	short tmp;
	
	if (argc != 4) {
		fprintf(stderr, "Argument is wrong.\n");
		exit(EXIT_FAILURE);
	}

	input_file = argv[1];
	y8_output_file = argv[2];
	dither_output_file = argv[3];

	/* Get file size. */
	stat(input_file, &input_file_stat);
	input_filesize = (unsigned int)input_file_stat.st_size;
	fprintf(stderr, "filesize:%d\n", input_filesize);
	

	/* Allocate input_bmpbuf memory. */
	if ((input_bmpbuf = (char *)calloc(input_filesize, sizeof(char))) == NULL) {
		fprintf(stderr, "Memory allocation error.\n");
		exit(EXIT_FAILURE);
	}

	/* Open input bmp file. */
	if ((fp_in = fopen(input_file, "rb")) == NULL) {
		fprintf(stderr, "Input bmp file open error.\n");
		free(input_bmpbuf);
		exit(EXIT_FAILURE);
	}
	
	/* Load input bmp file. */
	if (fread(input_bmpbuf, sizeof(char), input_filesize, fp_in) != input_filesize) {
		fprintf(stderr, "Input bmp file read error.\n");
		fclose(fp_in);
		free(input_bmpbuf);
		exit(EXIT_FAILURE);
	}
	fclose(fp_in);

	/* Get bmp information. */
	input_bmp_header = *((struct bmpHeader *)(input_bmpbuf + OFFSET_BMP_HEADER));
	input_bmp_info = *((struct bmpInfoHeader *)(input_bmpbuf + OFFSET_BMP_INFO_HEADER));

	/* Check header. */
	if ((char)((input_bmp_header.bfType >> 8) & 0xFF) != 'M' ||
		(input_bmp_header.bfType & 0xFF) != 'B') {
		
		fprintf(stderr, "Input file header is not correct.\n");
		free(input_bmpbuf);
		exit(EXIT_FAILURE);
	}

	/* Get width and height. */
	width = input_bmp_info.biWidth;
	height = input_bmp_info.biHeight;

	/* Calculate 8bpp bitmap width. */
	width_8bpp_bmp = ALIGN(width, 4);

	/* Calculate Y8 bmp filesize. */
	y8_output_filesize = sizeof(struct bmpHeader) + sizeof(struct bmpInfoHeader) +
		(sizeof(struct rgbQuadPalette) * NUM_PALETTE_8BPP) + (width_8bpp_bmp * height);

	/* Create Y8 bmp. */
	/* Open Y8 bmp file for write mode. */
	if ((fp_y8 = fopen(y8_output_file, "wb")) == NULL) {
		fprintf(stderr, "Y8 bmp file open failed.\n");
		free(input_bmpbuf);
		exit(EXIT_FAILURE);
	}
	written_data_cnt = 0;
	
	/* Setup bmpHeader. */
	y8_bmp_header.bfType = ((short)('M')) << 8 | ((short)('B'));
	y8_bmp_header.bfSize = y8_output_filesize;
	y8_bmp_header.bfReserved1 = 0;
	y8_bmp_header.bfReserved2 = 0;
	y8_bmp_header.bfOffBits = sizeof(struct bmpHeader) + sizeof(struct bmpInfoHeader) +
		(sizeof(struct rgbQuadPalette) * NUM_PALETTE_8BPP);

	/* Write bmpHeader. */
	if (fwrite(&y8_bmp_header, sizeof(struct bmpHeader), 1, fp_y8) == 1) {
		written_data_cnt += sizeof(struct bmpHeader);
	}

	/* Setup bmpInfoHeader. */
	y8_bmp_info.biSize = 40; /* Fixed value */
	y8_bmp_info.biWidth = (short)width_8bpp_bmp;
	y8_bmp_info.biHeight = (short)height;
	y8_bmp_info.biPlanes = 1; /* Fixed value */
	y8_bmp_info.biBitCount = 8; /* 8bits per pixel */
	y8_bmp_info.biCompression = 0; /* non compression */
	y8_bmp_info.biSizeImage = 0;
	y8_bmp_info.biXPixPerMeter = 0;
	y8_bmp_info.biYPixPerMeter = 0;
	y8_bmp_info.biClrUsed = 0;
	y8_bmp_info.biClrImportant = 0;
	
	/* Write bmpInfoHeader. */
	if (fwrite(&y8_bmp_info, sizeof(struct bmpInfoHeader), 1, fp_y8) == 1) {
		written_data_cnt += sizeof(struct bmpInfoHeader);
	}

	/* Setup and write color palette. */
	for (i = 0; i < NUM_PALETTE_8BPP; i++) {
		palette.rgbBlue = palette.rgbGreen = palette.rgbRed = i;
		palette.rgbReserved = 0;
		if (fwrite(&palette, sizeof(struct rgbQuadPalette), 1, fp_y8) == 1) {
			written_data_cnt += sizeof(struct rgbQuadPalette);
		}
	}

	/* Allocate y8_bmpbuf memory. */
	if ((y8_bmpbuf = calloc(width_8bpp_bmp * height, sizeof(char))) == NULL) {
		fprintf(stderr, "Memory allocation error.\n");
		fclose(fp_y8);
		free(input_bmpbuf);
		exit(EXIT_FAILURE);
	}

	printf("size : width=%d, height=%d.\n", width, height);

	/* Convert RGB565 to Y8 and store data in y8_bmpbuf. */
	input_curr_pix = (short *)((char *)input_bmpbuf + OFFSET_BMP_DATA_16BPP);
	y8_curr_pix = (char *)y8_bmpbuf;
	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			tmp = ((((*input_curr_pix & 0xF800) >> 8) + ((*input_curr_pix & 0x07C0) >> 3) +
					((*input_curr_pix & 0x001F) << 3)) / 3) & 0xF0;
			*y8_curr_pix = (char)tmp;
			//fprintf(stderr, "%d, %d\n", i, j);
			input_curr_pix++;
			y8_curr_pix++;
		}
		/* 8bpp bitmap image has to be 4byte aligned. */
		for (j = 0; j < width_8bpp_bmp - width; j++) {
			*y8_curr_pix = (char)0x00;
			y8_curr_pix++;
		}
	}
	free(input_bmpbuf);

	/* Write pixel data. */
	written_data_cnt += fwrite(y8_bmpbuf, sizeof(char), width_8bpp_bmp * height, fp_y8);

	/* Check written data size. */
	if (written_data_cnt != y8_output_filesize) {
		fprintf(stderr, "Y8 bmp file write failed.\n");
		free(y8_bmpbuf);
		fclose(fp_y8);
		exit(EXIT_FAILURE);
	}
	fclose(fp_y8);

	/* Open dither bmp file for write mode. */
	if ((fp_dither = fopen(dither_output_file, "wb")) == NULL) {
		fprintf(stderr, "Dither bmp file open failed.\n");
		free(y8_bmpbuf);
		exit(EXIT_FAILURE);
	}
	written_data_cnt = 0;
	
	/* Write bmpHeader. */
	if (fwrite(&y8_bmp_header, sizeof(struct bmpHeader), 1, fp_dither) == 1) {
		written_data_cnt += sizeof(struct bmpHeader);
	}

	/* Write bmpInfoHeader. */
	if (fwrite(&y8_bmp_info, sizeof(struct bmpInfoHeader), 1, fp_dither) == 1) {
		written_data_cnt += sizeof(struct bmpInfoHeader);
	}

	/* Setup and write color palette. */
	for (i = 0; i < NUM_PALETTE_8BPP; i++) {
		palette.rgbBlue = palette.rgbGreen = palette.rgbRed = i;
		palette.rgbReserved = 0;
		if (fwrite(&palette, sizeof(struct rgbQuadPalette), 1, fp_dither) == 1) {
			written_data_cnt += sizeof(struct rgbQuadPalette);
		}
	}

	/* Dither processing. */
	exec_dither(y8_bmpbuf, width_8bpp_bmp, height);

	/* Write pixel data. */
	written_data_cnt += fwrite(y8_bmpbuf, sizeof(char), width_8bpp_bmp * height, fp_dither);

	/* Check written data size. */
	if (written_data_cnt != y8_output_filesize) {
		fprintf(stderr, "Dither bmp file write failed.\n");
		free(y8_bmpbuf);
		fclose(fp_dither);
		exit(EXIT_FAILURE);
	}
	free(y8_bmpbuf);
	fclose(fp_dither);
	
	printf("success\n");
	exit(EXIT_SUCCESS);
}
