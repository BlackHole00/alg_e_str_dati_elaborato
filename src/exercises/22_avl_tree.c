#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>


////////////////////////////////////////////////////////////////////////////////
int max_int(int a, int b) {
	if (a > b) {
		return a;
	} else {
		return b;
	}
}


////////////////////////////////////////////////////////////////////////////////
typedef struct AVL_Node {
	int key;
	const char* value;

	struct AVL_Node* left;
	struct AVL_Node* right;

	int height;
} AVL_Node;

AVL_Node* avlnode_new(int key, const char* value) {
	AVL_Node* node = malloc(sizeof(AVL_Node));
	assert(node != NULL);

	node->key = key;
	node->value = strdup(value);

	node->left = NULL;
	node->right = NULL;

	node->height = 1;

	return node;
}

void avlnode_free(AVL_Node* node) {
	assert(node != NULL);

	free((void*)node->value);
	free(node);
}

int avlnode_height(AVL_Node* node) {
	if (node == NULL) {
		return 0;
	}

	return node->height;
}

int avlnode_balance_factor(AVL_Node* node) {
	if (node == NULL) {
		return 0;
	}

	return avlnode_height(node->left) - avlnode_height(node->right);
}

AVL_Node* avlnode_successor(AVL_Node* node) {
	assert(node != NULL);

	AVL_Node* current_node = node->right;
	if (current_node == NULL) {
		return current_node;
	}

	while (current_node->left != NULL) {
		current_node = current_node->left;
	}

	return current_node;
}

AVL_Node* avlnode_rotate_left(AVL_Node* node) {
	assert(node != NULL);
	assert(node->right != NULL);

	AVL_Node* right = node->right;
	AVL_Node* right_left = right->left;

	right->left = node;
	node->right = right_left;

	node->height = max_int(avlnode_height(node->left), avlnode_height(node->right)) + 1;
	right->height = max_int(avlnode_height(right->left), avlnode_height(right->right)) + 1;

	return right;
}

AVL_Node* avlnode_rotate_right(AVL_Node* node) {
	assert(node != NULL);
	assert(node->left != NULL);

	AVL_Node* left = node->left;
	AVL_Node* left_right = left->right;

	left->right = node;
	node->left = left_right;

	node->height = max_int(avlnode_height(node->left), avlnode_height(node->right)) + 1;
	left->height = max_int(avlnode_height(left->left), avlnode_height(left->right)) + 1;
	
	return left;
}

AVL_Node* avlnode_rotate_leftright(AVL_Node* node) {
	assert(node != NULL);

	node->left = avlnode_rotate_left(node->left);
	return avlnode_rotate_right(node);
}

AVL_Node* avlnode_rotate_rightleft(AVL_Node* node) {
	assert(node != NULL);

	node->right = avlnode_rotate_right(node->right);
	return avlnode_rotate_left(node);
}

AVL_Node* avlnode_rebalance(AVL_Node* node) {
	int balance_factor = avlnode_balance_factor(node);
	if (balance_factor > 1) {
		if (avlnode_balance_factor(node->left) >= 0) {
			return avlnode_rotate_right(node);
		} else {
			return avlnode_rotate_leftright(node);
		}
	} else if (balance_factor < -1) {
		if (avlnode_balance_factor(node->right) <= 0) {
			return avlnode_rotate_left(node);
		} else {
			return avlnode_rotate_rightleft(node);
		}
	}

	return node;
}

AVL_Node* avlnode_insert_in_subtree(AVL_Node* node, int key, const char* value) {
	if (node == NULL) {
		return avlnode_new(key, value);
	}

	if (key < node->key) {
		node->left = avlnode_insert_in_subtree(node->left, key, value);
	} else {
		node->right = avlnode_insert_in_subtree(node->right, key, value);
	}

	node->height = max_int(avlnode_height(node->left), avlnode_height(node->right)) + 1;
	return avlnode_rebalance(node);
}

AVL_Node* avlnode_find_in_subtree(AVL_Node* node, int key) {
	while (node != NULL) {
		if (key == node->key) {
			return node;
		} else if (key > node->key) {
			node = node->right;
		} else {
			node = node->left;
		}
	}

	return NULL;
}

