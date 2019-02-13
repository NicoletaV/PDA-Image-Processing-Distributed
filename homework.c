/* VINA NICOLETA 335CC */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

typedef struct {
	unsigned char r, g, b;
}color;

typedef struct {
	char *P56;
	int width, height, maxval;
	color **color_picture;
	unsigned char **grayscale_picture;
}image;

/* functie ce genereaza matricea-filtru pentru numele
primit la argument */
float** getFilter(char *name) {

	int i, j;

	float **m = malloc(3 * sizeof(float *));
	if(!m) {
		exit(1);
	}

	for(i = 0; i < 3; i++) {
		m[i] = calloc(3, sizeof(float));
		if(!m[i]) {
			exit(1);
		}
	}

	if (strcmp(name, "smooth") == 0) {
		
		float k = (float)1 / 9;
		
		for(i = 0; i < 3; i++) {
			for(j = 0; j < 3; j++) {				
				m[i][j] = (float)k;
			}
		}

		return m;
	}

	if (strcmp(name, "blur") == 0) {

		float k = (float)1 / 16;

		for(i = 0; i < 3; i += 2) {
			m[i][0] = (float)(k * 1);
			m[i][1] = (float)(k * 2);
			m[i][2] = (float)(k * 1);
		}

		m[1][0] = (float)(k * 2);
		m[1][1] = (float)(k * 4);
		m[1][2] = (float)(k * 2);

		return m;
	}

	if (strcmp(name, "sharpen") == 0) {

		float k = (float)1 / 3;

		for(i = 0; i < 3; i += 2) {
			m[i][0] = (float)(k * 0);
			m[i][1] = (float)(k * (-2));
			m[i][2] = (float)(k * 0);
		}

		m[1][0] = (float)(k * (-2));
		m[1][1] = (float)(k * 11);
		m[1][2] = (float)(k * (-2));

		return m;
	}

	if (strcmp(name, "mean") == 0) {

		for(i = 0; i < 3; i++) {
			for(j = 0; j < 3; j++) {
				m[i][j] = -1.0f;
			}
		}
		m[1][1] = 9.0f;

		return m;
	}

	if (strcmp(name, "emboss") == 0) {

		for(i = 0; i < 3; i++) {
			m[i][0] = 0.0f;
			m[i][2] = 0.0f;
		}
		m[0][1] = 1.0f;
		m[1][1] = 0.0f;
		m[2][1] = -1.0f;

		return m;
	}	

	return m;
}

/* se citeste inputul in matricea corespunzatoare (color_picture
sau grayscale_picture) */
void readInput(const char *fileName, image *img) {

	FILE* fin = fopen(fileName, "r");
	if(!fin) {
		exit(1);
	}

	/* se citesc in campurile structurii de imagine: tipul acesteia, width,
	height si maxval din fisierul de intrare */

	img->P56 = (char*)calloc(5, sizeof(char));
	if(!img) {
		exit(1);
	}
	fscanf(fin, "%s", img->P56);
	
	fscanf(fin, "%d", &img->width);
	fscanf(fin, "%d", &img->height);
	fscanf(fin, "%d\n", &img->maxval);
	
	unsigned int i;

	/* daca incepe cu P6, este imagine color, se aloca memorie pt matricea de
	pixeli color_picture si se citeste in aceasta din fisier, linie cu linie */

	if(strcmp(img->P56, "P6") == 0) {

		img->grayscale_picture = NULL;
		
		img->color_picture = malloc(img->height * sizeof(color *));
		if(!img->color_picture) {
			exit(1);
		}

		for(i = 0; i < img->height; i++) {
			img->color_picture[i] = calloc(img->width, sizeof(color));
			if(!img->color_picture[i]) {
				exit(1);
			}
		}

		for(i = 0; i < img->height; i++) {
			fread(img->color_picture[i], sizeof(color), img->width, fin);
		}

	/* daca incepe cu P5, este imagine grayscale, se aloca memorie pt matricea
	de pixeli grayscale_picture si se citeste in aceasta din fisier, linie cu
	linie (in fread) */

	} else if(strcmp(img->P56, "P5") == 0){

		img->color_picture = NULL;
		
		img->grayscale_picture = malloc(img->height * sizeof(unsigned char *));
		if(!img->grayscale_picture) {
			exit(1);
		}
		
		for(i = 0; i < img->height; i++) {
			img->grayscale_picture[i] = calloc(img->width, sizeof(unsigned char));
			if(!img->grayscale_picture[i]) {
				exit(1);
			}
		}

		for(i = 0; i < img->height; i++) {
			fread(img->grayscale_picture[i], sizeof(unsigned char), img->width, fin);
		}

	} else {
		exit(1);
	}

	fclose(fin);
}

