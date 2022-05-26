#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/mman.h>

#define MAX_WORD_SIZE 256
#define MAX_LINE_SIZE 1024

int replaceWord(char* old_text, char* new_test, const char* old_word, const char* new_word) {
    char* result;
    int i, n_cnt = 0, o_cnt = 0;
    //printf("passoui %s %s\n", old_word, new_word);
    int newWlen = strlen(new_word);
    //printf("passoui\n");
    int oldWlen = strlen(old_word);
    //printf("passoui\n");
    // Counting the number of times old word
    // occur in the string
    for (i = 0; old_text[i] != '\0'; i++) {
        if (strstr(&old_text[i], old_word) == &old_text[i]) {
            n_cnt++;
            // Jumping to index after the old word.
            i += oldWlen - 1;
        }
        if (strstr(&old_text[i], new_word) == &old_text[i]) {
            o_cnt++;
            i += newWlen - 1;
        }
    }
    //printf("passoui\n");
    // Making new string of enough length
    result = (char*)malloc(i + n_cnt * (newWlen - oldWlen) + o_cnt * (oldWlen - newWlen) + 1);
  
    i = 0;
    while (*old_text) {
        // compare the substring with the result
        if (strstr(old_text, old_word) == old_text) {
            //printf("\n--analise 1: %s\n\n", old_text);
            strcpy(&result[i], new_word);
            //printf("\n--resultado 1: %s\n\n", result);
            i += newWlen;
            old_text += oldWlen;
        }
        else if (strstr(old_text, new_word) == old_text){
            //printf("\n--analise 2: %s\n\n", old_text);
            strcpy(&result[i], old_word);
            //printf("\n--resultado 2: %s\n\n", result);
            i += oldWlen;
            old_text += newWlen;
        }
        else{
            result[i++] = *old_text++;
        }
    }
    result[i] = '\0';

    size_t size = strlen(result);
    char text[MAX_LINE_SIZE] = "";
    for (int index=0; index < size; index++){
        char ch = result[index];
        //printf("passoui %c\n", ch);
        strncat(text, &ch, 1);
        //printf("passoui\n");
    }
    //printf("%s\n\n", text);
    free(result);
    strcpy(new_test, text);
    return 0;
}

int write_to_pipe(int pipe){
    char line[MAX_LINE_SIZE];
    while(fgets (line, MAX_LINE_SIZE, stdin)){
        write(pipe, line, strlen(line));
    }
    return 0;
}

int read_from_pipe(int pipe, FILE* stream){
    int numRead;
    char line[MAX_LINE_SIZE];
    while (1) {
        numRead = read(pipe, line, MAX_LINE_SIZE);
        if (numRead == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }
        if (numRead == 0) break;
        fprintf(stream, line);
        fprintf(stream, "\n");
    }
    return 0;
}

int no_lines(char *filename, int* no_lines){
    FILE* file;
    if ((file = fopen(filename, "r")) == NULL){
        printf("Failed to open file\n");
        return 1;
    }
    char buff[MAX_LINE_SIZE];
    (*no_lines) = 0;
    while(fgets (buff, MAX_LINE_SIZE, file)){
        (*no_lines)++;
    }
    fclose(file);
    return 0;
}

int load_cyphers(char ** keys, char** vals){
    char buff[MAX_LINE_SIZE];
    FILE* cyphers;
    if ((cyphers = fopen("cypher.txt", "r")) == NULL){
        printf("Failed to open file\n");
        return 1;
    }
    rewind(cyphers);
    int i = 0;
    char k[MAX_WORD_SIZE]; char val[MAX_WORD_SIZE];
    while(fgets (buff, MAX_LINE_SIZE, cyphers)){
        //printf("antes passou i = %d\n", i);
        sscanf( buff, "%s %s", &k, &val);
        //printf("passou i = %d   %s    %s\n", i, k, val);
        strcpy(keys[i], k);
        strcpy(vals[i], val);
        //printf("passou i = %d\n   %s    %s\n", i, keys[i], vals[i]);
        i++;
    }
    //printf("i = %d\n", i);
    fclose(cyphers);
    //printf("passou i = %d\n", i);
    return 0;
}

int main() {
    int no_cyphers;
    if (no_lines("cypher.txt", &no_cyphers)) return 1;

    // create keys and values arrays to store the cyphers
    //char** keys = mmap(NULL, 2 * no_cyphers * sizeof(char*), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    char** keys = malloc(no_cyphers * sizeof(char*));
    for (int i = 0; i < no_cyphers; i++){
        keys[i] = malloc(MAX_WORD_SIZE * sizeof(char));
    }
    char** vals = malloc(no_cyphers * sizeof(char*));
    for (int i = 0; i < no_cyphers; i++){
        vals[i] = malloc(MAX_WORD_SIZE * sizeof(char));
    }

    if (load_cyphers(keys, vals)) return 1;
    for (int i=0; i<no_cyphers; i++){
        //printf("no main   %s    %s\n", keys[i], vals[i]);
    }
    //printf("passou\n");

    int fd1[2];
    int fd2[2];
    int pid;
    ssize_t numRead;
    
    

    if (pipe(fd1) == -1) {
        fprintf(stderr, "Pipe Failed");
        return 1;
    }
    if (pipe(fd2) == -1) {
        fprintf(stderr, "Pipe Failed");
        return 1;
    }
 
    if ((pid = fork()) < 0) {
        fprintf(stderr, "fork Failed");
        return 1;
    }
    // Parent process
    else if (pid > 0) {
        close(fd1[0]); // Close reading end of first pipe

        // Write input string and close writing end of first
        // pipe.
        write_to_pipe(fd1[1]);
        close(fd1[1]);
        
        // Wait for child to send a string
        wait(NULL);
 
        close(fd2[1]); // Close writing end of second pipe
 
        // Read string from child, print it and close
        // reading end.
        read_from_pipe(fd2[0], stdout);
        close(fd2[0]);
    }
 
    // child process
    else {
        //printf("in child process\n");
        close(fd1[1]);
        close(fd2[0]);
        char *res = NULL;
        char str[MAX_LINE_SIZE];
        //printf("passou\n");

        while (1) {
            numRead = read(fd1[0], str, MAX_LINE_SIZE);
            if (numRead == -1) {
                perror("read");
                exit(EXIT_FAILURE);
            }
            if (numRead == 0) break;
            //printf("before loop cyphers: %d\n", no_cyphers);
            char* new_str;
            for (int i=0; i < no_cyphers; i++){
                //printf("watching word: %s \n", keys[i]);
                char* k = keys[i];
                char* val = vals[i];
                //strcpy(k, keys[i]);
                //strcpy(val, vals[i]);
                //printf("old word: %s   new word: %s \n", k, val);
                replaceWord(str, new_str, k, val);
                strcpy(str, new_str);
                strcpy(new_str, "");
            }
            if (write(fd2[1], str, numRead) != numRead){
                perror("write - partial/failed write");
                exit(EXIT_FAILURE);
            }
            //printf("read data: %s\n", str);
        }
        close(fd1[0]);
        exit(0);
    }
    return 0;
}
