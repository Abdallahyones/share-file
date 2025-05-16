#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

#define fo(i , n) for (int i = 0 ; i < n ; i++)

int main() {
    int n;

    printf("Please,Enter the size of the matrix: \n");
    scanf("%d", &n);

    int** matrix = (int**) malloc(n * sizeof(int*));
    int* vec = (int*) malloc(n * sizeof(int));
    int* res = (int*) malloc(n * sizeof(int));

    fo(i , n){
        matrix[i] = (int*) malloc(n * sizeof(int));
    }


    printf("Enter elements of %dx%d matrix:\n", n, n);
    fo(i , n)
        fo(j , n)
            scanf("%d", &matrix[i][j]);


    printf("Enter elements of vector (size %d):\n", n);
    fo(i , n)
        scanf("%d", &vec[i]);

    #pragma omp parallel for
    fo(i , n) {
        res[i] = 0;
        fo(j , n){
            res[i] += matrix[i][j] * vec[j];
        }
    }

    printf("Resulting vector res = \n");
    fo(i , n) {
        printf("%d ", res[i]);
    }
    printf("\n");


    fo(i , n)
        free(matrix[i]);
    free(matrix);
    free(vec);
    free(res);

    return 0;
}