/* se scrie matricea corespunzatoare (color_picture
sau grayscale_picture) in fisierul de iesire */
void writeData(const char *fileName, image *img) {

	FILE* fout = fopen(fileName, "w");
	if(!fout) {
		exit(1);
	}

	/* se scriu in fisierul de iesire: tipul imaginii, width, height si maxval */

	fprintf(fout, "%s\n", img->P56);
	fprintf(fout, "%d %d\n", img->width, img->height);
	fprintf(fout, "%d\n", img->maxval);
	
	unsigned int i;

	/* se scriu matricile de pixeli cu fwrite, linie cu linie */

	if(strcmp(img->P56, "P6") == 0) {

		for(i = 0; i < img->height; i++) {
			fwrite(img->color_picture[i], sizeof(color), img->width, fout);
		}

	} else if(strcmp(img->P56, "P5") == 0) {

		for(i = 0; i < img->height; i++) {
			fwrite(img->grayscale_picture[i], sizeof(unsigned char), img->width, fout);
		}

	} else {
		exit(1);
	}

	fclose(fout);

	/* se elibereaza memoria alocata pentru matricile de pixeli si tipul imaginii
	la final, dupa ce s-a incheiat scrierea in fisier si acesta s-a inchis */

	if(strcmp(img->P56, "P6") == 0) {
		
		for(i = 0; i < img->height; i++) {
			free(img->color_picture[i]);
		}
		free(img->color_picture);

	} else if(strcmp(img->P56, "P5") == 0) {
		
		for(i = 0; i < img->height; i++) {
			free(img->grayscale_picture[i]);
		}
		free(img->grayscale_picture);
	}

	free(img->P56);
}

