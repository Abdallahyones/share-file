#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LEN 1000000
#define min(a,b) (((a)<(b))?(a):(b))

void encrypt(char* s, int n) {
    for (int i = 0; i < n; i++) {
        if (! isalpha(s[i])) continue;
        char base = isupper(s[i]) ? 'A' : 'a';
        s[i] = base + (s[i] - base + 3) % 26;
    }
}

void decrypt(char* s, int n) {
    for (int i = 0; i < n; i++) {
        if (! isalpha(s[i])) continue;
        char base = isupper(s[i]) ? 'A' : 'a';
        s[i] = base + (s[i] - base + 26 - 3) % 26;
    }
}

int read_from_file(const char* filename, char* buffer) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Error opening input file");
        return -1;
    }
    int len = fread(buffer, sizeof(char), MAX_LEN, file);
    buffer[len] = '\0';
    fclose(file);
    return len;
}

void write_to_file(const char* filename, const char* buffer, int length) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        perror("Error opening output file");
        return;
    }
    fwrite(buffer, sizeof(char), length, file);
    fclose(file);
}

int main(int argc, char** argv) {
    int rank, size, option;
    char input[MAX_LEN];
    int input_len = 0;
    char* segment;
    char* result;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    char input_filename[256];
    char output_filename[256];

    if (rank == 0) {
        if (argc < 4) {  // 👈 updated to expect 3 arguments
            fprintf(stderr, "Usage: %s <input_file> <output_file> <1=Encode, 2=Decode>\n", argv[0]);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        strncpy(input_filename, argv[1], sizeof(input_filename));
        strncpy(output_filename, argv[2], sizeof(output_filename));
        option = atoi(argv[3]);  // 👈 read option from command line

        if (option != 1 && option != 2) {
            fprintf(stderr, "Invalid option: %d. Use 1 for Encode or 2 for Decode.\n", option);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }

        input_len = read_from_file(input_filename, input);
        if (input_len < 0) {
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    MPI_Bcast(&option, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&input_len, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int chunk_size = (input_len + size - 1) / size;
    segment = (char*)malloc(chunk_size * sizeof(char));
    result = (rank == 0) ? (char*)malloc((input_len + 1) * sizeof(char)) : NULL;

    MPI_Scatter(input, chunk_size, MPI_CHAR, segment, chunk_size, MPI_CHAR, 0, MPI_COMM_WORLD);

    int actual_len = min(chunk_size, input_len - rank * chunk_size);
    if (option == 1)
        encrypt(segment, actual_len);
    else
        decrypt(segment, actual_len);

    MPI_Gather(segment, chunk_size, MPI_CHAR, result, chunk_size, MPI_CHAR, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        result[input_len] = '\0';
        write_to_file(output_filename, result, input_len);
       // printf("Output written to %s\n", output_filename);
        free(result);
    }

    free(segment);
    MPI_Finalize();
    return 0;
}
