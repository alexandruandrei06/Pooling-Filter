#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "util.h"
#include <omp.h>

void pooling(bmp_photo *photo, bmp_photo *new_photo, char type, int N){

	int i,j,k,m;
	RGB min, max;
    int height = photo->infoheader->height;
    int width = photo->infoheader->width;
	int idx;
	int idx_i, idx_j, idx_max, idx_min;
	
	if(type == 'M')	
		for(i = 0; i < height; i++){
			for(j = 0; j < width; j++){
				idx = i * width + j;
				max = photo->bitmap[idx];
				for(k = 0; k < N; k++){
					for(m = 0; m < N; m++){
						//verifica daca este vecin valid
						idx_i = i - N / 2 + k;
						idx_j = j - N / 2 + m;
						idx_max = idx_i * width + idx_j;
						if(!(idx_i < 0 || idx_j < 0 || 
							idx_i >= height || idx_j >= width)){
							if(photo->bitmap[idx_max].blue > max.blue)
								max.blue = photo->bitmap[idx_max].blue;
							if(photo->bitmap[idx_max].green > max.green)
								max.green = photo->bitmap[idx_max].green;
							if(photo->bitmap[idx_max].red > max.red)
								max.red = photo->bitmap[idx_max].red;
						}
					}
				}
				new_photo->bitmap[idx].blue = max.blue;
				new_photo->bitmap[idx].green = max.green;
				new_photo->bitmap[idx].red = max.red;
			}
		}
	else {
		for(i = 0; i < height; i++){
			for(j = 0; j < width; j++){
				idx = i * width + j;
				min = photo->bitmap[idx];
				for(k = 0; k < N; k++){
					for(m = 0; m < N; m++){
						//verifica daca este vecin valid
						idx_i = i - N / 2 + k;
						idx_j = j - N / 2 + m;
						idx_min = idx_i * width + idx_j;
						if(!(idx_i < 0 || idx_j < 0 || 
							idx_i >= height || idx_j >= width)){
							if(photo->bitmap[idx_min].blue < min.blue)
								min.blue = photo->bitmap[idx_min].blue;
							if(photo->bitmap[idx_min].green < min.green)
								min.green = photo->bitmap[idx_min].green;
							if(photo->bitmap[idx_min].red < min.red)
								min.red = photo->bitmap[idx_min].red;
						}
					}
				}
				new_photo->bitmap[idx].blue = min.blue;
				new_photo->bitmap[idx].green = min.green;
				new_photo->bitmap[idx].red = min.red;
			}
		}
	}
}

int main(int argc, char *argv[]){

    // alocare memorie pentru argumentele de rulare
    params *param = alloc_params();

    // se citesc argumentele programului
	get_args(argc, argv, param);

    bmp_photo *photo = alloc_photo();

    //citirea pozei
    read_photo(param, photo);

    bmp_photo *new_photo = alloc_photo();
    new_photo->bitmap = alloc_bitmap(photo->infoheader->width, photo->infoheader->height);
    memcpy(new_photo->header, photo->header, sizeof(bmp_fileheader));
    memcpy(new_photo->infoheader, photo->infoheader, sizeof(bmp_infoheader));

	double time_taken;
	time_taken = omp_get_wtime();

   	pooling(photo, new_photo, 'M', 5);
	
	time_taken = omp_get_wtime() - time_taken;
    printf("Algorithm took %f seconds to execute\n", time_taken);
 
    //scrierea pozei
    write_photo(param, new_photo);

    // free memory
    free_params(param);
    free_photo(new_photo);
    free_photo(photo);
    return 0;
}
