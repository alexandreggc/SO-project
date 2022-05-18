#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <math.h>


#include <string.h>
#include <stdlib.h>
#include <ctype.h>
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
    char *filename1 = argv[1];
    char *filename2 = argv[2];
    int rows;
    int cols;

    
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

    int offset = 0;
    int matrixDist = rows * cols;
    int nextColElem = cols;
    int pid;

   for (int i = 0; i < cols; i++)
    {
        if ((pid = fork()) == 0)     // child process
        {
            *(shared + offset + 2 * matrixDist) = *(shared + offset + matrixDist) + *(shared + offset);

            *(shared + offset + 2 * matrixDist + nextColElem) = *(shared + offset + matrixDist + nextColElem) + *(shared + offset + nextColElem);

            *(shared + offset + 2 * matrixDist + 2 * nextColElem) = *(shared + offset + matrixDist + 2 * nextColElem) + *(shared + offset + 2 * nextColElem);

            exit(0);            
        }
        offset++;
    }

    /* Wait for children */
    int corpse;
    int status;
    while ((corpse = wait(&status)) > 0){
        
    }
    printf("%dx%d\n",rows, cols);
    for(int i = rows * cols * 2; i < rows * cols * 3; i++){
        printf("%d ", *(shared + i));
        if(i % (rows * cols * 2) == cols - 1){
            printf("\n");
        }
    } 
    printf("\n");
    return 0;

}

int getMatrixInfo(char *filename1, char *filename2, int *rows, int *cols){
    FILE *file1, *file2;
    char c;
    char *row1 = malloc(sizeof(char) * MAX_NUM_CHARS);
    char *col1 = malloc(sizeof(char) * MAX_NUM_CHARS);
    char *row2 = malloc(sizeof(char) * MAX_NUM_CHARS);
    char *col2 = malloc(sizeof(char) * MAX_NUM_CHARS);
    int read_file = 1;
    int x = 0;
    int first_digit = 1;
    if ((file1 = fopen(filename1, "r")) == NULL){
        printf("Failed to open '%s' file\n", filename1);
        return 1;
    }

    while(read_file){
        c=fgetc(file1);
        if(c == '\n'){
            read_file = 0;
        }
        else if(c == 'x'){
            x = 1;
            first_digit = 1;
        }
        else{
            if(first_digit){
                if(x==0){
                    *row1 = c;
                }
                else{
                    *col1 = c;

                }
                first_digit = 0;
            }
            else{
                if(x==0){
                    *(row1+1) = c;
                }
                else{
                    *(col1+1) = c;
                }
                first_digit = 1;
            }
        }
    }
    fclose(file1);

    if ((file2 = fopen(filename2, "r")) == NULL){
        printf("Failed to open '%s' file\n", filename2);
        return 1;
    } 
    read_file = 1;
    x = 0;
    first_digit = 1;
    while(read_file){
        c=fgetc(file2);
        if(c == '\n'){
            read_file = 0;
        }
        else if(c == 'x'){
            x = 1;
            first_digit = 1;
        }
        else{
            if(first_digit){
                if(x==0){
                    *row2 = c;
                }
                else{
                    *col2 = c;

                }
                first_digit = 0;
            }
            else{
                if(x==0){
                    *(row2+1) = c;
                }
                else{
                    *(col2+1) = c;
                }
                first_digit = 1;
            }
        }
    }
    fclose(file2);
    size_t len = strlen(row2);
    if(strlen(row1) > strlen(row2)){
        len = strlen(row1);
    }
    for(int i = 0; i < len; i++){
        if(*(row1 + i) != *(row2 + i)){
            return 1;
        }
    }
    len = strlen(col2);
    if(strlen(col1) > strlen(col2)){
        len = strlen(col1);
    }
    for(int i = 0; i < len; i++){
        if(*(col1 + i) != *(col2 + i)){
            return 1;
        }
    }

    *rows = atoi(row1);
    *cols = atoi(col1);
    return 0;

}
