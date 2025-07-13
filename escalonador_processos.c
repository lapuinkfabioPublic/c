/*
Fabio Leandro Lapuinka
Escalonador de Processos
*/
#include <stdio.h>
#include <stdlib.h>

// Estrutura para matriz esparsa no formato CSR
typedef struct {
    int* values;     // Valores não nulos
    int* col_indices; // Índices de coluna dos valores não nulos
    int* row_ptr;     // Ponteiros para o início de cada linha
    int num_rows;     // Número de linhas
    int num_cols;     // Número de colunas
    int nnz;          // Número de elementos não nulos
} SparseMatrixCSR;

// Função para ler matriz de arquivo e converter para CSR
SparseMatrixCSR* readMatrixToCSR(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Erro ao abrir arquivo");
        return NULL;
    }

    int rows, cols;
    fscanf(file, "%d %d", &rows, &cols);

    // Contar elementos não nulos
    int nnz = 0;
    int val;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            fscanf(file, "%d", &val);
            if (val != 0) nnz++;
        }
    }

    // Alocar memória para a matriz esparsa
    SparseMatrixCSR* matrix = (SparseMatrixCSR*)malloc(sizeof(SparseMatrixCSR));
    matrix->values = (int*)malloc(nnz * sizeof(int));
    matrix->col_indices = (int*)malloc(nnz * sizeof(int));
    matrix->row_ptr = (int*)malloc((rows + 1) * sizeof(int));
    matrix->num_rows = rows;
    matrix->num_cols = cols;
    matrix->nnz = nnz;

    // Voltar ao início do arquivo
    rewind(file);
    fscanf(file, "%d %d", &rows, &cols);

    // Preencher a estrutura CSR
    int index = 0;
    matrix->row_ptr[0] = 0;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            fscanf(file, "%d", &val);
            if (val != 0) {
                matrix->values[index] = val;
                matrix->col_indices[index] = j;
                index++;
            }
        }
        matrix->row_ptr[i + 1] = index;
    }

    fclose(file);
    return matrix;
}

// Função para liberar memória da matriz esparsa
void freeSparseMatrix(SparseMatrixCSR* matrix) {
    if (matrix != NULL) {
        free(matrix->values);
        free(matrix->col_indices);
        free(matrix->row_ptr);
        free(matrix);
    }
}

// Função para imprimir matriz esparsa (para debug)
void printSparseMatrix(const SparseMatrixCSR* matrix) {
    printf("Matriz Esparsa CSR (%d x %d) com %d elementos não nulos:\n", 
           matrix->num_rows, matrix->num_cols, matrix->nnz);
    
    printf("Valores: ");
    for (int i = 0; i < matrix->nnz; i++) {
        printf("%d ", matrix->values[i]);
    }
    printf("\n");
    
    printf("Índices de Coluna: ");
    for (int i = 0; i < matrix->nnz; i++) {
        printf("%d ", matrix->col_indices[i]);
    }
    printf("\n");
    
    printf("Ponteiros de Linha: ");
    for (int i = 0; i <= matrix->num_rows; i++) {
        printf("%d ", matrix->row_ptr[i]);
    }
    printf("\n");
}

int main() {
    const char* filename = "matriz.txt"; // Arquivo de entrada
    
    // Exemplo de formato do arquivo:
    // 3 4       // linhas colunas
    // 1 0 0 2   // elementos da matriz
    // 0 0 3 0
    // 4 0 0 0
    
    SparseMatrixCSR* matrix = readMatrixToCSR(filename);
    if (matrix == NULL) {
        printf("Falha ao ler a matriz do arquivo.\n");
        return 1;
    }
    
    printSparseMatrix(matrix);
    freeSparseMatrix(matrix);
    
    return 0;
}
