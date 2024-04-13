## Concurrent word counter (OS - HW4)
The purpose of this homework for to create a program that keeps track of word frequency. The user inputs path to files
and the words which frequencies he wants to read. The files are processes paralelly. The words are stored in one
central hash table which every thread has access to.

### User interaction

** _count_ <path/to/file> ** <br>
This command adds a new file to the system. A scanner thread for the file is created. 

** _stopwords_ <path/to/file> ** <br>
This command adds a file with stop words that will be ignored when scanning the files in the future.

** _stop_ **
Stops the whole proram and all the threads (Main thread and Scanner threads).

Run program from the terminal:
```
gcc main.c map.c -o program
./program
```