int main(int argc, char *argv[]) {

	char P;
	image input;
	int lines_per_process, lines_last_process, height, width, k;	
	float **matrix;

	int rank, nProcesses;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);

	if(nProcesses > 1) {

		int sendcounts[nProcesses];
		int displs[nProcesses];
		int sendcounts2[nProcesses];
		int displs2[nProcesses];

		/* procesul 0 citeste matricea si face impartirea pe linii pt fiecare proces;
		formeaza vectorii sendcounts, sendcounts2, displs, displs2 folositi la
		Scatterv si Gatherv */
		if(rank == 0) {

			readInput(argv[1], &input);

			lines_per_process = input.height / nProcesses;
			lines_last_process = lines_per_process +
								(input.height - nProcesses * lines_per_process);

			P = input.P56[1];
			width = input.width;
			height = input.height;

			/* daca e imagine color, latimea va fi tripla */
			if(P == '6') {
				width *= 3;
			}			

			int offset = 0;
			sendcounts[0] = (lines_per_process + 1) * width;
			displs[0] = offset;
			offset += (lines_per_process - 1) * width;
			for(int i = 1; i < nProcesses - 1; i++) {
				sendcounts[i] = (lines_per_process + 2) * width;
				displs[i] = offset;
				offset += lines_per_process * width;
			}
			sendcounts[nProcesses - 1] = (lines_last_process + 1) * width;
			displs[nProcesses - 1] = offset;

			int offset2 = 0;
			for(int i = 0; i < nProcesses - 1; i++) {
				sendcounts2[i] = lines_per_process * width;
				displs2[i] = offset2;
				offset2 += lines_per_process * width;
			}
			sendcounts2[nProcesses - 1] = lines_last_process * width;
			displs2[nProcesses - 1] = offset2;
			
		}

		/* toate procesele au nevoie sa cunoasca sendcounts, sendcounts2, displs,
		displs2, P - tipul imaginii, height si width */
		MPI_Bcast(&P, 1, MPI_CHAR, 0, MPI_COMM_WORLD);
		MPI_Bcast(&sendcounts, nProcesses, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(&displs, nProcesses, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(&width, 1, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(&height, 1, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(&sendcounts2, nProcesses, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(&displs2, nProcesses, MPI_INT, 0, MPI_COMM_WORLD);

		unsigned char buf[height * width];
		unsigned char buffer[height * width];
		unsigned char recvbuf[sendcounts[rank]];
		unsigned char recvbuf2[sendcounts2[rank]];

		/* in rankul 0 se copiaza matricea de input in bufferul "buf" ce se va
		trimite in Scatterv */
		if(rank == 0) {	

			if(input.grayscale_picture) {
				int jj = 0;
				for(int i = 0; i < input.height; i++) {
					int j;
					for(j = 0; j < input.width; j++) {
						buf[jj + j] = input.grayscale_picture[i][j];
					}
					jj += j;
				}	
			}

			if(input.color_picture) {
				int jjj = 0;
				for(int i = 0; i < input.height; i++) {
					int j;
					for(j = 0; j < input.width; j++) {
						buf[jjj + 3 * j] = input.color_picture[i][j].r;
						buf[jjj + 3 * j + 1] = input.color_picture[i][j].g;
						buf[jjj + 3 * j + 2] = input.color_picture[i][j].b;
					}
					jjj += 3 * j;
				}	
			}
		}

		/* for-ul pentru filtre */
		for(k = 3; k < argc; k++) {

			/* se trimite spre impartire bufferul buf, in functie de sendcounts
			si displs si fiecare proces isi primeste partea in recvbuf */
			MPI_Scatterv(&buf, sendcounts, displs, MPI_UNSIGNED_CHAR, &recvbuf,
						sendcounts[rank], MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

			if(rank == 0) {

				matrix = getFilter(argv[k]);

				if(P == '5') {

					for(int i = 0; i < sendcounts[rank] - width; i++) {

						if(i < width) {
							recvbuf2[i] = recvbuf[i];
						} else {

							if((i % width == 0) || ((i + 1) % width == 0)) {
								recvbuf2[i] = recvbuf[i];
							} else {

								float sum = (float)0;
								sum += (float)(matrix[0][0] * (float)recvbuf[i - width - 1]);
								sum += (float)(matrix[0][1] * (float)recvbuf[i - width]);
								sum += (float)(matrix[0][2] * (float)recvbuf[i - width + 1]);
								sum += (float)(matrix[1][0] * (float)recvbuf[i - 1]);
								sum += (float)(matrix[1][1] * (float)recvbuf[i]);
								sum += (float)(matrix[1][2] * (float)recvbuf[i + 1]);
								sum += (float)(matrix[2][0] * (float)recvbuf[i + width - 1]);
								sum += (float)(matrix[2][1] * (float)recvbuf[i + width]);
								sum += (float)(matrix[2][2] * (float)recvbuf[i + width + 1]);

								recvbuf2[i] = (unsigned char)sum;
							}
						}
					}
				} else if(P == '6') {

					for(int i = 0; i < sendcounts[rank] - width; i++) {

						if(i < width) {
							recvbuf2[i] = recvbuf[i];
						} else {

							if(i % width == 0 || (i - 1) % width == 0 ||
								(i - 2) % width == 0 || (i + 1) % width == 0 ||
								(i + 2) % width == 0 || (i + 3) % width == 0) {

								recvbuf2[i] = recvbuf[i];
							} else {

								float sum = (float)0;
								sum += (float)(matrix[0][0] * (float)recvbuf[i - width - 3]);
								sum += (float)(matrix[0][1] * (float)recvbuf[i - width]);
								sum += (float)(matrix[0][2] * (float)recvbuf[i - width + 3]);
								sum += (float)(matrix[1][0] * (float)recvbuf[i - 3]);
								sum += (float)(matrix[1][1] * (float)recvbuf[i]);
								sum += (float)(matrix[1][2] * (float)recvbuf[i + 3]);
								sum += (float)(matrix[2][0] * (float)recvbuf[i + width - 3]);
								sum += (float)(matrix[2][1] * (float)recvbuf[i + width]);
								sum += (float)(matrix[2][2] * (float)recvbuf[i + width + 3]);

								recvbuf2[i] = (unsigned char)sum;
							}
						}
					}

				} else {
					exit(1);
				}	
			} 

			/* fiecare proces aplica filtrul */
			if(rank != 0 && rank != nProcesses - 1) {

				matrix = getFilter(argv[k]);

				if(P == '5') {
				
					for(int i = width; i < sendcounts[rank] - width; i++) {

						if((i % width == 0) || ((i + 1) % width == 0)) {
								recvbuf2[i - width] = recvbuf[i];
						} else {

							float sum = (float)0;
							sum += (float)(matrix[0][0] * (float)recvbuf[i - width - 1]);
							sum += (float)(matrix[0][1] * (float)recvbuf[i - width]);
							sum += (float)(matrix[0][2] * (float)recvbuf[i - width + 1]);
							sum += (float)(matrix[1][0] * (float)recvbuf[i - 1]);
							sum += (float)(matrix[1][1] * (float)recvbuf[i]);
							sum += (float)(matrix[1][2] * (float)recvbuf[i + 1]);
							sum += (float)(matrix[2][0] * (float)recvbuf[i + width - 1]);
							sum += (float)(matrix[2][1] * (float)recvbuf[i + width]);
							sum += (float)(matrix[2][2] * (float)recvbuf[i + width + 1]);

							recvbuf2[i - width] = (unsigned char)sum;
						}
					}

				} else if(P == '6') {

					for(int i = width; i < sendcounts[rank] - width; i++) {

						if(i % width == 0 || (i - 1) % width == 0 ||
							(i - 2) % width == 0 || (i + 1) % width == 0 ||
							(i + 2) % width == 0 || (i + 3) % width == 0) {

								recvbuf2[i - width] = recvbuf[i];
						} else {

							float sum = (float)0;
							sum += (float)(matrix[0][0] * (float)recvbuf[i - width - 3]);
							sum += (float)(matrix[0][1] * (float)recvbuf[i - width]);
							sum += (float)(matrix[0][2] * (float)recvbuf[i - width + 3]);
							sum += (float)(matrix[1][0] * (float)recvbuf[i - 3]);
							sum += (float)(matrix[1][1] * (float)recvbuf[i]);
							sum += (float)(matrix[1][2] * (float)recvbuf[i + 3]);
							sum += (float)(matrix[2][0] * (float)recvbuf[i + width - 3]);
							sum += (float)(matrix[2][1] * (float)recvbuf[i + width]);
							sum += (float)(matrix[2][2] * (float)recvbuf[i + width + 3]);

							recvbuf2[i - width] = (unsigned char)sum;
						}
					}

				} else {
					exit(1);
				}		
			}

			if(rank == nProcesses - 1) {

				matrix = getFilter(argv[k]);

				if(P == '5') {

					for(int i = width; i < sendcounts[rank] - width; i++) {

						if((i % width == 0) || ((i + 1) % width == 0)) {
							recvbuf2[i - width] = recvbuf[i];
						} else {

							float sum = (float)0;
							sum += (float)(matrix[0][0] * (float)recvbuf[i - width - 1]);
							sum += (float)(matrix[0][1] * (float)recvbuf[i - width]);
							sum += (float)(matrix[0][2] * (float)recvbuf[i - width + 1]);
							sum += (float)(matrix[1][0] * (float)recvbuf[i - 1]);
							sum += (float)(matrix[1][1] * (float)recvbuf[i]);
							sum += (float)(matrix[1][2] * (float)recvbuf[i + 1]);
							sum += (float)(matrix[2][0] * (float)recvbuf[i + width - 1]);
							sum += (float)(matrix[2][1] * (float)recvbuf[i + width]);
							sum += (float)(matrix[2][2] * (float)recvbuf[i + width + 1]);

							recvbuf2[i - width] = (unsigned char)sum;
						}
					}

					for(int i = sendcounts[rank] - width; i < sendcounts[rank]; i++) {
						recvbuf2[i - width] = recvbuf[i];
					}

				} else if(P == '6') {

					for(int i = width; i < sendcounts[rank] - width; i++) {

						if(i % width == 0 || (i - 1) % width == 0 ||
							(i - 2) % width == 0 || (i + 1) % width == 0 ||
							(i + 2) % width == 0 || (i + 3) % width == 0) {

							recvbuf2[i - width] = recvbuf[i];
						} else {

							float sum = (float)0;
							sum += (float)(matrix[0][0] * (float)recvbuf[i - width - 3]);
							sum += (float)(matrix[0][1] * (float)recvbuf[i - width]);
							sum += (float)(matrix[0][2] * (float)recvbuf[i - width + 3]);
							sum += (float)(matrix[1][0] * (float)recvbuf[i - 3]);
							sum += (float)(matrix[1][1] * (float)recvbuf[i]);
							sum += (float)(matrix[1][2] * (float)recvbuf[i + 3]);
							sum += (float)(matrix[2][0] * (float)recvbuf[i + width - 3]);
							sum += (float)(matrix[2][1] * (float)recvbuf[i + width]);
							sum += (float)(matrix[2][2] * (float)recvbuf[i + width + 3]);

							recvbuf2[i - width] = (unsigned char)sum;
						}
					}

					for(int i = sendcounts[rank] - width; i < sendcounts[rank]; i++) {
						recvbuf2[i - width] = recvbuf[i];
					}

				} else {
					exit(1);
				}
			}

			/* se colecteaza toate partile prelucrate de procese (in vectorul recvbuf2)
			in procesul 0, in bufferul "buffer", dar fara liniile in plus, lucru
			controlat de sendcounts2 si displs2 */
			MPI_Gatherv(&recvbuf2, sendcounts2[rank], MPI_UNSIGNED_CHAR, &buffer,
						sendcounts2, displs2, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

			/* la sfarsitul aplicarii unui filtru, se copiaza rezultatul in buf,
			pentru a se putea continua si aplica urmatorul filtru */
			if(rank == 0) {

				for(int i = 0; i < height * width; i++) {
					buf[i] = buffer[i];
				}
			}

			MPI_Barrier(MPI_COMM_WORLD);
		}
		
		/* are loc copierea din vector inapoi in matrice si apoi scrierea ei in fisier */
		if(rank == 0) {

			if(P == '6') {
				width /= 3;
			}

			if(input.grayscale_picture) {
				int jj = 0, j;
				for(int i = 0; i < height; i++) {				
					for(j = 0; j < width; j++) {
						input.grayscale_picture[i][j] = buffer[jj + j];
					}
					jj += j;
				}
			}

			if(input.color_picture) {
				int jjj = 0, j;
				for(int i = 0; i < height; i++) {
					for(j = 0; j < width; j++) {
						input.color_picture[i][j].r = buffer[jjj + 3 * j];
						input.color_picture[i][j].g = buffer[jjj + 3 * j + 1];
						input.color_picture[i][j].b = buffer[jjj + 3 * j + 2];
					}
					jjj += 3 * j;
				}
			}

			writeData(argv[2], &input);
		}
	} else {

		/* in cazul in care insa exista un singur proces, matricea nu se mai imparte,
		ci se aplica direct filtrele pe ea prin intermediul unei matrice auxiliare
		si la final, se scrie in fisier */

		if(rank == 0) {

			readInput(argv[1], &input);
			P = input.P56[1];
		}

		if(rank == 0) {

			for(k = 3; k < argc; k++) {

				matrix = getFilter(argv[k]);			

				if(P == '5') {

					unsigned char aux5[input.height][input.width];
					for(int i = 0; i < input.height; i++) {
						for(int j = 0; j < input.width; j++) {
							aux5[i][j] = input.grayscale_picture[i][j];
						}
					}

					for(int i = 1; i < input.height - 1; i++) {
						for(int j = 1; j < input.width - 1; j++) {

							float sum = (float)0;
							sum += (float)(matrix[0][0] * (float)aux5[i - 1][j - 1]);
							sum += (float)(matrix[0][1] * (float)aux5[i - 1][j]);
							sum += (float)(matrix[0][2] * (float)aux5[i - 1][j + 1]);
							sum += (float)(matrix[1][0] * (float)aux5[i][j - 1]);
							sum += (float)(matrix[1][1] * (float)aux5[i][j]);
							sum += (float)(matrix[1][2] * (float)aux5[i][j + 1]);
							sum += (float)(matrix[2][0] * (float)aux5[i + 1][j - 1]);
							sum += (float)(matrix[2][1] * (float)aux5[i + 1][j]);
							sum += (float)(matrix[2][2] * (float)aux5[i + 1][j + 1]);

							input.grayscale_picture[i][j] = (unsigned char)sum;
						}
					}

				} else if(P == '6') {

					color aux6[input.height][input.width];
					for(int i = 0; i < input.height; i++) {
						for(int j = 0; j < input.width; j++) {
							aux6[i][j].r = input.color_picture[i][j].r;
							aux6[i][j].g = input.color_picture[i][j].g;
							aux6[i][j].b = input.color_picture[i][j].b;
						}
					}

					for(int i = 1; i < input.height - 1; i++) {
						for(int j = 1; j < input.width - 1; j++) {

							float sum_r = (float)0;
							sum_r += (float)(matrix[0][0] * (float)aux6[i - 1][j - 1].r);
							sum_r += (float)(matrix[0][1] * (float)aux6[i - 1][j].r);
							sum_r += (float)(matrix[0][2] * (float)aux6[i - 1][j + 1].r);
							sum_r += (float)(matrix[1][0] * (float)aux6[i][j - 1].r);
							sum_r += (float)(matrix[1][1] * (float)aux6[i][j].r);
							sum_r += (float)(matrix[1][2] * (float)aux6[i][j + 1].r);
							sum_r += (float)(matrix[2][0] * (float)aux6[i + 1][j - 1].r);
							sum_r += (float)(matrix[2][1] * (float)aux6[i + 1][j].r);
							sum_r += (float)(matrix[2][2] * (float)aux6[i + 1][j + 1].r);

							input.color_picture[i][j].r = (unsigned char)sum_r;

							float sum_g = (float)0;
							sum_g += (float)(matrix[0][0] * (float)aux6[i - 1][j - 1].g);
							sum_g += (float)(matrix[0][1] * (float)aux6[i - 1][j].g);
							sum_g += (float)(matrix[0][2] * (float)aux6[i - 1][j + 1].g);
							sum_g += (float)(matrix[1][0] * (float)aux6[i][j - 1].g);
							sum_g += (float)(matrix[1][1] * (float)aux6[i][j].g);
							sum_g += (float)(matrix[1][2] * (float)aux6[i][j + 1].g);
							sum_g += (float)(matrix[2][0] * (float)aux6[i + 1][j - 1].g);
							sum_g += (float)(matrix[2][1] * (float)aux6[i + 1][j].g);
							sum_g += (float)(matrix[2][2] * (float)aux6[i + 1][j + 1].g);

							input.color_picture[i][j].g = (unsigned char)sum_g;

							float sum_b = (float)0;
							sum_b += (float)(matrix[0][0] * (float)aux6[i - 1][j - 1].b);
							sum_b += (float)(matrix[0][1] * (float)aux6[i - 1][j].b);
							sum_b += (float)(matrix[0][2] * (float)aux6[i - 1][j + 1].b);
							sum_b += (float)(matrix[1][0] * (float)aux6[i][j - 1].b);
							sum_b += (float)(matrix[1][1] * (float)aux6[i][j].b);
							sum_b += (float)(matrix[1][2] * (float)aux6[i][j + 1].b);
							sum_b += (float)(matrix[2][0] * (float)aux6[i + 1][j - 1].b);
							sum_b += (float)(matrix[2][1] * (float)aux6[i + 1][j].b);
							sum_b += (float)(matrix[2][2] * (float)aux6[i + 1][j + 1].b);

							input.color_picture[i][j].b = (unsigned char)sum_b;
						}
					}

				} else {
					exit(1);
				}
			}		
		}

		if(rank == 0) {
			writeData(argv[2], &input);
		}
	}

	MPI_Finalize();
	return 0;
}