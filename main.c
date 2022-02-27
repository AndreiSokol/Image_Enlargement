#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

uint8_t* originImg, outImage;
unsigned int spaceImage[10];
int dx[4]={0, -1, 0, 1};
int dy[4]={1, 0, -1, 0};
struct bitMapFileHeader fileHeader;
struct bitMapImageHeader imageHeader;

struct bitMapFileHeader
{
    char file_type[2];
    unsigned int bm_size; 
    unsigned int reserved; 
    unsigned int offset;
};

struct bitMapImageHeader
{
    unsigned int bmih_size;
    unsigned int width;
    unsigned int height;
    unsigned short int colorplanes;
    unsigned short int bitsperpixel;
    unsigned int compression;
    unsigned int image_size;
};

int openFile()
{
	//open image file "lena.bmp"
    FILE *file = fopen("pics/lena.bmp", "rb");
	//read the file header
    fread(fileHeader.file_type, 2, 1, file);
    fread(&fileHeader.bm_size, 3 * sizeof(int), 1, file);
	//read the image header
    fread(&imageHeader.bmih_size, sizeof(struct bitMapImageHeader), 1, file);
    printf("Header Size:%d\nWidth:%d\nHeight:%d\nColorplanes:%d\nBPP:%d\nCompression:%d\nImage Size:%d\n", imageHeader.bmih_size,
           imageHeader.width, imageHeader.height, imageHeader.colorplanes,
           imageHeader.bitsperpixel, imageHeader.compression, imageHeader.image_size);
    
    fread(spaceImage, 4 * sizeof(int), 1, file);
    //malloc the image memory
    originImg = (unsigned char*)malloc(sizeof(unsigned char)* imageHeader.image_size);
    //read the image part
    fread(originImg, sizeof(unsigned char), imageHeader.image_size, file);
    
    fclose(file);
    return 0;
};

int getArrayPos(int width, int height, int x, int y)
{
	int ry = height - 1 - y;
	return (ry * width + x) * 3 + (width % 4) * ry;
}

    
void zoom(const uint8_t *img, size_t width, size_t height, size_t scale_factor, uint8_t *result) {
	//get the new width of output image
	int nWidth = width * scale_factor;
	int nHeight = height * scale_factor;
	//malloc the memory for output image
	int newSize = 3 * nWidth * nHeight + (nWidth % 4) * nHeight;
	result = (unsigned char*)malloc(newSize);
	
	int i, k, p, t, h, m;
	
	for(i = 0 ; i < newSize ; i++)result[i]=0;
	
	for(i = 0; i < height; i++)
	{
		for(k = 0; k < width; k++)
		{
			int sx = scale_factor * k;
			int sy = scale_factor * i;
			
			//color to the new point
			int pixelPos = getArrayPos(width, height, k, i);
			int newPos = getArrayPos(nWidth, nHeight, sx, sy);
			for(t = 0; t < 3 ; t++)
			{
				result[newPos + t]=img[pixelPos + t];
				
			}
			//printf("%d\t%d\t%d\t%d\n", k, i, sx, sy);
			int iterNum = 2;
			
			sx++;
			sy--;
			
			for(h = 0; h < scale_factor/2; h++)
			{
				int nx = sx;
				int ny = sy;
				int breakNum = 0;
				
				for(p = 0; p < 4 ; p++)
				{
					for(m = 0; m < iterNum; m++){
						nx += dx[p];
						ny += dy[p];
						breakNum++;
						if(!(scale_factor%2) && h == scale_factor/2-1 && breakNum == scale_factor*2)break;
						
						if(nx >-1 && nx <nWidth && ny>-1 && ny<nHeight )
						{
							int pixelPos =  getArrayPos(width, height, k, i);
							
							int newPos = getArrayPos(nWidth, nHeight, nx, ny);
							//printf("%d\t%d\t%d\t%d\n", k, i, nx, ny);
							for(t = 0; t < 3 ; t++)
							{
								result[newPos + t]=img[pixelPos + t];
							}
						}
					}
					if(!(scale_factor%2) && h == scale_factor/2-1 && breakNum == scale_factor*2 && m<iterNum)break;
				}
				sx++;
				sy--;
				iterNum+=2; 
			}
			
		}
	}
	
	int restNum = (scale_factor - 1) / 2;
	for(t = 0; t < restNum; t++)
	{
		//fill the last column with the left cells.
		k = nWidth - restNum + t;
		for(i = 0 ; i < nHeight ; i++){
			int pixelPos = getArrayPos(nWidth, nHeight, k, i);
			for(p = 0; p < 3; p++){
				result[pixelPos + p] = result[pixelPos - 3 + p];
			}
		}
		//fill the last rows with the upper cells.
		i = nHeight - restNum + t;
		for(k = 0; k < nWidth; k++){
			int pixelPos = getArrayPos(nWidth, nHeight, k, i);
			int sourcePos = getArrayPos(nWidth, nHeight, k, i - 1);
			for(p = 0; p < 3; p++){
				result[pixelPos + p] = result[sourcePos + p];
			}
		}
	}

	
	
	
	//Output to the image file
	//Set the image header of output image
	imageHeader.width *= scale_factor;
	imageHeader.height *= scale_factor;
	imageHeader.image_size = newSize;
	//Set the name of output image as "out.bmp"
	FILE *out = fopen("pics/out.bmp","wb");
    
    //Write the file header
    fwrite(fileHeader.file_type, 2, 1, out);
    fwrite(&fileHeader.bm_size, 3 * sizeof(int), 1, out);
    //write the image header
    fwrite(&imageHeader.bmih_size, sizeof(struct bitMapImageHeader), 1, out);
    fwrite(spaceImage, 4 * sizeof(int), 1, out);
    //write the image part
    fwrite(result, sizeof(unsigned char), imageHeader.image_size, out);
    
    //close the output file
    fclose(out);
	//free memory of image
	free(originImg);
	free(outImage);
}

int main(int argc, char **argv){
	int scale = 3;
	
	printf("Please input the ratio of Enlargement: ");
	
	scanf("%d",&scale);
	
	
	openFile();
	
	zoom(originImg, imageHeader.width, imageHeader.height, scale, outImage);
	
	return 0;
}


