#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "util.h"
// Thread block size
#define BLOCK_SIZE 16

__global__ void pooling_cuda(RGB *bitmap, RGB *new_bitmap, char type, int N, int height, int width){
	int row = blockIdx.y * blockDim.y + threadIdx.y; //i
 	int col = blockIdx.x * blockDim.x + threadIdx.x; //j

	int idx_i, idx_j, idx_max, idx_min;
	RGB min, max;
	int idx;
	int k, m;

	if(row >= height || col >= width){
		return;
	}

	if(type == 'M')	{
		idx = row * width + col;
		max.blue = bitmap[idx].blue;
		max.green = bitmap[idx].green;
		max.red = bitmap[idx].red;
		for(k = 0; k < N; k++){
			for(m = 0; m < N; m++){
				//verifica daca este vecin valid
				idx_i = row - N / 2 + k;
				idx_j = col - N / 2 + m;
				idx_max = idx_i * width + idx_j;
				if(!(idx_i < 0 || idx_j < 0 || 
					idx_i >= height || idx_j >= width)){
					if(bitmap[idx_max].blue > max.blue){
						max.blue = bitmap[idx_max].blue;
					}
					if(bitmap[idx_max].green > max.green){
						max.green = bitmap[idx_max].green;
					}
					if(bitmap[idx_max].red > max.red){
						max.red = bitmap[idx_max].red;
					}
				}
			}
		}
		new_bitmap[idx].blue = max.blue;
		new_bitmap[idx].green = max.green;
		new_bitmap[idx].red = max.red;
	} else {
		idx = row * width + col;
		min.blue = bitmap[idx].blue;
		min.green = bitmap[idx].green;
		min.red = bitmap[idx].red;
		for(k = 0; k < N; k++){
			for(m = 0; m < N; m++){
				//verifica daca este vecin valid
				idx_i = row - N / 2 + k;
				idx_j = col - N / 2 + m;
				idx_min = idx_i * width + idx_j;
				if(!(idx_i < 0 || idx_j < 0 || 
					idx_i >= height || idx_j >= width)){
					if(bitmap[idx_min].blue < min.blue)
						min.blue = bitmap[idx_min].blue;
					if(bitmap[idx_min].green < min.green)
						min.green = bitmap[idx_min].green;
					if(bitmap[idx_min].red < min.red)
						min.red = bitmap[idx_min].red;
				}
			}
		}
		new_bitmap[idx].blue = min.blue;
		new_bitmap[idx].green = min.green;
		new_bitmap[idx].red = min.red;
	}
}

// functie care citeste headerele si bitmap-ul pozei
void read_photo(params *param, bmp_photo *photo){
    FILE *input_fd = fopen(param->input_file,"rb");
	if(input_fd == NULL)
		printf("Nu s-a putut deschide fisierul\n");

	//citire header
	fread(photo->header, sizeof(bmp_fileheader), 1, input_fd);

	//citire infoheader
	fread(photo->infoheader, sizeof(bmp_infoheader), 1, input_fd);
	
    cudaMallocManaged(&photo->bitmap, photo->infoheader->width * photo->infoheader->height * sizeof(RGB));

    read_bitmap(photo, input_fd);

    fclose(input_fd);
}


int main(int argc, char *argv[]){

    // alocare memorie pentru argumentele de rulare
    params *param = alloc_params();

    // se citesc argumentele programului
	get_args(argc, argv, param);

    bmp_photo *photo = alloc_photo();

	cudaMallocManaged(&photo->bitmap, photo->infoheader->width * photo->infoheader->height * sizeof(RGB));

    //citirea pozei
    read_photo(param, photo);

    bmp_photo *new_photo = alloc_photo();
	cudaMallocManaged(&new_photo->bitmap, photo->infoheader->width * photo->infoheader->height * sizeof(RGB));
    memcpy(new_photo->header, photo->header, sizeof(bmp_fileheader));
    memcpy(new_photo->infoheader, photo->infoheader, sizeof(bmp_infoheader));

	
	dim3 dimBlock(BLOCK_SIZE, BLOCK_SIZE);

	size_t grid_size_x = photo->infoheader->width / dimBlock.x;
	if (photo->infoheader->width % BLOCK_SIZE) 
		++grid_size_x;
	
	size_t grid_size_y = photo->infoheader->height / dimBlock.y;
	if (photo->infoheader->height % BLOCK_SIZE) 
		++grid_size_y;

    dim3 dimGrid(grid_size_x, grid_size_y);

	cudaEvent_t start, stop;
	cudaEventCreate(&start);
	cudaEventCreate(&stop);

	cudaEventRecord(start);
	//Executia kernelul-ului
	pooling_cuda<<<dimGrid, dimBlock>>>(photo->bitmap,
										new_photo->bitmap,
										'M',
										5, 
										photo->infoheader->height,
										photo->infoheader->width);
	
	cudaDeviceSynchronize();
	cudaEventRecord(stop);
	cudaEventSynchronize(stop);
	float milliseconds = 0;
	cudaEventElapsedTime(&milliseconds, start, stop);
    printf("Algorithm took %f milliseconds to execute\n", milliseconds);
 
    //scrierea pozei
    write_photo(param, new_photo);

    // free memory
	cudaFree(new_photo->bitmap);
	cudaFree(photo->bitmap);
    free_params(param);
    free_photo(new_photo);
    free_photo(photo);
    return 0;
}
