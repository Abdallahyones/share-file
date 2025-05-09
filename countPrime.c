#include <stdio.h>
#include <mpi.h>

int is_prime(int n) {
    if (n < 2) return 0;
    for (int i = 2; i * i <= n; ++i)
        if (n % i == 0) return 0;
    return 1;
}

int main(int argc, char** argv) {
    int x = 1, y = 16;  // Change as needed

    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int data[2] = {x, y};
    MPI_Bcast(data, 2, MPI_INT, 0, MPI_COMM_WORLD);
    x = data[0]; y = data[1];

    int total_range = y - x;
    int r = total_range / size;
    int extra = total_range % size;

    int start = x + rank * r + (rank < extra ? rank : extra);
    int end = start + r + (rank < extra ? 1 : 0);

    int local_count = 0;
    for (int i = start; i < end; ++i) {
        if (is_prime(i)) local_count++;
    }

    int global_count = 0;
    MPI_Reduce(&local_count, &global_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0)
        printf("Total primes between %d and %d = %d\n", x, y, global_count);

    MPI_Finalize();
    return 0;
}
