#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

// Structura pentru a retine startul pt fiecare proces si numarul de linii.
typedef struct {
  int start;
  int number_lines;
} elements;

// Structura pentru a retine fiecare culoare a pixelului.
typedef struct {
  unsigned char red;
  unsigned char green;
  unsigned char blue;
}rgb;

// Structura pentru a retine datele unei imagini.
typedef struct {
  char type[2];
  int width;
  int height;
  int maxval;
  unsigned char** pixels_bw;  // Folosit pentru imaginile alb-negru.
  rgb** pixels; // Folosit pentru imaginile color.
}image;

// Functie pentru citit datele imaginii primite ca parametru.
void readInput(const char * fileName, image *img) {
  FILE *f = fopen(fileName, "r");
  if(f == NULL) {
    exit(1);
  }

  // Se citesc tipul imaginii, latimea, lungimea si valoarea maxima.
  fscanf(f,"%c%c\n", &(img->type[0]), &(img->type[1]));
  fscanf(f,"%d %d\n", &(img->width), &(img->height));
  fscanf(f,"%d\n", &(img->maxval));

  int height = img->height;
  int width = img->width;
  int i = 0;

  // Se aloca memorie pentru matricea pixelilor.
  // Cazul cand imaginea este de tipul "P5" (alb sau negru)
  if(img->type[1] == '5') {
    img->pixels_bw = (unsigned char**)malloc(height * sizeof(unsigned char*));
    if(img->pixels_bw == NULL) {
      exit(2);
    }

    for (i = 0; i < height; i++) {
      img->pixels_bw[i] = (unsigned char*)malloc(width  * sizeof(unsigned char));
      if(img->pixels_bw[i] == NULL) {
        exit(2);
      }
    }

    for (i = 0; i < height; i++) {
      fread(img->pixels_bw[i], 1, width, f);
    }

    // Cazul cand imaginea este de tipul "P6" (color)
  } else {
    img->pixels = (rgb**)malloc(height * sizeof(rgb*));
    if(img->pixels == NULL) {
      exit(2);
    }

    for (i = 0; i < height; i++) {
      img->pixels[i] = (rgb*)malloc(width  * sizeof(rgb));
      if(img->pixels[i] == NULL) {
        exit(2);
      }
    }

    for (i = 0; i < height; i++) {
      fread(img->pixels[i], 1, 3 * width, f);
    }
  }

  fclose(f);
}

// Functie pentru eliberarea memoriei cand poza este alb-negru.
void free_memory_BW(image* img) {
  int i = 0;
  int height = img->height;
  for(i = 0; i < height; i++) {
    free(img->pixels_bw[i]);
  }
  free(img->pixels_bw);
}

// Functie pentru eliberarea memoriei cand poza este color.
void free_memory_Color(image* img) {
  int i = 0;
  int height = img->height;
  for(i = 0; i < height; i++) {
    free(img->pixels[i]);
  }
  free(img->pixels);
}

// Functie pentru scrierea pixelilor unei imagini alb-negru.
void writeDataBW(FILE *f, image *img) {
  int i = 0;
  int height = img->height;
  int width = img->width;
  // Valorile pixelilor trebuie scrise pe un rand.
  // Se scrie fiecare linie din matricea de pixeli.
  for (i = 0; i < height; i++) {
    fwrite(img->pixels_bw[i], 1, width, f);
  }

}

// Functie pentru scrierea pixelilor unei imagini color.
void writeDataColor(FILE *f, image *img) {
  int i = 0;
  int height = img->height;
  int width = img->width;
  // Valorile pixelilor trebuie scrise pe un rand.
  // Se scrie fiecare linie din matricea de pixeli.
  for (i = 0; i < height; i++) {
    fwrite(img->pixels[i], 1, 3 * width, f);
  }
}

// Functie pentru scrierea datelor noii imagini in fisierul de iesire.
void writeData(const char * fileName, image *img) {
  FILE *f = fopen(fileName, "w");
  if(f == NULL) {
    exit(1);
  }

  // Se scriu mai intai tipul imaginii, latimea, lungimea si valoarea maxima.
  fprintf(f,"%c%c\n", img->type[0], img->type[1]);
  fprintf(f,"%d %d\n", img->width, img->height);
  fprintf(f,"%d\n", img->maxval);

  // Daca imaginea este alb-negru, se apeleaza functia pentru scrierea pixelilor
  // unei imagini alb-negru. Daca imaginea este color se apeleaza functia pentru
  // scrierea pixelilor unei imagini color.
  if(img->type[1] == '5') {
    writeDataBW(f, img);
  } else {
    writeDataColor(f, img);
  }

  // Se elibereaza memoria alocata pentru imaginea de iesire.
  if(img->type[1] == '5') {
    free_memory_BW(img);
  } else {
    free_memory_Color(img);
  }

  fclose(f);
}

