
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#ifdef __APPLE__
#include <malloc/malloc.h>
#endif

struct Image {
	unsigned long sizeX;
	unsigned long sizeY;
	char *data;
};
typedef struct Image Image;

int ImageLoad(char *filename, Image *image) {
	FILE *file;
	int size; // size of the image in bytes.
	int i; // standard counter.
	int plane; // number of planes in image
	int bpp; // number of bits per pixel
	char temp; // temporary color storage for
	if ((file = fopen(filename, "rb")) == NULL) {
		printf("File Not Found : %s\n",filename);
		return 0;
	}
	fseek(file, 18, SEEK_CUR);
	if ((i = fread(&image->sizeX, 4, 1, file)) != 1) {
		printf("Error reading width from %s.\n", filename);
		fclose(file);
		return 0;
	}
	if ((i = fread(&image->sizeY, 4, 1, file)) != 1) {
		printf("Error reading height from %s.\n", filename);
		fclose(file);
		return 0;
	}
	size = image->sizeX * image->sizeY * 3;
	if ((fread(&plane, 2, 1, file)) != 1) {
		printf("Error reading planes from %s.\n", filename);
		fclose(file);
		return 0;
	}
	if (plane != 1) {
		printf("Planes from %s is not 1: %u\n", filename, plane);
		fclose(file);
		return 0;
	}
	if ((i = fread(&bpp, 2, 1, file)) != 1) {
		printf("Error reading bpp from %s.\n", filename);
		fclose(file);
		return 0;
	}
	if (bpp != 24) {
		printf("Bpp from %s is not 24: %u\n", filename, bpp);
		fclose(file);
		return 0;
	}
	fseek(file, 24, SEEK_CUR);
	image->data = (char *)malloc(size);
	if (image->data == NULL) {
		printf("Error allocating memory for color-corrected image data");
		fclose(file);
		return 0;
	}
	if ((i = fread(image->data, size, 1, file)) != 1) {
		printf("Error reading image data from %s.\n", filename);
		fclose(file);
		return 0;
	}
	for (i=0;i<size;i+=3) { // reverse all of the colors. (bgr -> rgb)
		temp = image->data[i];
		image->data[i] = image->data[i+2];
		image->data[i+2] = temp;
	}
	fclose(file);
	return 1;
}

Image *loadTexture(char *filename) {
	Image *image1;
	image1 = (Image *) malloc(sizeof(Image));
	if (image1 == NULL) {
		printf("Error allocating space for image");
		return NULL;
	}
	if (!ImageLoad(filename, image1)) {
		return NULL;
	}
	return image1;
}

GLuint texture = 0;
void texture_init (void) {
	glGenTextures(1, &texture);
}

GLuint texture_load (char *filename) {
	Image *image1 = loadTexture(filename);
	if(image1 == NULL) {
		printf("Image was not returned from loadTexture\n");
		return -1;
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); //
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); //
	glTexImage2D(GL_TEXTURE_2D, 0, 3, image1->sizeX, image1->sizeY, 0,
	GL_RGB, GL_UNSIGNED_BYTE, image1->data);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);

	free(image1->data);
	image1->data = NULL;
	free(image1);
	image1 = NULL;
	return texture;
}

