#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdbool.h>
#include "hashmap.h"

#define MAX_WORD_SIZE 200
#define MAX_STR_SIZE 1000

 
struct cypher {
    char *key;
    char *value;
};

int cypher_compare(const void *a, const void *b, void *udata) {
    const struct cypher *ua = a;
    const struct cypher *ub = b;
    return strcmp(ua->key, ub->key);
}

bool cypher_iter(const void *item, void *udata) {
    const struct cypher *cypher = item;
    //printf("%s (value=%s)\n", cypher->key, cypher->value);
    return true;
}

uint64_t cypher_hash(const void *item, uint64_t seed0, uint64_t seed1) {
    const struct cypher *cypher = item;
    return hashmap_sip(cypher->key, strlen(cypher->key), seed0, seed1);
}

char* replaceWord(const char* s, const char* oldW, const char* newW) {
    
    //char* result;
    int i, cnt = 0;
    int newWlen = strlen(newW);
    int oldWlen = strlen(oldW);
  
    // Counting the number of times old word
    // occur in the string
    for (i = 0; s[i] != '\0'; i++) {
        if (strstr(&s[i], oldW) == &s[i]) {
            cnt++;
  
            // Jumping to index after the old word.
            i += oldWlen - 1;
        }
    }
  
    // Making new string of enough length
    //result = (char*)malloc(i + cnt * (newWlen - oldWlen) + 1);
    char result[i + cnt * (newWlen - oldWlen) + 1];

    i = 0;
    while (*s) {
        // compare the substring with the result
        if (strstr(s, oldW) == s) {
            strcpy(&result[i], newW);
            i += newWlen;
            s += oldWlen;
        }
        else
            result[i++] = *s++;
    }
  
    result[i] = '\0';
    return result;
}

int write_to_pipe(int pipe, int* no_lines){
    char line[MAX_STR_SIZE];
    *no_lines = 0;
    while(fgets (line, MAX_STR_SIZE, stdin)){
        write(pipe, line, strlen(line));
        *no_lines++;
    }
    return 0;
}

int read_from_pipe(int pipe, FILE* stream){
    int numRead;
    char line[MAX_STR_SIZE];
    while (1) {
        numRead = read(pipe, line, MAX_STR_SIZE);
        if (numRead == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }
        if (numRead == 0) break;
        fprintf(stream, line);
    }
    return 0;
}

int load_map(struct hashmap *map){
    FILE* cyphers;
    char k[MAX_WORD_SIZE];
    char val[MAX_WORD_SIZE];
    if ((cyphers = fopen("cypher.txt", "r")) == NULL){
        printf("Failed to open file\n");
        return 1;
    }
    printf("load map loop\n");
    while(fscanf(cyphers, "%s %s", &k, &val) != EOF){
        //printf("%s %s\n", k, val);
        //printf("passou\n");
        hashmap_set(map, &(struct cypher){ .key=k, .value=val });
        //printf("passou\n");
        //hashmap_set(map, &(struct cypher){ .key=val, .value=k });
    }
    struct cypher *c = hashmap_get(map, &(struct cypher){ .key="night" });
    //printf("key %s len: %d\n", k, strlen(k));
    //printf("%s\n", c?"exists":"not exists");
    //printf("%s %s\n", c->key, c->value);
    size_t iter = 0;
    void *item;
    //printf("\n-- iterate over all users (hashmap_iter) --\n");
    while (hashmap_iter(map, &iter, &item)) {
        const struct cypher *c = item;
        //printf("%s (age=%s)\n", c->key, c->value);
    }
    return 0;
}

int main() {
    struct hashmap *map = hashmap_new(sizeof(struct cypher), 0, 0, 0, cypher_hash, cypher_compare, NULL, NULL);
    load_map(map);
    
    // We use two pipes
    // First pipe to send input string from parent
    // Second pipe to send concatenated string from child
 
    int fd1[2]; // Used to store two ends of first pipe
    int fd2[2]; // Used to store two ends of second pipe
    int no_lines, pid;
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
        char concat_str[100];
        close(fd1[0]); // Close reading end of first pipe
 
        // Write input string and close writing end of first
        // pipe.
        write_to_pipe(fd1[1], &no_lines);
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
        printf("in child process\n");
        close(fd1[1]);
        close(fd2[0]);
        char *res = NULL;
        char str[MAX_STR_SIZE];
        printf("passou\n");

        while (1) {
            numRead = read(fd1[0], str, MAX_STR_SIZE);
            if (numRead == -1) {
                perror("read");
                exit(EXIT_FAILURE);
            }
            if (numRead == 0)
                break;
            if (write(fd2[1], str, numRead) != numRead){
                perror("write - partial/failed write");
                exit(EXIT_FAILURE);
            }
            //printf("read data: %s\n", str);
        }
        /*
        while (hashmap_iter(map, &iter, &item)) {
            const struct cypher *cypher = item;
            printf("%s (value=%s)\n", cypher->key, cypher->value);
            //str = replaceWord(str, cypher->key, cypher->value);
            printf("completou um ciclo: %s\n", str);
        }
        */
        close(fd1[0]);
        exit(0);
    }
    
    hashmap_free(map);
    return 0;
}