// Functie pentru calculul matricei filtrelor
void assign_filter_values(float filter_matrix[3][3], unsigned char filter) {
  switch (filter) {
    // Smooth
    case 'A':
      filter_matrix[0][0] = 1.0 / 9.0;
      filter_matrix[0][1] = 1.0 / 9.0;
      filter_matrix[0][2] = 1.0 / 9.0;
      filter_matrix[1][0] = 1.0 / 9.0;
      filter_matrix[1][1] = 1.0 / 9.0;
      filter_matrix[1][2] = 1.0 / 9.0;
      filter_matrix[2][0] = 1.0 / 9.0;
      filter_matrix[2][1] = 1.0 / 9.0;
      filter_matrix[2][2] = 1.0 / 9.0;
      break;
    // Blur
    case 'B':
      filter_matrix[0][0] = 1.0 / 16.0;
      filter_matrix[0][1] = 2.0 / 16.0;
      filter_matrix[0][2] = 1.0 / 16.0;
      filter_matrix[1][0] = 2.0 / 16.0;
      filter_matrix[1][1] = 4.0 / 16.0;
      filter_matrix[1][2] = 2.0 / 16.0;
      filter_matrix[2][0] = 1.0 / 16.0;
      filter_matrix[2][1] = 2.0 / 16.0;
      filter_matrix[2][2] = 1.0 / 16.0;
      break;
    // Sharpen
    case 'C':
      filter_matrix[0][0] = 0.0;
      filter_matrix[0][1] = -2.0 / 3.0;
      filter_matrix[0][2] = 0;
      filter_matrix[1][0] = -2.0 / 3.0;
      filter_matrix[1][1] = 11.0 / 3.0;
      filter_matrix[1][2] = -2.0 / 3.0;
      filter_matrix[2][0] = 0.0;
      filter_matrix[2][1] = -2.0 / 3.0;
      filter_matrix[2][2] = 0.0;
      break;
    // Mean
    case 'D':
      filter_matrix[0][0] = -1.0;
      filter_matrix[0][1] = -1.0;
      filter_matrix[0][2] = -1.0;
      filter_matrix[1][0] = -1.0;
      filter_matrix[1][1] = 9.0;
      filter_matrix[1][2] = -1.0;
      filter_matrix[2][0] = -1.0;
      filter_matrix[2][1] = -1.0;
      filter_matrix[2][2] = -1.0;
      break;
    // Emboss
    case 'E':
      filter_matrix[0][0] = 0.0;
      filter_matrix[0][1] = 1.0;
      filter_matrix[0][2] = 0.0;
      filter_matrix[1][0] = 0.0;
      filter_matrix[1][1] = 0.0;
      filter_matrix[1][2] = 0.0;
      filter_matrix[2][0] = 0.0;
      filter_matrix[2][1] = -1.0;
      filter_matrix[2][2] = 0.0;
      break;
  }
}


