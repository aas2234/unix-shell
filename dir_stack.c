#include "dir_stack.h"

/* allocate memory and create stack */
struct dir_stack *create_dir_stack(void)
{
	struct dir_stack *stck = malloc(sizeof(struct dir_stack));
	if (!stck)
		return NULL;
	else {
		stck->list = create_dir_list();
		if (!stck->list)
			return NULL;
	}
	return stck;
}

/* destroy created stack */
void destroy_dir_stack(struct dir_stack *stck)
{
	if (stck) {
		destroy_dir_list(stck->list);
		free(stck);
	}
}

/* check if stack is empty */
int dir_stackempty(struct dir_stack *stck)
{
	if (!stck->list->tail)
		return STACK_EMPTY;
	else
		return STACK_NOT_EMPTY;
}

/* check if stack is full */
int dir_stackfull(struct dir_stack *stck)
{
	return STACK_NOT_FULL;
}

/* push node onto stack */
int push_dir(struct dir_stack *stck, struct dir_node *node)
{
	if (dir_stackfull(stck) == STACK_NOT_FULL)
		return append_to_list_tail(stck->list, node);
	else
		return EXIT_FAILURE;
}

/* pop node from stack and return it */
struct dir_node *pop_dir(struct dir_stack *stck)
{
	if (dir_stackempty(stck) == STACK_NOT_EMPTY)
		return remove_from_list_tail(stck->list);
	else
		return NULL;
}

/* print stack */
void print_dir_stack(struct dir_stack *stck)
{
	print_list_tail_to_head(stck->list);
}
