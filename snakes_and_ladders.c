#include <string.h> // For strlen(), strcmp(), strcpy()
#include "markov_chain.h"

#define MAX(X, Y) (((X) < (Y)) ? (Y) : (X))

#define EMPTY -1
#define BOARD_SIZE 100
#define MAX_GENERATION_LENGTH 60

#define DICE_MAX 6
#define NUM_OF_TRANSITIONS 20
#define MAX_PATH 60
#define NUM_ARGS_ERROR "Usage: invalid number of arguments"

/**
 * represents the transitions by ladders and snakes in the game
 * each tuple (x,y) represents a ladder from x to if x<y or a snake otherwise
 */
const int transitions[][2] = {{13, 4},
                              {85, 17},
                              {95, 67},
                              {97, 58},
                              {66, 89},
                              {87, 31},
                              {57, 83},
                              {91, 25},
                              {28, 50},
                              {35, 11},
                              {8,  30},
                              {41, 62},
                              {81, 43},
                              {69, 32},
                              {20, 39},
                              {33, 70},
                              {79, 99},
                              {23, 76},
                              {15, 47},
                              {61, 14}};

/**
 * struct represents a Cell in the game board
 */
typedef struct Cell {
    int number; // Cell number 1-100
    int ladder_to;  // ladder_to represents the jump of the ladder in case there is one from this square
    int snake_to;  // snake_to represents the jump of the snake in case there is one from this square
    //both ladder_to and snake_to should be -1 if the Cell doesn't have them
} Cell;

/** Error handler **/
int handle_error_snakes(char *error_msg, MarkovChain **database)
{
    printf("%s", error_msg);
    if (database != NULL)
    {
        free_markov_chain(database);
    }
    return EXIT_FAILURE;
}


int create_board(Cell *cells[BOARD_SIZE])
{
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        cells[i] = malloc(sizeof(Cell));
        if (cells[i] == NULL)
        {
            for (int j = 0; j < i; j++) {
                free(cells[j]);
            }
            handle_error_snakes(ALLOCATION_ERROR_MASSAGE,NULL);
            return EXIT_FAILURE;
        }
        *(cells[i]) = (Cell) {i + 1, EMPTY, EMPTY};
    }

    for (int i = 0; i < NUM_OF_TRANSITIONS; i++)
    {
        int from = transitions[i][0];
        int to = transitions[i][1];
        if (from < to)
        {
            cells[from - 1]->ladder_to = to;
        }
        else
        {
            cells[from - 1]->snake_to = to;
        }
    }
    return EXIT_SUCCESS;
}

/**
 * fills database
 * @param markov_chain
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int fill_database_snakes(MarkovChain *markov_chain)
{
    Cell* cells[BOARD_SIZE];
    if(create_board(cells) == EXIT_FAILURE)
    {
        return EXIT_FAILURE;
    }
    MarkovNode *from_node = NULL, *to_node = NULL;
    size_t index_to;
    for (size_t i = 0; i < BOARD_SIZE; i++)
    {
        add_to_database(markov_chain, cells[i]);
    }

    for (size_t i = 0; i < BOARD_SIZE; i++)
    {
        from_node = get_node_from_database(markov_chain,cells[i])->data;

        if (cells[i]->snake_to != EMPTY || cells[i]->ladder_to != EMPTY)
        {
            index_to = MAX(cells[i]->snake_to,cells[i]->ladder_to) - 1;
            to_node = get_node_from_database(markov_chain, cells[index_to])->data;
            add_node_to_frequency_list(from_node, to_node, markov_chain);
        }
        else
        {
            for (int j = 1; j <= DICE_MAX; j++)
            {
                index_to = ((Cell*) (from_node->data))->number + j - 1;
                if (index_to >= BOARD_SIZE)
                {
                    break;
                }
                to_node = get_node_from_database(markov_chain, cells[index_to])->data;
                int  res = add_node_to_frequency_list(from_node, to_node, markov_chain);
                if(res==EXIT_FAILURE)
                {
                    return EXIT_FAILURE;
                }
            }
        }
    }
    // free temp arr
    for (size_t i = 0; i < BOARD_SIZE; i++)
    {
        free(cells[i]);
    }
    return EXIT_SUCCESS;
}

bool is_last_cell(void* data){
    Cell* ptr = (Cell*) data;
    return ptr->number == 100;
}

int comp_cell(void* data_1, void* data_2){
    Cell * num1 = (Cell *) data_1;
    Cell * num2 = (Cell *) data_2;
    return num1->number - num2->number;
}

void print_cell(void* data){
    Cell* ptr = (Cell*) data;
    if (ptr->snake_to != EMPTY)
    {
        printf("[%i] -snake to-> ", ptr->number);
    }
    else{
        if(ptr->ladder_to != EMPTY){
            printf("[%i] -ladder to-> ", ptr->number);
        }
        else{
            if(ptr->number!=100){
                printf("[%i] -> ", ptr->number);
            }
            else{
                printf("[%i] ", ptr->number);
            }
        }
    }


}

void* copy_cell(void *source){
    Cell *ptr = (Cell*) source;
    Cell * new_cell = malloc(sizeof(Cell));
    new_cell->number = ptr->number;
    new_cell->ladder_to = ptr->ladder_to;
    new_cell->snake_to = ptr->snake_to;
    return new_cell;
}

void free_cell(void* data){
    free(data);
}

void print_markov_chain(MarkovChain *markov_chain) {
    Node *current = markov_chain->database->first;
    while (current != NULL) {
        printf("num %i, ladder %i, snake %i",
               ((Cell*)current->data->data)->number, ((Cell*)current->data->data)->ladder_to, ((Cell*)current->data->data)->snake_to);
        current = current->next;
    }
}

/**
 * @param argc num of arguments
 * @param argv 1) Seed
 *             2) Number of sentences to generate
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int main(int argc, char *argv[])
{
    if(argc != 3){
        printf(NUM_ARGS_ERROR);
        return EXIT_FAILURE;
    }
    int number_of_path = strtol(argv[2], NULL, 10);
    int seed = strtol(argv[1], NULL, 10);
    srand(seed);
    MarkovChain* markovChain = malloc(sizeof(MarkovChain));
    if (markovChain == NULL){
        printf(ALLOCATION_ERROR_MASSAGE);
        return EXIT_FAILURE;
    }
    LinkedList* database = malloc(sizeof(LinkedList));
    if(database == NULL){
        printf(ALLOCATION_ERROR_MASSAGE);
        return EXIT_FAILURE;
    }
    markovChain->database = database;
    markovChain->copy_func = copy_cell;
    markovChain->comp_func = comp_cell;
    markovChain->free_data = free_cell;
    markovChain->print_func = print_cell;
    markovChain->is_last = is_last_cell;
    if(fill_database_snakes(markovChain)==EXIT_FAILURE){
        free_markov_chain(&markovChain);
        return EXIT_FAILURE;
    }
    for (int i = 0; i < number_of_path; i++){
        printf("Random Walk %i: ", i + 1);
        generate_random_sequence(markovChain,markovChain->database->first->data, MAX_PATH);
    }
    free_markov_chain(&markovChain);
    return EXIT_SUCCESS;
}
