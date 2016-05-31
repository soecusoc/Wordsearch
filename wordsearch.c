#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define WORD_SIZE 20
#define BYTES_PER_WORD_AV 6.1
#define NUM_OF_INTERESTING_WORDS 100
#define TRUE 1
#define EMPTY_SPACE -1

//Row of a hash table
struct TableRow{
    //Hash table will be huge, char is smallest number in bytes
    //Words have a fixed size of WORD_SIZE
    char word[WORD_SIZE];
    int numOfOccurence;
};

//Row in the table of most common words
struct FrequentWord{
    int numOfOccurence;
    //Word itself will be in hash table, pointer to it is be enough
    char* pItself;
};

struct Word{
    char itself[WORD_SIZE];
    int length;
};

//Makes a minimum heap from given FrequentWord table
void buildMinHeap(FrequentWord* table, int tableSize);
//Approximates the amount of words in file by the size of file
//Defines the appropriate size for the hash table from this information
int defineHashTableSize(FILE* file);
//Hashes all words in file and puts them in hash table
void fillHashTableFromFile(FILE* file, TableRow* hashTable, int hashTableSize);
//Checks the next word in file and returns it
Word getNextWordInFile(FILE* file);
//Calculates the hash value of given word
int hashWord(char word[], int wordlength, int hashTableSize);
//Checks if given character is a letter of the alphabet or '
int isAChar(char asciiCode);
//Makes a minimum heap out of a given table, if all its sub trees are minimum heaps
void minHeapify(FrequentWord* table, int index, int heapSize);
//Opens a file based on the command line arguments and returns a pointer to it
FILE* openFile(int argc, char* argv[]);
//Integer to the power of integer, quite a fast way to calculate powers with integers
int power(int base, int exp);
//Special way of printing a word from a given char table
void printWord(char word[]);
//Puts a given word on its own place in a hash table according to the given hash key
void putWordInHashTable(TableRow* hashTable, int hashTableSize, int key, char word[], int wordlength);
//Puts given word in minimum heap as a new root discarding the old root
//Sorts the table as a minimum heap
void putWordInMinHeap(FrequentWord* heap, int heapSize, int numOfOccurence, char* pWord);
//Sorting algorithm for a FrequentWord table using minimum heap method
void sortTableWithMinHeap(FrequentWord* table, int tableSize);
//Switches the values of two words with each other
void switchFrequentWords(FrequentWord* word1, FrequentWord* word2);
//Makes uppercase out of given letter of alphabet
char uppercase(char character);

//BEGINNING OF MAIN
int main(int argc, char* argv[]){
    //Run time analysis
    clock_t start = clock(), diff;
    
    int i, j;
    int hashTableSize;
    FILE* file;
    //Reserves table for most common words
    //First cell is "empty" for indexing to start from 1 instead of 0
    FrequentWord mostFrequentWords[NUM_OF_INTERESTING_WORDS + 1];
    //Null pointers for safety reasons
    for(i = 0; i <= NUM_OF_INTERESTING_WORDS; i++) mostFrequentWords[i].pItself = 0;
    
    file = openFile(argc, argv);
    
    hashTableSize = defineHashTableSize(file);
    //Reserves a hash table of appropriate size
    TableRow hashTable[hashTableSize];
    //Sets number of occurences to 0 for all words
    for(i = 0; i < hashTableSize; i++) hashTable[i].numOfOccurence = 0;
    
    fillHashTableFromFile(file, hashTable, hashTableSize);
    
    fclose(file);

    i = 0;
    j = 1;
    //This loop fills the most common words table from the hash table
    //Loop ends, when the most common words table is full or hash table is empty
    //Index i runs through the hash table keys
    while(i < hashTableSize && j <= NUM_OF_INTERESTING_WORDS){
        //If number of occurences is not 0, there's a word from file in hash table at this key
        if(hashTable[i].numOfOccurence != 0){
            //Simply puts the word in most common words
            mostFrequentWords[j].numOfOccurence = hashTable[i].numOfOccurence;
            mostFrequentWords[j].pItself = &hashTable[i].word[0];
            //Keeps count on different words
            j++;  
        }
    i++;
    }
    
    //Most common words is full or hash table is empty
    //Make a minimum heap out of the most common words table
    buildMinHeap(mostFrequentWords, NUM_OF_INTERESTING_WORDS + 1);
    
    //Now that the most common words is a minimum heap
    //This while-loop checks the words from hash table that didn't fit in most common words
    while(i < hashTableSize){
        //Again checks, if the sequence in hash table is a word from file
        if(hashTable[i].numOfOccurence != 0){
            //Keep on counting different words, starts from 101
            j++;
            //Checks, if the next word found is more common than the least common word in
            //the most common words table
            //If the found word is more common, it must be included and the least common word
            //in the most common words table must be discarded
            if(hashTable[i].numOfOccurence > mostFrequentWords[1].numOfOccurence){
                putWordInMinHeap(mostFrequentWords, NUM_OF_INTERESTING_WORDS + 1, hashTable[i].numOfOccurence, &hashTable[i].word[0]);
            }
        }
    i++;
    }
    
    //Most common words has been found. Now it's time to put them in order
    sortTableWithMinHeap(mostFrequentWords, NUM_OF_INTERESTING_WORDS + 1);

    printf("Number of different words: %d\n", j - 1);
    printf("The %d most common words:\n", NUM_OF_INTERESTING_WORDS);
    
    //Print the most common words
    //printWord-function is used for printing the words from char pointer
    for(i = 1; i <= NUM_OF_INTERESTING_WORDS; i++){
        //In case of total of different words is less than NUM_OF_INTERESTING_WORDS
        //there will be null pointers with junk information
        if(mostFrequentWords[i].pItself == 0) continue;
        printWord(mostFrequentWords[i].pItself);
        printf(" %d\n", mostFrequentWords[i].numOfOccurence);
    }

    //Stops the timer for run time analysis
    diff = clock() - start;
    printf("Time taken %d seconds %d milliseconds", diff/1000, diff%1000);
    
    return 0;
}//END OF MAIN