int main(int argc, char* argv[]) {
  int rank;
  int nProcesses;
  MPI_Status status;
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &nProcesses);

  // Formez structura de tip "rgb" pentru comunicatie
  const int nitems = 3;
  int blocklengths[3] = {1,1,1};
  MPI_Datatype types[3] = {MPI_UNSIGNED_CHAR, MPI_UNSIGNED_CHAR, MPI_UNSIGNED_CHAR};
  MPI_Datatype mpi_pixel_info;
  MPI_Aint  offsets[3];
  offsets[0] = offsetof(rgb, red);
  offsets[1] = offsetof(rgb, green);
  offsets[2] = offsetof(rgb, blue);
  MPI_Type_create_struct(nitems, blocklengths, offsets, types, &mpi_pixel_info);
  MPI_Type_commit(&mpi_pixel_info);

  // Pe pozitia 0 va fi numarul de linii de procesat al fiecarui proces
  // si pe pozitia 1 va fi numarul de coloane.
  int packet_size[2];
  // Numarul de filtre care trebuie aplicate.
  int number_filters = argc - 3;
  // Flag pentru a sti daca procesul curent este ultimul proces cu linii
  // de procesat (pot fi procese care sa nu aiba linii de procesat)
  int last_with_lines = 0;
  // Vectorul in care se retin filtrele.
  unsigned char filters[number_filters];

  // Ultimul proces este procesul master
  if (rank == nProcesses - 1) {
    // Trebuie sa se primeasca ca argumente cel putin imaginea in si out
    if(argc < 3) {
      exit(-1);
    }

    // Se retine pozitia primei linii de procesat pentru fiecare proces
    // si numarul de linii care vor fi procesate de catre acel proces
    elements vector_start[nProcesses];
    // Variabila pentru imaginea de intrare si iesire (se modifica
    // direct pe structura imaginii de intrare)
    image input;
    int start;
    int end;

    // Se formeaza vectorul filtrelor
    for (int i = 0; i < number_filters; i++) {
      if (!strcmp(argv[3 + i], "smooth")) { // pun ! ca returneaza 0 daca e ok
        filters[i] = 'A';
      } else if (!strcmp(argv[3 + i], "blur")) {
        filters[i] = 'B';
      } else if (!strcmp(argv[3 + i], "sharpen")) {
        filters[i] = 'C';
      } else if (!strcmp(argv[3 + i], "mean")) {
        filters[i] = 'D';
      } else if (!strcmp(argv[3 + i], "emboss")) {
        filters[i] = 'E';
      }
    }

    // Se trimite vectorul filtrelor la fiecare proces in parte
    MPI_Bcast(filters, number_filters, MPI_UNSIGNED_CHAR, nProcesses - 1, MPI_COMM_WORLD);

    // Se citeste imaginea
    readInput(argv[1], &input);

    packet_size[1] = input.width;

    // Se trimit fiecarui proces liniile corespunzatoare
    // In tag se trimite tipul imaginii
    int ok = 0;
    for (int i = 0; i < nProcesses - 1; i++) {
      ok = 0;
      // Se calculeaza intervalul de linii de prelucrat al fiecarui proces
      start = ceil((double)input.height / nProcesses) * i;
      end = fmin((i + 1) * ceil((double)input.height / nProcesses), input.height);
      vector_start[i].start = start;
      packet_size[0] = end - start;
      vector_start[i].number_lines = end - start;

      // Daca este ultimul proces ce are linii de prelucrat
      if ((end - start > 0) && end == input.height){
        ok = 1;
      }

      if (input.type[1] == '5') {
        if (ok == 1) {
          MPI_Send(packet_size, 2, MPI_INT, i, 10, MPI_COMM_WORLD);
        } else {
          MPI_Send(packet_size, 2, MPI_INT, i, 5, MPI_COMM_WORLD);
        }
        // Se trimit liniile de prelucrat fiecarui proces in parte
        // Daca nu sunt linii de prelucrat nu va intra in for oricum.
        for (int j = 0; j < packet_size[0]; j++) {
          MPI_Send(input.pixels_bw[start + j], input.width, MPI_UNSIGNED_CHAR, i, 0, MPI_COMM_WORLD);
        }
      } else {
        if (ok == 1) {
          MPI_Send(packet_size, 2, MPI_INT, i, 11, MPI_COMM_WORLD);
        } else {
          MPI_Send(packet_size, 2, MPI_INT, i, 6, MPI_COMM_WORLD);
        }
        // Se trimit liniile de prelucrat fiecarui proces in parte
        // Daca nu sunt linii de prelucrat nu va intra in for oricum.
        for (int j = 0; j < packet_size[0]; j++) {
          MPI_Send(input.pixels[start + j], input.width, mpi_pixel_info, i, 0, MPI_COMM_WORLD);
        }
      }
    }

    // Se retin valorile si pentru procesul master
    start = ceil((double)input.height / nProcesses) * (nProcesses - 1);
    end = fmin((nProcesses - 1 + 1) * ceil((double)input.height / nProcesses), input.height);
    vector_start[nProcesses - 1].start = start;
    packet_size[0] = end - start;
    vector_start[nProcesses - 1].number_lines = end - start;
    // Daca este ultimul proces ce are linii de prelucrat
    if ((end - start > 0) && end == input.width){
      last_with_lines = 1;
    }

    // Matricile in care se retin noile valori dupa aplicarea unui filtru
    unsigned char** local_matrix_bw;
    rgb** local_matrix;

    if(packet_size[0] > 0) {
      // Se aloca memorie pentru matricea locala
      if(input.type[1] == '5') {
        local_matrix_bw = (unsigned char**)malloc(packet_size[0] * sizeof(unsigned char*));
        if(local_matrix_bw == NULL) {
          exit(2);
        }
        for (int i = 0; i < packet_size[0]; i++) {
          local_matrix_bw[i] = (unsigned char*)malloc(input.width * sizeof(unsigned char));
          if(local_matrix_bw[i] == NULL) {
            exit(2);
          }
        }

        // Cazul cand imaginea este color.
      } else {
        local_matrix = (rgb**)malloc(packet_size[0] * sizeof(rgb*));
        if(local_matrix == NULL) {
          exit(2);
        }
        for (int i = 0; i < packet_size[0]; i++) {
          local_matrix[i] = (rgb*)malloc(input.width * sizeof(rgb));
          if(local_matrix[i] == NULL) {
            exit(2);
          }
        }
      }
    }

    // Matrice pentru valorile unui filtru
    float filter_matrix[3][3];
    MPI_Barrier(MPI_COMM_WORLD);

    // Se trece prin vectorul filtrelor si se aplica fiecare filtru
    // pe liniile corespunzatoare procesului
    for (int k = 0; k < number_filters; k++) {
      if (nProcesses > 1) {
        // Se primeste linia de sus (de la procesul anterior)
        if (packet_size[0] > 0) {
          if(input.type[1] == '5') {
            MPI_Recv(input.pixels_bw[vector_start[rank].start- 1] , packet_size[1], MPI_UNSIGNED_CHAR, rank - 1, 0, MPI_COMM_WORLD, &status);
          } else {
            MPI_Recv(input.pixels[vector_start[rank].start-1] , packet_size[1], mpi_pixel_info, rank - 1, 0, MPI_COMM_WORLD, &status);
          }
        }
        // Se trimite prima linia a procesului master la procesul anterior
        if (packet_size[0] > 0) {
          if(input.type[1] == '5') {
            MPI_Send(input.pixels_bw[vector_start[rank].start], packet_size[1], MPI_UNSIGNED_CHAR, rank - 1, 0, MPI_COMM_WORLD);
          } else {
            MPI_Send(input.pixels[vector_start[rank].start], packet_size[1], mpi_pixel_info, rank - 1, 0, MPI_COMM_WORLD);
          }
        }
      }

      // Se calculeaza valorile filtrului corespunzator
      assign_filter_values(filter_matrix, filters[k]);

      if (packet_size[0] > 0) {
        // Se calculeaza noile valori pentru imagine alb-negru
        if(input.type[1] == '5') {
          for (int i = vector_start[rank].start; i < input.height; i++) {
            for (int j = 0; j < input.width; j++) {
              // Marginile pozei
              if ((i == 0) || (i == input.height - 1) || (j == 0) || (j == input.width - 1)) {
                local_matrix_bw[i - vector_start[rank].start][j] = input.pixels_bw[i][j];
              } else {
                local_matrix_bw[i - vector_start[rank].start][j] =
                          filter_matrix[0][0] * input.pixels_bw[i - 1][j - 1] +
                          filter_matrix[0][1] * input.pixels_bw[i - 1][j] +
                          filter_matrix[0][2] * input.pixels_bw[i - 1][j + 1] +
                          filter_matrix[1][0] * input.pixels_bw[i][j - 1] +
                          filter_matrix[1][1] * input.pixels_bw[i][j] +
                          filter_matrix[1][2] * input.pixels_bw[i][j + 1] +
                          filter_matrix[2][0] * input.pixels_bw[i + 1][j - 1] +
                          filter_matrix[2][1] * input.pixels_bw[i + 1][j] +
                          filter_matrix[2][2] * input.pixels_bw[i + 1][j + 1];
              }
            }
          }
          // Se copiaza rezultatul in matricea initiala
          for (int i = vector_start[rank].start; i < input.height; i++) {
            memcpy(input.pixels_bw[i], local_matrix_bw[i - vector_start[rank].start], sizeof(unsigned char) * input.width);
          }

        // Se calculeaza noile valori pentru imagine color
        } else {
          for (int i = vector_start[rank].start; i < input.height; i++) {
            for (int j = 0; j < input.width; j++) {
              // Marginile pozei
              if ((i == 0) || (i == input.height - 1) || (j == 0) || (j == input.width - 1)) {
                local_matrix[i - vector_start[rank].start][j].red = input.pixels[i][j].red;
                local_matrix[i - vector_start[rank].start][j].green = input.pixels[i][j].green;
                local_matrix[i - vector_start[rank].start][j].blue = input.pixels[i][j].blue;
              } else {
                local_matrix[i - vector_start[rank].start][j].red =
                        filter_matrix[0][0] * input.pixels[i - 1][j - 1].red +
                        filter_matrix[0][1] * input.pixels[i - 1][j].red +
                        filter_matrix[0][2] * input.pixels[i - 1][j + 1].red +
                        filter_matrix[1][0] * input.pixels[i][j - 1].red +
                        filter_matrix[1][1] * input.pixels[i][j].red +
                        filter_matrix[1][2] * input.pixels[i][j + 1].red +
                        filter_matrix[2][0] * input.pixels[i + 1][j - 1].red +
                        filter_matrix[2][1] * input.pixels[i + 1][j].red +
                        filter_matrix[2][2] * input.pixels[i + 1][j + 1].red;

                local_matrix[i - vector_start[rank].start][j].green =
                        filter_matrix[0][0] * input.pixels[i - 1][j - 1].green +
                        filter_matrix[0][1] * input.pixels[i - 1][j].green +
                        filter_matrix[0][2] * input.pixels[i - 1][j + 1].green +
                        filter_matrix[1][0] * input.pixels[i][j - 1].green +
                        filter_matrix[1][1] * input.pixels[i][j].green +
                        filter_matrix[1][2] * input.pixels[i][j + 1].green +
                        filter_matrix[2][0] * input.pixels[i + 1][j - 1].green +
                        filter_matrix[2][1] * input.pixels[i + 1][j].green +
                        filter_matrix[2][2] * input.pixels[i + 1][j + 1].green;

                local_matrix[i - vector_start[rank].start][j].blue =
                        filter_matrix[0][0] * input.pixels[i - 1][j - 1].blue +
                        filter_matrix[0][1] * input.pixels[i - 1][j].blue +
                        filter_matrix[0][2] * input.pixels[i - 1][j + 1].blue +
                        filter_matrix[1][0] * input.pixels[i][j - 1].blue +
                        filter_matrix[1][1] * input.pixels[i][j].blue +
                        filter_matrix[1][2] * input.pixels[i][j + 1].blue +
                        filter_matrix[2][0] * input.pixels[i + 1][j - 1].blue +
                        filter_matrix[2][1] * input.pixels[i + 1][j].blue +
                        filter_matrix[2][2] * input.pixels[i + 1][j + 1].blue;
              }
            }
          }
          // Se copiaza rezultatul in matricea initiala
          for (int i = vector_start[rank].start; i < input.height; i++) {
            memcpy(input.pixels[i], local_matrix[i - vector_start[rank].start], sizeof(rgb) * input.width);
          }
        }
      }
      MPI_Barrier(MPI_COMM_WORLD);
    }

    // Se primesc liniile modificate de la celelalte procese pentru asamblare
    if(input.type[1] == '5') {
      for (int i = 0; i < nProcesses - 1; i++) {
        for (int j = 0; j < vector_start[i].number_lines; j++) {
          MPI_Recv(input.pixels_bw[vector_start[i].start + j], input.width, MPI_UNSIGNED_CHAR, i, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        }
      }
    } else {
      for (int i = 0; i < nProcesses - 1; i++) {
        for (int j = 0; j < vector_start[i].number_lines; j++) {
          MPI_Recv(input.pixels[vector_start[i].start + j], input.width, mpi_pixel_info, i, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        }
      }
    }

    if (packet_size[0] > 0) {
      // Se elibereaza memoria pentru matricea locala
      if(input.type[1] == '5') {
        for (int i = 0; i < packet_size[0]; i++) {
          free(local_matrix_bw[i]);
        }
        free(local_matrix_bw);
      // Cazul cand imaginea este color.
      } else {
        for (int i = 0; i < packet_size[0]; i++) {
          free(local_matrix[i]);
        }
        free(local_matrix);
      }
    }

    // Se scrie noua imagine in fisierul de output
    writeData(argv[2], &input);

 // Cazul pentru procesele care nu sunt master
  } else {

    // Matricea "initiala" este compusa din liniile initiale pe care le are de
    // procesat, dar matricea va mai avea 1-2 linii in plus pentru a putea
    // lua liniile de la vecini, necesare pentru calcul.
    int offset_start = 0; // Pentru a sti daca e o linie libera la inceput
    int offset_end = 0; // Pentru a sti daca e o linie libera la final
    // Matrice pentru liniile initiale de prelucrat primite de la procesul master
    unsigned char** pixels_bw;
    rgb** pixels;
    // Numarul total de linii alocate pentru matricea initiala
    int nr_lines_allocated_initial = 0;
    // Tipul imaginii
    int type_image;

    // Se primeste vectorul cu filtrele ce trebuie aplicate
    MPI_Bcast(filters, number_filters, MPI_UNSIGNED_CHAR, nProcesses - 1, MPI_COMM_WORLD);
    // Se primeste numarul de linii ce trebuie prelucrate si dimensiunea unei linii
    MPI_Recv(packet_size, 2, MPI_INT, nProcesses - 1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    nr_lines_allocated_initial = packet_size[0];

    // Se aloca memoria necesara si se primesc liniile de la procesul master
    if (packet_size[0] > 0) {
      if (status.MPI_TAG == 5 || status.MPI_TAG == 10) {
        type_image = 5;
        // Daca mpi_tag == 10 => ultimul proces cu linii de prelucrat
        // Daca rank == 0 => nu se primesc linii mai de deasupra
        // => trebuie alocate numarul de linii primit de la master +
        // o linie suplimentara
        if(status.MPI_TAG == 10 || rank == 0) {
          nr_lines_allocated_initial++;
          if(status.MPI_TAG == 10) {
            last_with_lines = 1;
          }
          // Se aloca matricea initiala cu o linie in plus sus pentru a primii
          // date de la vecinul de sus sau de jos (primul sau ultimul proces)
          pixels_bw = (unsigned char**)malloc((packet_size[0] + 1) * sizeof(unsigned char*));
          if(pixels_bw == NULL) {
            exit(2);
          }
          for (int i = 0; i < packet_size[0] + 1; i++) {
            pixels_bw[i] = (unsigned char*)malloc(packet_size[1]  * sizeof(unsigned char));
            if(pixels_bw[i] == NULL) {
              exit(2);
            }
          }
          // Se primesc datele
          for (int i = 0; i < packet_size[0] + 1; i++) {
            // Daca e primul proces => se lasa ultima linie libera
            if (rank == 0) {
              if (i < packet_size[0]){
                MPI_Recv(pixels_bw[i], packet_size[1], MPI_UNSIGNED_CHAR, nProcesses - 1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                offset_end = 1;
              }
            } else {
              // Daca e ultimul proces se lasa prima linie libera
              if (i < packet_size[0]){
                MPI_Recv(pixels_bw[i + 1], packet_size[1], MPI_UNSIGNED_CHAR, nProcesses - 1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                offset_start = 1;
              }
            }
          }

        // Cazul cand nu este nici primul nici ultimul proces ce primeste linii
        // si trebuie alocate 2 linii in plus pentru a primii linii de la vecini
        } else {
          nr_lines_allocated_initial += 2;
          pixels_bw = (unsigned char**)malloc((packet_size[0] + 2) * sizeof(unsigned char*));
          if(pixels_bw == NULL) {
            exit(2);
          }
          for (int i = 0; i < packet_size[0] + 2; i++) {
            pixels_bw[i] = (unsigned char*)malloc(packet_size[1]  * sizeof(unsigned char));
            if(pixels_bw[i] == NULL) {
              exit(2);
            }
          }
          // Se primesc datele
          for (int i = 0; i < packet_size[0] + 2; i++) {
            if (i < packet_size[0]){
              offset_start = 1;
              offset_end = 1;
              MPI_Recv(pixels_bw[i + 1], packet_size[1], MPI_UNSIGNED_CHAR, nProcesses - 1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            }
          }
        }

      // Cazul cand imaginea este color
      } else {
        type_image = 6;
        // Daca mpi_tag == 11 => ultimul proces cu linii de prelucrat
        // Daca rank == 0 => nu se primesc linii mai de deasupra
        // => trebuie alocate numarul de linii primit de la master +
        // o linie suplimentara
        if(status.MPI_TAG == 11 || rank == 0) {
          nr_lines_allocated_initial++;
          if(status.MPI_TAG == 11) {
            last_with_lines = 1;
          }
          // Se aloca matricea initiala cu o linie in plus sus pentru a primii
          // date de la vecinul de sus sau de jos (primul sau ultimul proces)
          pixels = (rgb**)malloc((packet_size[0] + 1) * sizeof(rgb*));
          if(pixels == NULL) {
            exit(2);
          }
          for (int i = 0; i < packet_size[0] + 1; i++) {
            pixels[i] = (rgb*)malloc(packet_size[1]  * sizeof(rgb));
            if(pixels[i] == NULL) {
              exit(2);
            }
          }
          // Se primesc datele
          for (int i = 0; i < packet_size[0] + 1; i++) {
            if (rank == 0) {
              // Daca e primul proces se lasa ultima linie libera
              if (i < packet_size[0]){
                MPI_Recv(pixels[i], packet_size[1], mpi_pixel_info, nProcesses - 1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                offset_end = 1;
              }
            } else {
              if (i < packet_size[0]){
                // Daca e ultimul proces se lasa prima linie libera
                MPI_Recv(pixels[i + 1], packet_size[1], mpi_pixel_info, nProcesses - 1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                offset_start = 1;
              }
            }
          }

        // Cazul cand nu este nici primul nici ultimul proces ce primeste linii
        // si trebuie alocate 2 linii in plus pentru a primii linii de la vecini
        } else {
          nr_lines_allocated_initial += 2;
          pixels = (rgb**)malloc((packet_size[0] + 2) * sizeof(rgb*));
          if(pixels == NULL) {
            exit(2);
          }
          for (int i = 0; i < packet_size[0] + 2; i++) {
            pixels[i] = (rgb*)malloc(packet_size[1]  * sizeof(rgb));
            if(pixels[i] == NULL) {
              exit(2);
            }
          }
          // Se primesc datele
          for (int i = 0; i < packet_size[0] + 2; i++) {
            if (i < packet_size[0]){
              offset_start = 1;
              offset_end = 1;
              MPI_Recv(pixels[i + 1], packet_size[1], mpi_pixel_info, nProcesses - 1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            }
          }
        }
      }
    }

    // Matricile in care se retin noile valori dupa aplicarea unui filtru
    unsigned char** local_matrix_bw;
    rgb** local_matrix;

    if (packet_size[0] > 0) {
      // Se aloca memorie pentru matricea locala
      if(type_image == 5) {
        local_matrix_bw = (unsigned char**)malloc(packet_size[0] * sizeof(unsigned char*));
        if(local_matrix_bw == NULL) {
          exit(2);
        }
        for (int i = 0; i < packet_size[0]; i++) {
          local_matrix_bw[i] = (unsigned char*)malloc(packet_size[1] * sizeof(unsigned char));
          if(local_matrix_bw[i] == NULL) {
            exit(2);
          }
        }
      // Cazul cand imaginea este color.
      } else {
        local_matrix = (rgb**)malloc(packet_size[0] * sizeof(rgb*));
        if(local_matrix == NULL) {
          exit(2);
        }
        for (int i = 0; i < packet_size[0]; i++) {
          local_matrix[i] = (rgb*)malloc(packet_size[1] * sizeof(rgb));
          if(local_matrix[i] == NULL) {
            exit(2);
          }
        }
      }
    }
    
    // Matrice pentru valorile unui anume filtru
    float filter_matrix[3][3];
    MPI_Barrier(MPI_COMM_WORLD);

    // Se trece prin vectorul filtrelor si se aplica fiecare filtru
    // pe liniile corespunzatoare procesului
    for (int k = 0; k < number_filters; k++) {
      // Daca nu e ultimul proces cu linii si are linii de trimis se trimite
      // linia la veciniul de dupa (linia asta o sa fie linie superioara a vecinului)
      if (last_with_lines == 0 && packet_size[0] > 0) {
        if(type_image == 5) {
          MPI_Send(pixels_bw[packet_size[0] - 1 + offset_start], packet_size[1], MPI_UNSIGNED_CHAR, rank + 1, 0, MPI_COMM_WORLD);
        } else {
          MPI_Send(pixels[packet_size[0] - 1 + offset_start], packet_size[1], mpi_pixel_info, rank + 1, 0, MPI_COMM_WORLD);
        }
      }
      // Daca are linii de prelucrat si nu e primul proces
      // se primeste linia de sus
      if (rank != 0 && packet_size[0] > 0) {
        if(type_image == 5) {
          MPI_Recv(pixels_bw[0], packet_size[1], MPI_UNSIGNED_CHAR, rank - 1, 0, MPI_COMM_WORLD, &status);
        } else {
          MPI_Recv(pixels[0], packet_size[1], mpi_pixel_info, rank - 1, 0, MPI_COMM_WORLD, &status);
        }
      }
      // Daca nu e primul proces cu linii si are linii de trimis se trimite
      // linia la veciniul de inainte (linia asta o sa fie linia de jos a vecinului)
      if (rank != 0 && packet_size[0] > 0) {
        if(type_image == 5) {
          MPI_Send(pixels_bw[0 + offset_start], packet_size[1], MPI_UNSIGNED_CHAR, rank - 1, 0, MPI_COMM_WORLD);
        } else {
          MPI_Send(pixels[0 + offset_start], packet_size[1], mpi_pixel_info, rank - 1, 0, MPI_COMM_WORLD);
        }
      }
      // Daca are linii de prelucrat si nu e ultimul proces
      // se primeste linia de jos
      if (last_with_lines == 0 && packet_size[0] > 0) {
        if(type_image == 5) {
          MPI_Recv(pixels_bw[packet_size[0] - 1 + offset_start + offset_end], packet_size[1], MPI_UNSIGNED_CHAR, rank + 1, 0, MPI_COMM_WORLD, &status);
        } else {
          MPI_Recv(pixels[packet_size[0] - 1 + offset_start + offset_end], packet_size[1], mpi_pixel_info, rank + 1, 0, MPI_COMM_WORLD, &status);
        }
      }

      // Se calculeaza valorile filtrului corespunzator
      assign_filter_values(filter_matrix, filters[k]);

      if (packet_size[0] > 0) {
        // Se calculeaza noile valori pentru imagine alb-negru
        if(type_image == 5) {
          for (int i = 0 + offset_start; i < packet_size[0] + offset_start; i++) {
            for (int j = 0; j < packet_size[1]; j++) {
              // Marginile imaginii
              if ((rank == 0 && i == 0 + offset_start) ||
                 ((i ==  packet_size[0] - 1 + offset_start) && (last_with_lines == 1)) ||
                 (j == 0) || (j == packet_size[1] - 1)) {

                local_matrix_bw[i - offset_start][j] = pixels_bw[i][j];
              } else {
                local_matrix_bw[i - offset_start][j] =
                                filter_matrix[0][0] * pixels_bw[i - 1][j - 1] +
                                filter_matrix[0][1] * pixels_bw[i - 1][j] +
                                filter_matrix[0][2] * pixels_bw[i - 1][j + 1] +
                                filter_matrix[1][0] * pixels_bw[i][j - 1] +
                                filter_matrix[1][1] * pixels_bw[i][j] +
                                filter_matrix[1][2] * pixels_bw[i][j + 1] +
                                filter_matrix[2][0] * pixels_bw[i + 1][j - 1] +
                                filter_matrix[2][1] * pixels_bw[i + 1][j] +
                                filter_matrix[2][2] * pixels_bw[i + 1][j + 1];
              }
            }
          }
          // Se copiaza rezultatul in matricea initiala
          for (int i = 0 + offset_start; i < packet_size[0] + offset_start; i++) {
            memcpy(pixels_bw[i], local_matrix_bw[i - offset_start], sizeof(unsigned char) * packet_size[1]);
          }

        } else {
          // Se calculeaza noile valori pentru imagine color
          for (int i = 0 + offset_start; i < packet_size[0] + offset_start; i++) {
            for (int j = 0; j < packet_size[1]; j++) {
              if ((rank == 0 && i == 0 + offset_start) ||
                 ((i ==  packet_size[0] - 1 + offset_start) && (last_with_lines == 1)) ||
                 (j == 0) || (j == packet_size[1] - 1)) {

                local_matrix[i - offset_start][j].red = pixels[i][j].red;
                local_matrix[i - offset_start][j].green = pixels[i][j].green;
                local_matrix[i - offset_start][j].blue = pixels[i][j].blue;
              } else {
                local_matrix[i - offset_start][j].red =
                                filter_matrix[0][0] * pixels[i - 1][j - 1].red +
                                filter_matrix[0][1] * pixels[i - 1][j].red +
                                filter_matrix[0][2] * pixels[i - 1][j + 1].red +
                                filter_matrix[1][0] * pixels[i][j - 1].red +
                                filter_matrix[1][1] * pixels[i][j].red +
                                filter_matrix[1][2] * pixels[i][j + 1].red +
                                filter_matrix[2][0] * pixels[i + 1][j - 1].red +
                                filter_matrix[2][1] * pixels[i + 1][j].red +
                                filter_matrix[2][2] * pixels[i + 1][j + 1].red;

                local_matrix[i - offset_start][j].green =
                                filter_matrix[0][0] * pixels[i - 1][j - 1].green +
                                filter_matrix[0][1] * pixels[i - 1][j].green +
                                filter_matrix[0][2] * pixels[i - 1][j + 1].green +
                                filter_matrix[1][0] * pixels[i][j - 1].green +
                                filter_matrix[1][1] * pixels[i][j].green +
                                filter_matrix[1][2] * pixels[i][j + 1].green +
                                filter_matrix[2][0] * pixels[i + 1][j - 1].green +
                                filter_matrix[2][1] * pixels[i + 1][j].green +
                                filter_matrix[2][2] * pixels[i + 1][j + 1].green;

                local_matrix[i - offset_start][j].blue =
                                filter_matrix[0][0] * pixels[i - 1][j - 1].blue +
                                filter_matrix[0][1] * pixels[i - 1][j].blue +
                                filter_matrix[0][2] * pixels[i - 1][j + 1].blue +
                                filter_matrix[1][0] * pixels[i][j - 1].blue +
                                filter_matrix[1][1] * pixels[i][j].blue +
                                filter_matrix[1][2] * pixels[i][j + 1].blue +
                                filter_matrix[2][0] * pixels[i + 1][j - 1].blue +
                                filter_matrix[2][1] * pixels[i + 1][j].blue +
                                filter_matrix[2][2] * pixels[i + 1][j + 1].blue;
              }
            }
          }
          // Se copiaza rezultatul in matricea initiala
          for (int i = 0 + offset_start; i < packet_size[0] + offset_start; i++) {
            memcpy(pixels[i], local_matrix[i - offset_start], sizeof(rgb) * packet_size[1]);
          }
        }
      }
      MPI_Barrier(MPI_COMM_WORLD);
    }

    // Se trimit liniile catre master pentru asamblare
    if(type_image == 5) {
      for (int i = 0; i < packet_size[0]; i++) {
        MPI_Send(local_matrix_bw[i], packet_size[1], MPI_UNSIGNED_CHAR, nProcesses -1, i, MPI_COMM_WORLD);
      }
    } else {
      for (int i = 0; i < packet_size[0]; i++) {
        MPI_Send(local_matrix[i], packet_size[1], mpi_pixel_info, nProcesses -1, i, MPI_COMM_WORLD);
      }
    }

    if (packet_size[0] > 0) {
      // Se elibereaza memoria pentru matricea locala
      if(type_image == 5) {
        for (int i = 0; i < packet_size[0]; i++) {
          free(local_matrix_bw[i]);
        }
        free(local_matrix_bw);
        // Cazul cand imaginea este color.
      } else {
        for (int i = 0; i < packet_size[0]; i++) {
          free(local_matrix[i]);
        }
        free(local_matrix);
      }

      // Se elibereaza memoria pentru matricea initiala
      if(type_image == 5) {
        for (int i = 0; i < nr_lines_allocated_initial; i++) {
          free(pixels_bw[i]);
        }
        free(pixels_bw);
        // Cazul cand imaginea este color.
      } else {
        for (int i = 0; i < nr_lines_allocated_initial; i++) {
          free(pixels[i]);
        }
        free(pixels);
      }
    }
  }

  MPI_Finalize();
  return 0;
}
