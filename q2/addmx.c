#include <sys/mman.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h> 

#define MAX_NUM_CHARS 2

int getMatrixInfo(char *filename1, char *filename2, int *rows, int *cols);

int main(int argc, char **argv) {
    if (argc < 2){
        printf("usage: addmx file1 file2\n");
        return 0;
    }

    FILE *file1;
    FILE *file2;
    char *filename1 = argv[1], *filename2 = argv[2];
    int rows, cols;

    if(getMatrixInfo(filename1, filename2, &rows, &cols)){
        printf("They dont have same dimensions! \n");
        return 1;
    }

    int *shared = mmap(NULL, sizeof(int) * rows * cols * 3, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0); 

    if ((file1 = fopen(filename1, "r")) == NULL){
        printf("Failed to open '%s' file\n", filename1);
        return 1;
    }
    int number, counter = 0; 
    char trashline[5];
    fgets (trashline, 100, file1);
    while(!feof (file1)){
        fscanf (file1, "%d", &number);
        *(shared + counter) = number;
        counter++;
    }
    fclose(file1);

    if ((file2 = fopen(filename2, "r")) == NULL){
        printf("Failed to open '%s' file\n", filename2);
        return 1;
    }
    fgets (trashline, 100, file2);
    while(!feof (file2)){
        fscanf (file2, "%d", &number);  
        *(shared + counter) = number;
        counter++;
    }
    fclose(file2);

    int offset = 0, matrixDist = rows * cols, pid;
    int pid;
    for (int col = 0; col < cols; col++)
    {   
        if ((pid = fork()) == 0)     // child process
        {
            for(int row = 0; row < rows; row++){
                *(shared + col + 2 * matrixDist + cols * row) = *(shared + col + matrixDist + cols * row) + *(shared + col + cols * row);
            }
            exit(0);
        }
    }

    /* Wait for children */
    int corpse;
    int status;
    while ((corpse = wait(&status)) > 0){
        
    }
    printf("%dx%d\n",rows, cols);
    for(int i = rows * cols * 2; i < rows * cols * 3; i++){
        printf("%d ", *(shared + i));
    if((i - rows * cols * 2) % cols == cols - 1){
            printf("\n");
        }
    } 
    printf("\n");
    return 0;
}

int getMatrixInfo(char *filename1, char *filename2, int *rows, int *cols){
    FILE *file1, *file2;
    char c;
    int row1, row2, col1, col2;

    if ((file1 = fopen(filename1, "r")) == NULL){
        printf("Failed to open '%s' file\n", filename1);
        return 1;
    }
    fscanf(file1, "%i %c %i", &row1, &c, &col1);
    fclose(file1);

    if ((file2 = fopen(filename2, "r")) == NULL){
        printf("Failed to open '%s' file\n", filename2);
        return 1;
    }
    fscanf(file2, "%i %c %i", &row2, &c, &col2);
    fclose(file2);

    *rows = row1;
    *cols = col1;
    return (row1 != row2 || col1 != col2);
}