void buildMinHeap(FrequentWord* table, int tableSize){
    int i;
    //This is the actual size of the table, first cell is "empty"
    tableSize = tableSize - 1;
    //This loop makes minimum heaps of every sub tree starting from trivial leaves-only subtrees
    for(i = tableSize / 2; i >= 1; i--) minHeapify(table, i, tableSize);
}

int defineHashTableSize(FILE* file){
    int size;
    int wordsInFile;
    int hashTableSize;
    double temp;

    //Finds the end of the file
    fseek(file, 0, SEEK_END);
    //Tells how many bytes there are at the end of file
    size = ftell(file);
    //Rewinds at the beginning of the file
    fseek(file, 0, SEEK_SET);

    //The number of words in file is approximated by dividing number of chars
    //(which is the size of the file in bytes) by word length average + 1
    //(+1 means the space between each word)
    //Gives an OK approximation without having to actually count the words
    temp = size / BYTES_PER_WORD_AV;
    wordsInFile = (int)temp;
    hashTableSize = wordsInFile;
    
    printf("Words in file: %d (approx)\n", wordsInFile);
    
    //Compresses the hash table based on the size of the file
    //Smaller hash table requires less memory but more collision occured
    //This is an approximation of a non-linear function
    //This is a little shady business
    if(wordsInFile < 10000) hashTableSize = 10000;
    if(wordsInFile >= 10000 && wordsInFile <50000) hashTableSize = hashTableSize / 5;
    if(wordsInFile >= 50000 && wordsInFile < 100000) hashTableSize = hashTableSize / 10;
    if(wordsInFile >= 100000 && wordsInFile < 500000) hashTableSize = hashTableSize / 15;
    if(wordsInFile >= 500000) hashTableSize = hashTableSize / 20;

    return hashTableSize;
}

void fillHashTableFromFile(FILE* file, TableRow* hashTable, int hashTableSize){
    int i;
    int key;    
    Word word;
    
    //Repeat infinitely
    do{
        word = getNextWordInFile(file);
        //If getNextWordInFile word with length -1, EOF has been reached
        if(word.length >= 0){
            //Uppercase, hash and put the found word the in hash table
            for(i = 0; i < WORD_SIZE; i++) word.itself[i] = uppercase(word.itself[i]);
            key = hashWord(word.itself, word.length, hashTableSize);
            putWordInHashTable(hashTable, hashTableSize, key, word.itself, word.length);
        }else{
            //EOF has been reach. Time to exit loop
            break;
        }
    }while(TRUE);
    
    //Rewind the file to the beginning
    fseek(file, 0, SEEK_SET);
}

Word getNextWordInFile(FILE* file){
    Word word;
    char character;
    int i = 0;
    int j;
    
    //Read the file char by char
    while((character = fgetc(file)) != EOF){
        //Searches for the next letter of the alphabet
        //That is the beginning of a new word
        if(isAChar(character)){
            //The beginning of the next word was found
            do{
                //The word has ended, character is not a letter of the alphabet
                if(!isAChar(character)) break;
                //Puts character at the end of the word being processed
                word.itself[i] = character;
                i++;
            }while((character = fgetc(file)) != EOF);
            //i has grown with every letter, so i is the length of the word
            word.length = i;
            //The rest of the spaces are filled with EMPTY_SPACE
            //Insides of the word:
            //word itself and enough empty spaces to total up to WORD_SIZE
            for(j = i; j < WORD_SIZE; j++){
                word.itself[j] = EMPTY_SPACE;
            }

            //Word is ready, exit loop without reaching EOF by returning word
            return word;
        }
    }
    
    //No words, just EOF was found
    for(j = 0; j < WORD_SIZE; j++){
        word.itself[i] = EMPTY_SPACE;
    }
    //Word length for EOF is -1, "error" state
    word.length = -1;

    return word;
}

int hashWord(char word[], int wordlength, int hashTableSize){
    int i;
    int hash = 0;
    
    //Hash function straight from the excerises
    for(i = 0; i < wordlength; i++){
        hash = hash + int(word[i]) * power(256, i);
        //%-operator makes sure the key is in hash table
        hash = hash % hashTableSize;
    }

    return hash;
}

