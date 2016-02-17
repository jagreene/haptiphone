#ifndef _NODE_H_
#define _NODE_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct node{
	uint16_t val;
	bool isTerminal;
	struct node* next;
} node;

node* new_node(uint8_t val,  node* next);

node* init_list(int length);

node* add_node(uint8_t val, node* head);

node* add_deriv(node* phead, node* dhead);

node* add_int(node* phead, node* ihead);

#endif
