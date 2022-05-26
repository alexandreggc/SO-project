#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_PHRASE_SIZE 512
#define MAX_STR_SIZE 128

int no_phrases(char* filename);

int main(int argc, char **argv) {
    if (argc < 2){
        printf("usage: phrases [-l] file\n");
        return 0;
    }

    bool extend_res;
    char str[MAX_LINE_SIZE];
    FILE *file;
    char *filename;

    if (strcmp(argv[1], "-l")==0 && argc > 2){
        filename = argv[2];
        extend_res = true;
    }
    else{
        filename = argv[1];
        extend_res = false;
    }
    
    if ((file = fopen(filename, "r")) == NULL){
        printf("Failed to open '%s' file\n", filename);
        return 1;
    }

    int num_phrases = no_phrases(filename);
    rewind(file);

    char** phrases = malloc(num_phrases * sizeof(char*));
    for (int i = 0; i < num_phrases; i++){
        phrases[i] = malloc(MAX_PHRASE_SIZE * sizeof(char));
    }

    int n = 0;
    while(fscanf(file, "%s", str) != EOF){
        strcat(phrases[n], " ");
        strcat(phrases[n], str);
        if(strchr(str, '.') || strchr(str, '?') || strchr(str, '!')){
            if (n+1 < num_phrases) n++;
        }
    }
    fclose(file);

    if(extend_res){
        for (int i=0; i < num_phrases; i++){
            printf("[%d]%s\n",i+1, phrases[i]);
        }
    }
    else{
        printf("%d\n", num_phrases);
    }

    return 0;
}

int no_phrases(char* filename){
    char c;
    int num = 0;
    bool in_phrase = false;
    FILE *file = fopen(filename, "r");

    while((c=fgetc(file)) != EOF){
        if (c != '.' && c != '!' && c != '?' && !in_phrase){
            num++;
            in_phrase = true;
        }
        else if((c == '.' || c == '!' || c == '?') && in_phrase){
            in_phrase = false;
        }
    }
    fclose(file);
    return num;
}
