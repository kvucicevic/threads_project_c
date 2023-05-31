#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <malloc.h>
#include "defs.h"

pthread_mutex_t fileMutex;

int fileChanged(char* fileName);
void my_strtok(char* str, const char* delimiters, char* words[]);

int countElements(char* arr[]) {
    int count = 0;
    while (arr[count] != NULL) {
        count++;
    }
    return count;
}

int container(char* input){  /// koji je mode

    char inputcpy[20];
    strcpy(inputcpy, input);
    char* word = strtok(inputcpy, " ");

    if(strcmp(word, "_count_") == 0){           // filePath
        return 5;
    } else if(strcmp(word, "_stop_") == 0){     // stop the prog
        return -1;
    } else {                                    // keyword
        return 8;
    }

}

unsigned long hash_djb2(char* str) {  /// implemented a djb2 algorithm for hashing a string
    unsigned long hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }

    return hash;
}

void putFile(char* filePath, char* input){     // uzima samo poslednje uneto za fileName

    char* delimiters = " ";
    char* words[BUFFER_SIZE];
    my_strtok(input, delimiters, words);

    int size = countElements(words);

    strcpy(filePath, words[--size]);
}

void my_strtok(char* str, const char* delimiters, char** words) {
    int numWords = 0;
    char helpStr[BUFFER_SIZE];
    strcpy(helpStr, str);
    char* token = strtok(helpStr, delimiters);

    while (token != NULL) {
        words[numWords] = (char*)malloc(strlen(token) + 1);
        words[numWords] = strdup(token);
        numWords++;
        token = strtok(NULL, delimiters);
    }
}


void *scanner_work(void *_args){ //funkcija scanner niti,

    scanned_file* scannedFile = (scanned_file*) _args;

    FILE* file = fopen(scannedFile->file_name, "r");

    if (file == NULL) {
        fprintf(stderr, "Failed to open the file: %s\n", scannedFile->file_name);
        return NULL;
    }

    printf("File is: %s\n", scannedFile->file_name);

    size_t bytesRead = fread(scannedFile->buffer, sizeof(char), BUFFER_SIZE - 1, file);
    if (bytesRead > 0) {
        scannedFile->buffer[bytesRead] = '\0';  // Add null terminator
    }


    ///mapping
    char helpBuf[BUFFER_SIZE];
    strcpy(helpBuf, scannedFile->buffer);

    const char delimiters[] = " \t\n";

    char* store[BUFFER_SIZE];

    my_strtok(helpBuf, delimiters, store);

    int numElements = countElements(store);

    // Create an array of hash_map structures
    hash_map* map = (hash_map*)malloc(numElements * sizeof(hash_map));
    if (map == NULL) {
        fprintf(stderr, "Failed to allocate memory for hash_map\n");
        return NULL;
    }


    for (int i = 0; i < numElements; i++) {
        // Allocate memory for the word

        map[i].word = strdup(store[i]);
        if (map[i].word == NULL) {
            fprintf(stderr, "Failed to allocate memory for word\n");
            break;
        }

        // Assign the hash value
        map[i].hash = hash_djb2(store[i]);
    }


    // Print the hash_map array
    for (int i = 0; i < numElements; i++) {
        printf("Word: %s, Hash: %d\n", map[i].word, map[i].hash);
    }



    /*
    while(1){
        pthread_mutex_lock(&fileMutex);
        if (fileChanged(scannedFile->file_name) == 8) {
            // Perform file iteration and mapping again
            printf("ye");
        }
        printf("no");
        pthread_mutex_unlock(&fileMutex);

        // Sleep for 5 seconds
        sleep(5);
    }
     */

    fclose(file);


    // logika za pretragu reci, stavljanje fajla u procitane


    return NULL;
}

int fileChanged(char* fileName){

    struct stat fileStat;
    if (stat(fileName, &fileStat) == -1) {
        perror("Failed to get file status");
        return -1;
    }

    time_t previousModTime = fileStat.st_mtime;

    if (stat(fileName, &fileStat) == -1) {
        perror("Failed to get file status");
        return -1;
    }

    time_t currentModTime = fileStat.st_mtime;

    if (currentModTime != previousModTime) {
        return 8;   /// file has changed
    } else {
        return 5;   /// file has NOT changed
    }

}


int main() {

    char input[20];
    printf("Command: ");
    gets(input);


    while(container(input) != -1) {

        char filePath[30];
        char fileData[100];

        putFile(filePath, input);

        pthread_t scanTh;

        if (container(input) == 5) {

            scanned_file scannedFile;
            strcpy(scannedFile.file_name, filePath);
            strcpy(scannedFile.buffer, fileData);


            if (pthread_create(&scanTh, NULL, scanner_work, (void*)&scannedFile) != 0) {
                fprintf(stderr, "Failed to create the thread.\n");
                return 1;
            }

            // Wait for the thread to finish
            if (pthread_join(scanTh, NULL) != 0) {
                fprintf(stderr, "Failed to join the thread.\n");
                return 1;
            }

            pthread_mutex_destroy(&fileMutex);
            printf("Contents of the file:\n%s\n", scannedFile.buffer);

        }

        printf("Command: ");
        gets(input);
    }


    return 0;
}
