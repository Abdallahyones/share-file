#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LEN 1000000
#define SHIFT 3
#define min(a,b) (((a)<(b))?(a):(b))

// Caesar cipher encryption
void encrypt(char* str, int len) {
    for (int i = 0; i < len; i++) {
        if (isalpha(str[i])) {
            char base = isupper(str[i]) ? 'A' : 'a';
            str[i] = ((str[i] - base + SHIFT) % 26) + base;
        }
    }
}

// Caesar cipher decryption
void decrypt(char* str, int len) {
    for (int i = 0; i < len; i++) {
        if (isalpha(str[i])) {
            char base = isupper(str[i]) ? 'A' : 'a';
            str[i] = ((str[i] - base - SHIFT + 26) % 26) + base;
        }
    }
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

    if (rank == 0) {
        // Console-only mode
        printf("Enter 1 for Encode, 2 for Decode: ");
        scanf("%d", &option);
        printf("Enter input text: ");
        scanf("%s", input);
        input_len = strlen(input);
    }

    // Broadcast the mode and length to all processes
    MPI_Bcast(&option, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&input_len, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int chunk_size = (input_len + size - 1) / size;
    segment = (char*)malloc(chunk_size * sizeof(char));
    result = (rank == 0) ? (char*)malloc((input_len + 1) * sizeof(char)) : NULL;

    // Distribute chunks of input
    MPI_Scatter(input, chunk_size, MPI_CHAR, segment, chunk_size, MPI_CHAR, 0, MPI_COMM_WORLD);

    // Encrypt/Decrypt the portion assigned to this process
    int actual_len = min(chunk_size, input_len - rank * chunk_size);
    if (option == 1)
        encrypt(segment, actual_len);
    else
        decrypt(segment, actual_len);

    // Collect results
    MPI_Gather(segment, chunk_size, MPI_CHAR, result, chunk_size, MPI_CHAR, 0, MPI_COMM_WORLD);

    // Master prints the result
    if (rank == 0) {
        result[input_len] = '\0';
        printf("Output: %s\n", result);
        free(result);
    }

    free(segment);
    MPI_Finalize();
    return 0;
}
