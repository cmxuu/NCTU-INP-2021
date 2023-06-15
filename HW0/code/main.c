#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char command[20];
char word[50];

void reverse(char *word);
void split(char *word, char *sc);

int main(int argc, char *argv[]){

		// argv[1]: fileName
		// argv[2]: splitChar

		// PART1: FILE
		printf("-------------------------Input file %s-------------------------\n", argv[1]);
        FILE *fp = fopen(argv[1], "r");

        while(fscanf(fp, "%s",command) == 1){
        	fscanf(fp, "%s", word);
        	printf("%s %s\n", command, word);
			if(strcmp(command, "reverse") == 0){
				reverse(word);
			}
			else if(strcmp(command, "split") == 0){
				split(word, argv[2]);
			}
			printf("\n");
        }
        fclose(fp);
        printf("-------------------------End of input file %s-------------------------\n", argv[1]);

        // PART2: USER
        printf("*****************************User input*****************************\n");
        while(1){
        	scanf("%s", command);
        	if(strcmp(command, "exit") == 0){
        		return 0;
        	}
        	scanf("%s", word);
        	if(strcmp(command, "reverse") == 0){
				reverse(word);
			}
			else if(strcmp(command, "split") == 0){
				split(word, argv[2]);
			}
			printf("\n");
        }
        return 0;
}

void reverse(char *word){
	for(int i = strlen(word) - 1; i >= 0; i--)
		printf("%c", word[i]);
}

void split(char *word, char *sc){
	char *pch = strtok(word, sc);
	while(pch != NULL){
		printf("%s ", pch);
		pch = strtok(NULL, sc);
	}
}