int isAChar(char asciiCode){
    //Character must be letter of the alphabet or ' to be in a word
    //Checking for that
    if(asciiCode == 39 || 65 <= asciiCode &&  asciiCode <= 90 || 97 <= asciiCode && asciiCode <= 122){
        return 1; //return true
    }else{
        return 0; //return false
    }
}

void minHeapify(FrequentWord* table, int index, int heapSize){
    //Straight from lectures
    int smallest;
    int lft = 2 * index;
    int rgt = 2 * index + 1;
    
    if(lft <= heapSize && table[lft].numOfOccurence < table[index].numOfOccurence) smallest = lft;
    else smallest = index;
    if(rgt <= heapSize && table[rgt].numOfOccurence < table[smallest].numOfOccurence) smallest = rgt;
    if(smallest != index){
        //Notice the use of pointers: two elements must be changed
        switchFrequentWords(&table[index], &table[smallest]);
        //Recursive function
        minHeapify(table, smallest, heapSize);
    }
}

FILE* openFile(int argc, char* argv[]){
    //Checks, if filename was given as a command line argument.
    if(argc != 2){
        printf("Usage: %s filename", argv[0]);
        exit(EXIT_FAILURE);
    }
    
    FILE* file = fopen(argv[1], "r");
    //Checks, if file was opened succesfully.
    if(file == 0){
        printf("Could not open file \"%s\".", argv[1]);
        exit(EXIT_FAILURE);
    }
    
    return file;
}

int power(int base, int exp){
    int result = 1;
    int i;

    //Any real number to the power of 0 is 1
    if(exp == 0){
        return 1;
    }
    
    //For the non-trivial case, numbers are positive integers:
    //Multiply base with itself exp times
    for(i = 1; i <= exp; i++){
        result = result * base;
    }
    
    return result;
}

void printWord(char word[]){
    int i = 0;
    
    //Loop ends when EMPTY_SPACE has been reached in word
    //Word and only word is printed
    while(word[i] >= 0){
        printf("%c", word[i]);
        i++;
    }
}

void putWordInHashTable(TableRow* hashTable, int hashTableSize, int key, char word[], int wordlength){
    int i;
    
    //Unique word has been found and the key is not reserved
    if(hashTable[key].numOfOccurence == 0){
        hashTable[key].numOfOccurence = 1;
        //Word is put in hash table at the right key
        for(i = 0; i < WORD_SIZE; i++){
            hashTable[key].word[i] = word[i];
        }

    //Word is not unique or key is reserved
    }else{
        //IMPORTANT
        //i <= wordlength
        //Check also the next letter after the wordlength
        //Reason: Make difference with words that begin with other word
        //e.g. WASP != WASPS
        for(i = 0; i <= wordlength; i++){
            //Check for collision
            if(hashTable[key].word[i] != word[i]){
                //Collision occured
                //Linear probing with recursion
                key++;
                //Will not save outside the hash table
                if(!(key < hashTableSize)) key = 0;
                //Recursion
                putWordInHashTable(hashTable, hashTableSize, key, word, wordlength);
                //Break, if the words are the same
                break;
            }
        }
        
    //At this point i is actually too large because the for loop
    //increased it one too many times
    i--;
    
    //Another occurence of the non-unique word has been found
    if(i == wordlength) hashTable[key].numOfOccurence++;
    }
}

void putWordInMinHeap(FrequentWord* heap, int heapSize, int numOfOccurence, char* pWord){
    //Table indexing starts at 1, thus the decreasment
    heapSize = heapSize - 1;
    //Drops the word with least occurences from the heap
    //Puts the new word at the root of the tree
    heap[1].numOfOccurence = numOfOccurence;
    heap[1].pItself = pWord;
    //Every true subtree is a minimum heap
    //Only the root breaks the heap structure
    //That's, why there's no need to sort the whole tree, only the root
    minHeapify(heap, 1, heapSize);
}

void sortTableWithMinHeap(FrequentWord* table, int tableSize){
    //Straight from the lectures
    int i;

    //Again, indexing start from 1
    tableSize = tableSize -1;

    //In this program, table is already a minimum heap
    //The next command is not needed because of that
    //buildMinHeap(table, tableSize);
    for(i = tableSize; i >= 2; i--){
        switchFrequentWords(&table[1], &table[i]);
        tableSize = tableSize - 1;
        minHeapify(table, 1, tableSize);
    }
}

void switchFrequentWords(FrequentWord* word1, FrequentWord* word2){
    FrequentWord temp;
    
    //Old trick for switching two elements in place
    temp.numOfOccurence = word1->numOfOccurence;
    temp.pItself = word1->pItself;
    word1->numOfOccurence = word2->numOfOccurence;
    word1->pItself = word2->pItself;
    word2->numOfOccurence = temp.numOfOccurence;
    word2->pItself = temp.pItself;
}

char uppercase(char character){
    //If lowercase, make it uppercase by subracting 32 in ascii
    if(97 <= character && character <= 122) character = character - 32;

    return character;
}
