#include "markov_chain.h"

/**
 * Get random number between 0 and max_number [0, max_number).
 * @param max_number
 * @return Random number
 */
int get_random_number(int max_number)
{
    return rand() % max_number;
}
Node* get_node_from_database(MarkovChain *markov_chain, void *data_ptr){
    Node * current = markov_chain->database->first;
    while(current != NULL){
        if(markov_chain->comp_func(current->data->data, data_ptr) == 0){
            return current;
        }
        current = current->next;
    }
    return NULL;
}

Node* add_to_database(MarkovChain *markov_chain, void *data_ptr){
    Node *node = get_node_from_database(markov_chain, data_ptr);
    if(node != NULL){
        return node;
    }
    node = malloc(sizeof(Node));
    MarkovNode *markov_node = malloc(sizeof(MarkovNode));
    if (markov_node == NULL){
        printf(ALLOCATION_ERROR_MASSAGE);
        return NULL;
    }
    markov_node->data = markov_chain->copy_func(data_ptr);
    markov_node->frequency_list = NULL;
    markov_node->frequency_list_size = 0;
    node->data = markov_node;
    node->next = NULL;
    if (markov_chain->database->first == NULL){
        markov_chain->database->first = node;
        markov_chain->database->last = node;
    }
    else{
        markov_chain->database->last->next = node;
        markov_chain->database->last = node;
    }
    markov_chain->database->size++;
    return node;
}

int add_node_to_frequency_list(MarkovNode *first_node, MarkovNode
*second_node, MarkovChain *markov_chain){
    for(int i = 0; i<first_node->frequency_list_size; i++){
        if(markov_chain->comp_func(first_node->frequency_list[i].markov_node->data,second_node->data) == 0){
            first_node->frequency_list[i].frequency++;
            return 0;
        }
    }
    first_node->frequency_list_size++;
    MarkovNodeFrequency *tmp = realloc(first_node->frequency_list,first_node->frequency_list_size * sizeof(MarkovNodeFrequency));
    if(tmp == NULL){
        first_node->frequency_list_size--;
        printf(ALLOCATION_ERROR_MASSAGE);
        return 1;
    }
    first_node->frequency_list = tmp;
    first_node->frequency_list[first_node->frequency_list_size - 1] = (MarkovNodeFrequency) {second_node,1};
    return 0;
}

void free_markov_chain(MarkovChain **chain_ptr){
    MarkovChain *markovchain = *chain_ptr;
    Node *current = (*chain_ptr)->database->first;
    while(current != NULL){
        Node *tmp = current;
        current = current->next;
        markovchain->free_data(tmp->data->data);
        free(tmp->data->frequency_list);
        free(tmp->data);
        free(tmp);
    }
    free(markovchain->database);
    free(markovchain);
}

MarkovNode* get_first_random_node(MarkovChain *markov_chain){
    Node *current = markov_chain->database->first;
    while(1){
        int random = get_random_number(markov_chain->database->size);
        current = markov_chain->database->first;
        for(int i = 0; i<random; i++){
            current = current->next;
        }
        if(!markov_chain->is_last(current->data->data)){
            return current->data;
        }
    }
}

MarkovNode* get_next_random_node(MarkovNode *cur_markov_node){
    int n = 0;
    for (int i = 0; i< cur_markov_node->frequency_list_size; i++){
        n += cur_markov_node->frequency_list[i].frequency;
    }
    int random = get_random_number(n);
    for(int j = 0; j< cur_markov_node->frequency_list_size; j++){
        random -= cur_markov_node->frequency_list[j].frequency;
        if(random<0){
            return cur_markov_node->frequency_list[j].markov_node;
        }
    }
    return NULL;
}

void generate_random_sequence(MarkovChain *markov_chain, MarkovNode *
first_node, int max_length){
    MarkovNode *current = first_node;
    markov_chain->print_func(current->data);
    int length = 1;
    while(length<max_length){
        if(markov_chain->is_last(current->data)){
            break;
        }
        MarkovNode *next = get_next_random_node(current);
        if(next == NULL){
            break;
        }
        markov_chain->print_func(next->data);
        current = next;
        length++;
    }
    printf("\n");
}