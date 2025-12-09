#include<stdio.h>
#include<string.h>
#include<stdbool.h>
#include"markov_chain.h"

#define FILE_PATH_ERROR "Error: incorrect file path"
#define NUM_ARGS_ERROR "Usage: invalid number of arguments"
#define MAX_CHARACTERS 1000

#define DELIMITERS " \n\t\r"
#define LENGTH_OF_TWEET 20
#define ARGV_SEED 1
#define ARGV_NUMBER_OF_TWEETS 2
#define ARGV_FILE_PATH 3
#define AGRV_MAX_WORDS_TO_READ 4

bool is_last(void* ch){
    char *word = (char *) ch;
    return word[strlen(ch) - 1] == '.';
}

void* copy_string(void* source){
    char* tmp = (char*) source;
    char* target = malloc(strlen(tmp) * sizeof(char) + 1);
    if(target == NULL){
        printf(ALLOCATION_ERROR_MASSAGE);
        return NULL;
    }
    strcpy(target,(char*)source);
    return target;
}
void print_string(void* data){
    char* word = (char*) data;
    printf("%s ", word);
}

int comp_strings(void* string_1, void* string_2){
    return strcmp((char*)string_1, (char*)string_2);
}
void free_data(void* data){
    free(data);
}

void string_to_null(char *str) {
    memset(str, '\0', MAX_CHARACTERS);
}

int fill_database(FILE *fp, int words_to_read, MarkovChain *markov_chain){
    char sentence[MAX_CHARACTERS] = {"\0"};
    int words_counter = 0;
    char* token;
    Node *current = NULL, *previous = NULL;
    while (fgets(sentence, sizeof(sentence),fp) != NULL) {
        if (words_to_read == words_counter){
            break;
        }
        else{
            token = strtok(sentence, DELIMITERS);
            while (token != NULL) {
                previous = current;
                current = add_to_database(markov_chain, token);
                if (current != NULL && previous != NULL && !markov_chain->is_last(previous->data->data)){
                    add_node_to_frequency_list(previous->data, current->data, markov_chain);
                }
                token = strtok(NULL, DELIMITERS);
                words_counter++;
                if (words_to_read == words_counter){
                    break;
                }
            }
            string_to_null(sentence);
        }
    }
    return 0;
}

int main(int argc, char* argv[]) {
    if(argc != 5 && argc != 4){
        printf(NUM_ARGS_ERROR);
        return 1;
    }
    int number_of_words_to_read = -1;
    if (argc == 5){
        number_of_words_to_read = strtol(argv[AGRV_MAX_WORDS_TO_READ], NULL, 10);
    }
    int seed = strtol(argv[ARGV_SEED], NULL, 10);
    int number_of_tweets = strtol(argv[ARGV_NUMBER_OF_TWEETS], NULL, 10);
    LinkedList *database = malloc(sizeof(LinkedList));
    if (database == NULL){
        printf(ALLOCATION_ERROR_MASSAGE);
        return 1;
    }
    MarkovChain* markov_chain = malloc(sizeof(MarkovChain));
    if (markov_chain == NULL){
        printf(ALLOCATION_ERROR_MASSAGE);
        return 1;
    }
    markov_chain->database = database;
    markov_chain->is_last = is_last;
    markov_chain->print_func = print_string;
    markov_chain->free_data = free_data;
    markov_chain->comp_func = comp_strings;
    markov_chain->copy_func = copy_string;
    FILE *file = fopen(argv[ARGV_FILE_PATH], "r");
    if (file == NULL) {
        printf("%s\n", FILE_PATH_ERROR);
        return 1;
    }
    fill_database(file, number_of_words_to_read, markov_chain);
    srand(seed);
    for (int i = 0; i < number_of_tweets; i++){
        printf("Tweet %i: ", i + 1);
        generate_random_sequence(markov_chain,get_first_random_node(markov_chain), LENGTH_OF_TWEET);
    }
    free_markov_chain(&markov_chain);
    fclose(file);
    return 0;
}