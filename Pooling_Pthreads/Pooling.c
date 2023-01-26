#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "util.h"
#include <pthread.h>
#include <omp.h>

#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

typedef struct thread_args {
    params *param;
   	bmp_photo *photo;
    bmp_photo *new_photo;
    int id;
	int N;
} thread_args;


void *thread_function_max(void *arg) {
	struct thread_args *args = (struct thread_args *) arg;
    int P = args->param->threads_number;
    int total_size = args->photo->infoheader->height;
    int start = args->id * (double)total_size / P;
    int end = MIN((args->id + 1) * (double)total_size / P, total_size);

	int i,j,k,m;
	RGB max;
    int height = args->photo->infoheader->height;
    int width = args->photo->infoheader->width;
	int idx;
	int idx_i, idx_j, idx_max;
	int N = args->N;

	for(i = start; i < end; i++){
		for(j = 0; j < width; j++){
			idx = i * width + j;
			max = args->photo->bitmap[idx];
			for(k = 0; k < N; k++){
				for(m = 0; m < N; m++){
					//verifica daca este vecin valid
					idx_i = i - N / 2 + k;
					idx_j = j - N / 2 + m;
					idx_max = idx_i * width + idx_j;
					if(!(idx_i < 0 || idx_j < 0 || 
						idx_i >= height || idx_j >= width)){
						if(args->photo->bitmap[idx_max].blue > max.blue)
							max.blue = args->photo->bitmap[idx_max].blue;
						if(args->photo->bitmap[idx_max].green > max.green)
							max.green = args->photo->bitmap[idx_max].green;
						if(args->photo->bitmap[idx_max].red > max.red)
							max.red = args->photo->bitmap[idx_max].red;
					}
				}
			}
			args->new_photo->bitmap[idx].blue = max.blue;
			args->new_photo->bitmap[idx].green = max.green;
			args->new_photo->bitmap[idx].red = max.red;
		}
	}

	pthread_exit(NULL);
}

void *thread_function_min(void *arg) {
	struct thread_args *args = (struct thread_args *) arg;
    int P = args->param->threads_number;
    int total_size = args->photo->infoheader->height;
    int start = args->id * (double)total_size / P;
    int end = MIN((args->id + 1) * (double)total_size / P, total_size);

	int i,j,k,m;
	RGB min;
    int height = args->photo->infoheader->height;
    int width = args->photo->infoheader->width;
	int idx;
	int idx_i, idx_j, idx_min;
	int N = args->N;

	for(i = start; i < end; i++){
		for(j = 0; j < width; j++){
			idx = i * width + j;
			min = args->photo->bitmap[idx];
			for(k = 0; k < N; k++){
				for(m = 0; m < N; m++){
					//verifica daca este vecin valid
					idx_i = i - N / 2 + k;
					idx_j = j - N / 2 + m;
					idx_min = idx_i * width + idx_j;
					if(!(idx_i < 0 || idx_j < 0 || 
						idx_i >= height || idx_j >= width)){
						if(args->photo->bitmap[idx_min].blue < min.blue)
							min.blue = args->photo->bitmap[idx_min].blue;
						if(args->photo->bitmap[idx_min].green < min.green)
							min.green = args->photo->bitmap[idx_min].green;
						if(args->photo->bitmap[idx_min].red < min.red)
							min.red = args->photo->bitmap[idx_min].red;
					}
				}
			}
			args->new_photo->bitmap[idx].blue = min.blue;
			args->new_photo->bitmap[idx].green = min.green;
			args->new_photo->bitmap[idx].red = min.red;
		}
	}

	pthread_exit(NULL);
}

void pooling(bmp_photo *photo, bmp_photo *new_photo, char type, int N, params *param){

	int P = param->threads_number;

	pthread_t threads[P];
	int r;
	void *status;

	thread_args **args = (thread_args **)calloc(P, sizeof(thread_args *));
	if (args == NULL) {
		printf("Eroare la alocare\n");
		exit(1);
	}

	for(int id = 0; id < P; id++){
		args[id] = (thread_args *)calloc(1, sizeof(thread_args));
		if (args[id] == NULL) {
			printf("Eroare la alocare\n");
			exit(1);
		}

		args[id]->id = id;
		args[id]->N = N;
		args[id]->photo = photo;
		args[id]->new_photo = new_photo;
		args[id]->param = param;

		if(type == 'M') {
			r = pthread_create(&threads[id], NULL, thread_function_max, args[id]);
		} else {
			r = pthread_create(&threads[id], NULL, thread_function_min, args[id]);
		}
		
		if (r) {
			printf("Eroare la crearea thread-ului %d\n", id);
			exit(-1);
		}
	}

	for (int id = 0; id < P; id++) {
		r = pthread_join(threads[id], &status);

		free(args[id]);
		if (r) {
			printf("Eroare la asteptarea thread-ului %d\n", id);
			exit(-1);
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

   	pooling(photo, new_photo, 'M', 5, param);
	
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
