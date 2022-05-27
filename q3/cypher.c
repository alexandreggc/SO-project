#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdbool.h>

#define MAX_WORD_SIZE 256
#define MAX_LINE_SIZE 1024

int switchWords(char* old_text, char* new_test, const char* old_word, const char* new_word) {
    char* result;
    int i, n_cnt = 0, o_cnt = 0;
    int newWlen = strlen(new_word);
    int oldWlen = strlen(old_word);

    // counting the number of times the cyphers appear in the old_text
    for (i = 0; old_text[i] != '\0'; i++) {
        if (strstr(&old_text[i], old_word) == &old_text[i]) {
            n_cnt++;
            i += oldWlen - 1;
        }
        if (strstr(&old_text[i], new_word) == &old_text[i]) {
            o_cnt++;
            i += newWlen - 1;
        }
    }
    // making new string of enough length to change the cypher strings
    result = (char*)malloc(i + n_cnt * (newWlen - oldWlen) + o_cnt * (oldWlen - newWlen) + 1);
    
    i = 0;
    // compares the cyphers with the text and switch when they are matched
    while (*old_text) {
        if (strstr(old_text, old_word) == old_text) {
            strcpy(&result[i], new_word);
            i += newWlen;
            old_text += oldWlen;
        }
        else if (strstr(old_text, new_word) == old_text){
            strcpy(&result[i], old_word);
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
        strncat(text, &ch, 1);
    }
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
        sscanf( buff, "%s %s", &k, &val);
        strcpy(keys[i], k);
        strcpy(vals[i], val);
        i++;
    }
    fclose(cyphers);
    return 0;
}

int main() {
    int no_cyphers;
    if (no_lines("cypher.txt", &no_cyphers)) return 1;

    // create keys and values arrays to store the cyphers
    char** keys = malloc(no_cyphers * sizeof(char*));
    for (int i = 0; i < no_cyphers; i++){
        keys[i] = malloc(MAX_WORD_SIZE * sizeof(char));
    }
    char** vals = malloc(no_cyphers * sizeof(char*));
    for (int i = 0; i < no_cyphers; i++){
        vals[i] = malloc(MAX_WORD_SIZE * sizeof(char));
    }

    if (load_cyphers(keys, vals)) return 1;

    int fd1[2];
    int fd2[2];
    int pid;
    ssize_t numRead;
    
    if (pipe(fd1) == -1) {
        fprintf(stderr, "Pipe 1 Failed");
        return 1;
    }
    if (pipe(fd2) == -1) {
        fprintf(stderr, "Pipe 2 Failed");
        return 1;
    }
 
    if ((pid = fork()) < 0) {
        fprintf(stderr, "fork Failed");
        return 1;
    }

    // parent process
    else if (pid > 0) {
        // close reading end of first pipe
        close(fd1[0]); 
        // write input text and close writing end of first pipe.
        write_to_pipe(fd1[1]);
        close(fd1[1]);
        
        // wait for child to send the text
        wait(NULL);
 
        close(fd2[1]); // close writing end of second pipe
        // read text from child, print it to stdout and close
        read_from_pipe(fd2[0], stdout);
        close(fd2[0]);
    }
 
    // child process
    else {
        close(fd1[1]);
        close(fd2[0]);
        char *res = NULL;
        char str[MAX_LINE_SIZE];

        while (1) {
            numRead = read(fd1[0], str, MAX_LINE_SIZE);
            if (numRead == -1) {
                perror("child process: partial/failed read from pipe 1");
                exit(EXIT_FAILURE);
            }
            if (numRead == 0) break;
            char* new_str;
            for (int i=0; i < no_cyphers; i++){
                char* k = keys[i];
                char* val = vals[i];
                switchWords(str, new_str, k, val);
                strcpy(str, new_str);
                strcpy(new_str, "");
            }
            if (write(fd2[1], str, numRead) != numRead){
                perror("child process: partial/failed write to pipe 2");
                exit(EXIT_FAILURE);
            }
        }
        close(fd1[0]);
        exit(0);
    }

    //free alocated memory
    for (int i = 0; i < no_cyphers; i++){
        free(keys[i]);
        free(vals[i]);
    }
    free(keys);
    free(vals);
    return 0;
}