AVL_Node* avlnode_remove_in_subtree(AVL_Node* node, int key) {
	if (node == NULL) {
		return NULL;
	}
	if (key < node->key) {
		node->left = avlnode_remove_in_subtree(node->left, key);
	} else if (key > node->key) {
		node->right = avlnode_remove_in_subtree(node->right, key);
	} else {
		if (node->left == NULL && node->right == NULL) {
			avlnode_free(node);
			node = NULL;
		} else if (node->left == NULL || node->right == NULL) {
			AVL_Node* children;
			if (node->left != NULL) {
				children = node->left;
			} else {
				children = node->right;
			}

			AVL_Node temp = *node;
			*node = *children;
			*children = temp;

			avlnode_free(children);
		} else {
			AVL_Node* successor = avlnode_successor(node);

			const char* temp = node->value;
			node->value = successor->value;
			successor->value = temp;

			node->key = successor->key;
			node->right = avlnode_remove_in_subtree(node->right, successor->key);
		}
	}

	if (node == NULL) {
		return NULL;
	}

	node->height = max_int(avlnode_height(node->left), avlnode_height(node->right)) + 1;

	return avlnode_rebalance(node);
}


////////////////////////////////////////////////////////////////////////////////
typedef struct {
	AVL_Node* root;
} AVL_Tree;

void avltree_create(AVL_Tree* tree) {
	tree->root = NULL;
}

void avltree_destroy_helper(AVL_Node* node) {
	if (node == NULL) {
		return;
	}

	avltree_destroy_helper(node->left);
	avltree_destroy_helper(node->right);

	avlnode_free(node);
}

void avltree_destroy(AVL_Tree* tree) {
	avltree_destroy_helper(tree->root);
	tree->root = NULL;
}

void avltree_insert(AVL_Tree* tree, int key, const char* value) {
	tree->root = avlnode_insert_in_subtree(tree->root, key, value);
}

void avltree_remove(AVL_Tree* tree, int key) {
	tree->root = avlnode_remove_in_subtree(tree->root, key);
}

const char* avltree_find(AVL_Tree tree, int key) {
	AVL_Node* search_result = avlnode_find_in_subtree(tree.root, key);
	if (search_result == NULL) {
		return NULL;
	} else {
		return search_result->value;
	}
}

void avltree_show_helper(AVL_Node* node, int current_height, bool* is_first_print) {
	if (!*is_first_print) {
		printf(" ");
	} else {
		*is_first_print = false;
	}

	if (node == NULL) {
		printf("NULL");
		return;
	}

	printf("%d:%s:%d", node->key, node->value, node->height);
	avltree_show_helper(node->left, current_height + 1, is_first_print);
	avltree_show_helper(node->right, current_height + 1, is_first_print);
}

void avltree_show(AVL_Tree tree) {
	bool is_first_print = true;
	avltree_show_helper(tree.root, 1, &is_first_print);
	printf("\n");
}

///////////////////////////////////////////////////////////////////////////////////
int main(void) {
	AVL_Tree tree;
	avltree_create(&tree);

	char command[255];

	for (;;) {
		scanf("%s", command); // UHHH-Scaryyy

		if (strcmp(command, "insert") == 0 || strcmp(command, "i") == 0) {
			int key;
			char value[1024];
			scanf("%d %s", &key, value);

			avltree_insert(&tree, key, value);
		} else if (strcmp(command, "ii") == 0) {
			int key;
			scanf("%d", &key);

			char value[64];
			sprintf(value, "%d", key);

			avltree_insert(&tree, key, value);
		} else if (strcmp(command, "remove") == 0 || strcmp(command, "r") == 0) {
			int key;
			scanf("%d", &key);

			avltree_remove(&tree, key);
		} else if (strcmp(command, "find") == 0 || strcmp(command, "f") == 0) {
			int key;
			scanf("%d", &key);

			const char* value = avltree_find(tree, key);
			printf("%s", value);
		} else if (strcmp(command, "show") == 0 || strcmp(command, "s") == 0) {
			avltree_show(tree);
		} else if (strcmp(command, "exit") == 0 || strcmp(command, "q") == 0) {
			break;
		}
	}
	
	avltree_destroy(&tree);
}

