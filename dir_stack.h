#include <stdlib.h>
#include "dir_list.h"

#ifndef __STACK_H_
#define __STACK_H_

#define STACK_FULL 1
#define STACK_NOT_FULL 2
#define STACK_EMPTY 0
#define STACK_NOT_EMPTY 4

/* stack using a list */
struct dir_stack {
	struct dir_list *list;
};

/* creation and destruction routines */
struct dir_stack *create_dir_stack(void);
void destroy_dir_stack(struct dir_stack *stck);

/* push and pop routines for stack */
int push_dir(struct dir_stack *stck, struct dir_node *node);
struct dir_node *pop_dir(struct dir_stack *stck);

void print_dir_stack(struct dir_stack *stck);

#endif	/* __STACK_H_ */
