/* Declares routines for list manipulation */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef __DIR_LIST_H_
#define __DIR_LIST_H_

struct dir_node {
	char *dir;
	struct dir_node *next, *prev;
};

struct dir_list {
	struct dir_node *head, *tail;
};

struct dir_node *create_node(char *path);
void destroy_node(struct dir_node *node);

/* creation and destruction routines */
struct dir_list *create_dir_list(void);
void destroy_dir_list(struct dir_list *list);

/* append and remove from list tail */
int append_to_list_tail(struct dir_list *list, struct dir_node *node);
struct dir_node *remove_node_from_list(struct dir_list *list,
					struct dir_node *node);
struct dir_node *remove_from_list_tail(struct dir_list *list);
int remove_dir_from_list(struct dir_list *list, char *dir);

/* print contents of list from tail to head */
void print_list_tail_to_head(struct dir_list *list);

#endif	/* __DIR_LIST_H_ */
