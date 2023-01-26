#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "util.h"
#include <mpi.h>

#define ROOT 0

void pooling(RGB *bitmap, RGB *new_bitmap, char type, int start, int end, int width,int height, int N){

	int i,j,k,m;
	RGB min, max;
	int idx;
	int idx_i, idx_j, idx_max, idx_min;
	int index_new = 0;
	
	if(type == 'M')	
		for(i = start; i < end; i++){
			for(j = 0; j < width; j++){
				idx = i * width + j;
				max = bitmap[idx];
				for(k = 0; k < N; k++){
					for(m = 0; m < N; m++){
						//verifica daca este vecin valid
						idx_i = i - N / 2 + k;
						idx_j = j - N / 2 + m;
						idx_max = idx_i * width + idx_j;
						if(!(idx_i < 0 || idx_j < 0 || 
							idx_i >= height || idx_j >= width)){
							if(bitmap[idx_max].blue > max.blue)
								max.blue = bitmap[idx_max].blue;
							if(bitmap[idx_max].green > max.green)
								max.green = bitmap[idx_max].green;
							if(bitmap[idx_max].red > max.red)
								max.red = bitmap[idx_max].red;
						}
					}
				}
				new_bitmap[index_new].blue = max.blue;
				new_bitmap[index_new].green = max.green;
				new_bitmap[index_new].red = max.red;
				index_new++;
			}
		}
	else {
		for(i = start; i < end; i++){
			for(j = 0; j < width; j++){
				idx = i * width + j;
				min = bitmap[idx];
				for(k = 0; k < N; k++){
					for(m = 0; m < N; m++){
						//verifica daca este vecin valid
						idx_i = i - N / 2 + k;
						idx_j = j - N / 2 + m;
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

				new_bitmap[index_new].blue = min.blue;
				new_bitmap[index_new].green = min.green;
				new_bitmap[index_new].red = min.red;
				index_new++;
			}
		}
	}
}

int main(int argc, char *argv[]){

	double ta,tt;
	int rank, numtasks;
	int height, width;

	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
	MPI_Comm_rank(MPI_COMM_WORLD,&rank);


	if(rank == ROOT) {
		// alocare memorie pentru argumentele de rulare
		params *param = alloc_params();

		// se citesc argumentele programului
		get_args(argc, argv, param);

		bmp_photo *photo = alloc_photo();

		//citirea pozei
		read_photo(param, photo);

		//trimitem dimensiunile pozei
        width = photo->infoheader->width;
        height = photo->infoheader->height;
		ta = MPI_Wtime();

		for(int i = 1; i < numtasks; i++){
            MPI_Send(&width, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(&height, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(photo->bitmap, width * height * sizeof(RGB), MPI_CHAR, i, 0, MPI_COMM_WORLD);
        }

        int total_size = height;
        int i_start = rank * (double)total_size / numtasks;
        int i_end = MIN((rank + 1) * (double)total_size / numtasks, total_size);

        int task_size = (i_end - i_start) * width;

        RGB *new_bitmap = (RGB *)calloc(task_size, sizeof(RGB));;
        if(new_bitmap == NULL){
            printf("Eroare la alocare");
            exit(1);
        }

       
	    pooling(photo->bitmap, new_bitmap, 'M', i_start, i_end, width, height, 5);


        memcpy(photo->bitmap + i_start, new_bitmap, task_size * sizeof(RGB));


		MPI_Status status;
        for(int i = 1; i < numtasks; i++){
            i_start = i * (double)total_size / numtasks;
            i_end = MIN((i + 1) * (double)total_size / numtasks, total_size);
            task_size = (i_end - i_start) * width;

            MPI_Recv(photo->bitmap + (i_start * width), task_size * sizeof(RGB), MPI_CHAR, i, 0, MPI_COMM_WORLD, &status);
        }

		tt = MPI_Wtime();

        //scrierea pozei
        write_photo(param, photo);

        // free memory
        free_params(param);
        free_photo(photo);   
        free(new_bitmap);
        
        printf("Algorithm took %f seconds to execute\n", tt-ta);

	}else{
		MPI_Status status;
		RGB *bitmap;

		//primim dimensiunile pozei
		MPI_Recv(&width, 1 , MPI_INT, 0 ,0, MPI_COMM_WORLD, &status);
		MPI_Recv(&height, 1 , MPI_INT, 0 ,0, MPI_COMM_WORLD, &status);

		bitmap = (RGB *)calloc(height * width, sizeof(RGB));
        if(bitmap == NULL){
            printf("Eroare la alocare");
            exit(1);
        }

        MPI_Recv(bitmap, width * height * sizeof(RGB), MPI_CHAR, 0, 0, MPI_COMM_WORLD, &status);

        int total_size = height;
        int i_start = rank * (double)total_size / numtasks;
        int i_end = MIN((rank + 1) * (double)total_size / numtasks, total_size);

        int task_size = (i_end - i_start) * width;
        
        RGB *new_bitmap = (RGB *)calloc(task_size, sizeof(RGB));;
        if(new_bitmap == NULL){
            printf("Eroare la alocare");
            exit(1);
        }

	    pooling(bitmap, new_bitmap, 'M', i_start, i_end, width, height, 5);

		MPI_Send(new_bitmap, task_size * sizeof(RGB), MPI_CHAR, 0, 0, MPI_COMM_WORLD);

        free(bitmap);
        free(new_bitmap);

	}



    MPI_Finalize();
    return 0;
}
