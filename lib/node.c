#include "node.h"
#include <stdlib.h>

node* new_node(uint8_t val, node* next){
	node* new = (node*)malloc(sizeof(node));
	new->val = val;
	new->next = next;
	new->isTerminal = false;
	return new;
};

node* init_list(int length){
	//make terminal node
	node* tail = (node*)malloc(sizeof(node));
	tail->val = 0;
	tail->isTerminal = true;
	node* head = tail;
	//make the rest
	int i = 0;
	for(i=0; i<length-1; i++){
		head = new_node(0, head);
	}
	return head;
};

node* add_node(uint8_t val, node* head){
	head = new_node(val, head);
	node* curr = head;
        int count = 0
	while (!curr->next->isTerminal){
		curr = curr->next;
	}
	curr->isTerminal = true;
	free(curr->next);
	return head;
};

node* get_avg_diff(node* head){
    int16_t total = 0;
    int count = 0;
    node* curr = head;

    while (!curr->isTerminal){
        count++;
        total += curr->next->val - curr->val;
        curr = curr->next;
    }
    return total/count;
}
