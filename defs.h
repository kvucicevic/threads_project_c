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


typedef struct hash_map //cvor unutar trie strukture
{
    char* word; // reč
    unsigned int hash; // trenutna frekvencija
} hash_map;

typedef struct search_result //rezultat pretrage
{
    char* key; // reč
    int value; // trenutna frekvencija
} search_result;

typedef struct scanned_file //datoteka koju je scanner vec skenirao
{
    char file_name[30]; //naziv datoteke
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

extern void map_add_word_count(char *word, int value); //operacija za dodavanje reci i njihove frekvencije
extern search_result *map_get_frequency(char *word); //operacija za pretragu
extern void add_stopword(char* word); //dodaje rec u niz stop reci
