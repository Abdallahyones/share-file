#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LEN 1000000
#define min(a,b) (((a)<(b))?(a):(b))

// Caesar cipher encryption
void encrypt(char* s, int n) {
    for (int i = 0; i < n; i++) {
        if (!isalpha(s[i])) continue;
        char base = isupper(s[i]) ? 'A' : 'a';
        int idx  = (s[i] - base + 3) % 26;
        s[i] = base + idx;
    }
}

// Caesar cipher decryption
void decrypt(char* s, int n) {
    for (int i = 0; i < n; i++) {
        if (!isalpha(s[i])) continue;
        char base = isupper(s[i]) ? 'A' : 'a';
        int idx  = (s[i] - base + 26 - 3) % 26;
        s[i] = base + idx;
    }
}

int main(int argc, char** argv) {
    int rank, size;
    int option, input_len;
    char  input[MAX_LEN];
    char* segment;
    char* result = NULL;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        // 1) Read mode and text from user
        printf("Enter 1 for Encode, 2 for Decode: ");
        scanf("%d", &option);
        printf("Enter input text: ");
        scanf("%s", input);
        input_len = strlen(input);

        // 2) Send mode and length to each other rank
        for (int dst = 1; dst < size; dst++) {
            MPI_Send(&option,    1, MPI_INT, dst, /*tag=*/0, MPI_COMM_WORLD);
            MPI_Send(&input_len, 1, MPI_INT, dst, /*tag=*/0, MPI_COMM_WORLD);
        }
    } else {
        // 3) Receive mode and length
        MPI_Recv(&option,    1, MPI_INT, 0, /*tag=*/0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&input_len, 1, MPI_INT, 0, /*tag=*/0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    // 4) Compute chunk boundaries
    int chunk_size = (input_len + size - 1) / size;
    int offset     = rank * chunk_size;
    int actual_len = min(chunk_size, input_len - offset);
    if (actual_len < 0) actual_len = 0;

    segment = malloc(chunk_size);
    if (!segment) {
        fprintf(stderr, "Rank %d: malloc failed\n", rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    if (rank == 0) {
        // 5) Root sends each segment
        for (int dst = 1; dst < size; dst++) {
            int off = dst * chunk_size;
            int len = min(chunk_size, input_len - off);
            if (len > 0)
                MPI_Send(input + off, len, MPI_CHAR, dst, /*tag=*/1, MPI_COMM_WORLD);
        }
        // 6) Root copies its own segment
        memcpy(segment, input, actual_len);
        // Prepare result buffer
        result = malloc(input_len + 1);
    } else {
        // 7) Each non-root receives its segment
        MPI_Recv(segment, actual_len, MPI_CHAR, 0, /*tag=*/1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    // 8) Encrypt or decrypt locally
    if (option == 1)
        encrypt(segment, actual_len);
    else
        decrypt(segment, actual_len);

    if (rank == 0) {
        // 9) Root places its own processed chunk
        memcpy(result, segment, actual_len);

        // 10) Root collects processed chunks from others
        for (int src = 1; src < size; src++) {
            int off = src * chunk_size;
            int len = min(chunk_size, input_len - off);
            if (len > 0)
                MPI_Recv(result + off, len, MPI_CHAR, src, /*tag=*/2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
        result[input_len] = '\0';
        printf("Output: %s\n", result);
        free(result);
    } else {
        // 11) Non-roots send back their processed chunk
        MPI_Send(segment, actual_len, MPI_CHAR, 0, /*tag=*/2, MPI_COMM_WORLD);
    }

    free(segment);
    MPI_Finalize();
    return 0;
}
