#include "defs.h"

pthread_mutex_t fileMutex;

map_file mapFiles[LETTERS];
int fileCounter = 0;


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

int isValidWord(char* word){

    for(int i = 0; word[i] != '\0'; i++){
        if(!isalpha(word[i])){
            return -1;
        }
    }

    for(int i = 0; word[i] != '\0'; i++) {  //uslov za 64
        word[i] = tolower(word[i]);
    }

    return 8;

}

void map_add_word_count(char *word, map_result* map, int mapSize){

    hash_map* hashMap = mapFiles[fileCounter].hashMap;

    for (int i = 0; i < mapSize; i++) {   /// size of hashmap
        // Check if the word already exists in the frequency map
        int found = 0;
        for (int j = 0; j < mapSize; j++) {   /// size of freqmap
            if (strcmp(hashMap[i].word, word) == 0) {
                map[j].frequency++;
                found = 1;
                break;
            }
        }

        // If the word doesn't exist in the frequency map, add a new entry
        if (!found) {
            map[mapSize].word = strdup(map[i].word);   /// maybe here should be just word
            map[mapSize].frequency = 1;
            map->maxSize = mapSize;
        }
    }
}

int fileChanged(char* fileName, time_t previousTimestamp){

    struct stat fileStat;

    if (stat(fileName, &fileStat) == -1) {
        // Error occurred while getting file information
        perror("stat");
        return -1;
    }

    // Initialize the previous timestamp on the first call
    if (previousTimestamp == 0) {
        previousTimestamp = fileStat.st_mtime;
        return 5;  // File has not changed (initial check)
    }

    // Check if the modification timestamp has changed
    if (fileStat.st_mtime != previousTimestamp) {
        previousTimestamp = fileStat.st_mtime;
        return 8;  // File has changed
    }

    return 5;  // File has not changed

}

hash_map* hashmapper(char* source){
    char helpBuf[BUFFER_SIZE];
    strcpy(helpBuf, source);

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


    int i;
    for (i = 0; i < numElements; i++) {
        // Allocate memory for the word

        if(isValidWord(store[i]) == 8) {  // ne znam kako da mu kazem da ne upisuje null-ove
            map[i].word = strdup(store[i]);

            if (map[i].word == NULL) {
                fprintf(stderr, "Failed to allocate memory for word\n");
                break;
            }

            // Assign the hash value
            map[i].hash = hash_djb2(store[i]);
        }

    }
    map->size = i;

    return map;
}

void *scanner_work(void *_args){ //funkcija scanner niti

    scanned_file* scannedFile = (scanned_file*) _args;

    FILE* file = fopen(scannedFile->file_name, "r");

    if (file == NULL) {
        fprintf(stderr, "Failed to open the file: %s\n", scannedFile->file_name);
        return NULL;
    }

    printf("File is: %s\n", scannedFile->file_name);

    struct stat fileInfo;

    // Get the file information
    if (stat(scannedFile->file_name, &fileInfo) == -1) {
        // Error occurred while getting file information
        perror("stat");
        return NULL;
    }

    scannedFile->mod_time = fileInfo.st_mtime;

    size_t bytesRead = fread(scannedFile->buffer, sizeof(char), BUFFER_SIZE - 1, file);
    if (bytesRead > 0) {
        scannedFile->buffer[bytesRead] = '\0';  // Add null terminator
    }


    ///mapping
    hash_map* map = hashmapper(scannedFile->buffer);

    map_file mapFile;
    mapFile.hashMap = map;
    strcpy(mapFile.fileName, scannedFile->file_name);

    mapFiles[fileCounter] = mapFile;

    // Print the hash_map array
    for (int j = 0; j < map->size; j++) {
        printf("Word: %s, Hash: %d, Size: %d \n", map[j].word, map[j].hash, map->size);
    }


    while(1){
        pthread_mutex_lock(&fileMutex);
        if (fileChanged(scannedFile->file_name, scannedFile->mod_time) == 8) {
            // Perform file iteration and mapping again
            printf("ye");
        }
        printf("no");
        pthread_mutex_unlock(&fileMutex);

        // Sleep for 5 seconds
        sleep(5);
    }


    fclose(file);


    // logika za pretragu reci, stavljanje fajla u procitane


    return NULL;
}


void *map_get_frequency(void* args){


    hash_map* hashMap = mapFiles[fileCounter].hashMap;
    char* word = (char*) args;

    map_result* map = (map_result*)malloc(BUFFER_SIZE * sizeof(map_result));
    if (map == NULL) {
        fprintf(stderr, "Failed to allocate memory for frequency map\n");
        return NULL;
    }

    for (int i = 0; i < hashMap->size; i++) {
        map[i].word = strdup(hashMap[i].word);
        map[i].frequency = 0;
    }

    map->maxSize = hashMap->size;

    map_add_word_count(word, map, hashMap->size);

    printf("word is: %s , frequency is %d\n", word, map->frequency);

    return map;

}

int main() {

    char input[20];
    printf("Command: ");
    gets(input);


    while(container(input) != -1) {

        char filePath[30];
        char fileData[100];

        putFile(filePath, input);

        scanned_file scannedFile;
        strcpy(scannedFile.file_name, filePath);
        strcpy(scannedFile.buffer, fileData);

        pthread_t scanTh;
        pthread_t mapTh;

        if (container(input) == 5) {


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

        } else if(container(input) == 8){



            if (pthread_create(&mapTh, NULL, map_get_frequency, (void*)&input) != 0) {
                fprintf(stderr, "Failed to create the thread.\n");
                return 1;
            }

            // Wait for the thread to finish
            if (pthread_join(mapTh, NULL) != 0) {
                fprintf(stderr, "Failed to join the thread.\n");
                return 1;
            }
        }

        printf("Command: ");
        gets(input);
    }


    return 0;
}
