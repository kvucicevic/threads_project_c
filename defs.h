#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include <malloc.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>

#define MAX_WORD_LEN 64 //najveca dozvoljena duzina reci, uz \0
#define LETTERS 26 //broj slova u abecedi i broj dece u trie
#define BUFFER_SIZE 4096


typedef struct hash_map
{
    char* word; // reč
    unsigned int hash;
    int size;
} hash_map;

typedef struct map_result //rezultat pretrage
{
    char* word; // reč
    int frequency; // trenutna frekvencija
    int maxSize;
} map_result;

typedef struct map_file
{
    hash_map* hashMap;
    char fileName[MAX_WORD_LEN];
} map_file;

typedef struct scanned_file //fajl koji je scanner vec skenirao
{
    char file_name[30]; //naziv
    char buffer[BUFFER_SIZE];  //podaci iz fajla
    time_t mod_time; //vreme poslednje modifikacije datoteke
} scanned_file;

//my functions
extern int countElements(char* arr[]);
extern int container(char* input);
extern unsigned long hash_djb2(char* str);
extern void putFile(char* filePath, char* input);
extern void my_strtok(char* str, const char* delimiters, char* words[]);
extern int isValidWord(char* word);
extern int fileChanged(char* fileName);



extern void scanner_init(); //poziva se jednom na pocetku rada sistema
extern void *scanner_work(void *_args); //funkcija scanner niti

extern void map_init(); //poziva se jednom na pocetku rada sistema

extern void map_add_word_count(char *word, map_result* map, int map_size); //operacija za dodavanje reci i njihove frekvencije
extern void *map_get_frequency(void* args); //operacija za pretragu
extern void add_stopword(char* word); //dodaje rec u niz stop reci
