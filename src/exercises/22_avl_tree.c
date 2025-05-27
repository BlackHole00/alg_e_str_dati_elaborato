#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#define DEBUG

////////////////////////////////////////////////////////////////////////////////
typedef struct AVL_Node {
	int key;
	const char* value;

	struct AVL_Node* parent;
	struct AVL_Node* left;
	struct AVL_Node* right;

	// Dato da height(right) - height(left)
	// Valori OK: -1, 0, 1. Altri valori richiedono bilanciamento
	int balance_factor;
} AVL_Node;

AVL_Node* avlnode_new(AVL_Node* parent, int key, const char* value) {
	AVL_Node* node = malloc(sizeof(AVL_Node));
	assert(node != NULL);

	node->key = key;
	node->value = strdup(value);

	node->parent = parent;
	node->left = NULL;
	node->right = NULL;

	node->balance_factor = 0;

	return node;
}

void avlnode_free(AVL_Node* node) {
	assert(node != NULL);

	free((void*)node->value);
	free(node);
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

int avlnode_height(AVL_Node* node) {
	if (node == NULL) {
		return 0;
	}

	int left_height = avlnode_height(node->left);
	int right_height = avlnode_height(node->right);

	if (left_height > right_height) {
		return left_height + 1;
	} else {
		return right_height + 1;
	}
}

////////////////////////////////////////////////////////////////////////////////
typedef struct {
	AVL_Node* root;
} AVL_Tree;

typedef enum {
	AVL_TREE_REBALANCE_RIGHT,
	AVL_TREE_REBALANCE_LEFT,
	AVL_TREE_REBALANCE_LEFT_RIGHT,
	AVL_TREE_REBALANCE_RIGHT_LEFT,
} AVL_Tree_Rebalance_Action;

typedef enum {
	AVL_TREE_BALANCE_NEW_NODE,
	AVL_TREE_BALANCE_DELETED_NODE,
} AVL_Tree_Balance_Change;

void avltree_show(AVL_Tree tree);

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

void avltree_destroy(AVL_Tree tree) {
	avltree_destroy_helper(tree.root);
}

void avltree_rotate_left(AVL_Tree* tree, AVL_Node* node) {
	assert(tree != NULL);
	assert(node != NULL);

	AVL_Node* right = node->right;
	if (right == NULL) {
		return;
	}

	node->right = right->left;
	if (right->left != NULL) {
		right->left->parent = node;
	}

	if (node->parent == NULL) {
		tree->root = right;
		right->parent = NULL;
	} else {
		right->parent = node->parent;
		if (node->parent->left == node) {
			node->parent->left = right;
		} else {
			node->parent->right = right;
		}
	}

	right->left = node;
	node->parent = right;

	node->balance_factor -= 2;
	right->balance_factor -= 1;
	
}

void avltree_rotate_right(AVL_Tree* tree, AVL_Node* node) {
	assert(tree != NULL);
	assert(node != NULL);

	AVL_Node* left = node->left;
	if (left == NULL) {
		return;
	}

	node->left = left->right;
	if (left->right != NULL) {
		left->right->parent = node;
	}

	if (node->parent == NULL) {
		tree->root = left;
		left->parent = NULL;
	} else {
		left->parent = node->parent;
		if (node->parent->left == node) {
			node->parent->left = left;
		} else {
			node->parent->right = left;
		}
	}

	left->right = node;
	node->parent = left;

	node->balance_factor += 2;
	left->balance_factor += 1;

}

void avltree_rotate_left_right(AVL_Tree* tree, AVL_Node* node) {
	assert(tree != NULL);
	assert(node != NULL);

	AVL_Node* left = node->left;
	if (left == NULL) {
		return;
	}

	avltree_rotate_left(tree, left);
	avltree_rotate_right(tree, node);

	left->balance_factor += 1;
}

void avltree_rotate_right_left(AVL_Tree* tree, AVL_Node* node) {
	assert(tree != NULL);
	assert(node != NULL);

	AVL_Node* right = node->right;
	if (right == NULL) {
		return;
	}

	avltree_rotate_right(tree, right);
	avltree_rotate_left(tree, node);

	right->balance_factor -= 1;
}

void avltree_rebalance(AVL_Tree* tree, AVL_Node* node) {
	assert(tree != NULL);
	assert(node != NULL);

	if (abs(node->balance_factor) < 1) {
		// No rebalance needed
		return;
	}

	AVL_Tree_Rebalance_Action rebalance_action;

	if (node->balance_factor < 0) {
		assert(node->balance_factor == -2);
		assert(node->left != NULL);

		if (node->left->balance_factor < 0) {
			rebalance_action = AVL_TREE_REBALANCE_RIGHT;
		} else {
			rebalance_action = AVL_TREE_REBALANCE_LEFT_RIGHT;
		}
	} else {
		assert(node->balance_factor == 2);
		assert(node->right != NULL);

		if (node->right->balance_factor > 0) {
			rebalance_action = AVL_TREE_REBALANCE_LEFT;
		} else {
			rebalance_action = AVL_TREE_REBALANCE_RIGHT_LEFT;
		}
	}

	#ifdef DEBUG
	avltree_show(*tree);
	#endif

	switch (rebalance_action) {
	case AVL_TREE_REBALANCE_LEFT:
		avltree_rotate_left(tree, node);
		break;
	case AVL_TREE_REBALANCE_RIGHT:
		avltree_rotate_right(tree, node);
		break;
	case AVL_TREE_REBALANCE_LEFT_RIGHT:
		avltree_rotate_left_right(tree, node);
		break;
	case AVL_TREE_REBALANCE_RIGHT_LEFT:
		avltree_rotate_right_left(tree, node);
		break;
	}

	#ifdef DEBUG
	avltree_show(*tree);
	#endif
}

void avltree_propagate_balance_insertion(AVL_Tree* tree, AVL_Node* node) {
	int balance_delta = 1;

	AVL_Node* current_node = node;
	while(current_node->parent != NULL) {
		AVL_Node* prev_node = current_node;
		current_node = current_node->parent;

		int old_balance_factor = current_node->balance_factor;

		if (current_node->left == prev_node) {
			#ifdef DEBUG
			printf("Incrementing bf of %s to %d (from %d, coming from left)\n", current_node->value, current_node->balance_factor + balance_delta, current_node->balance_factor);
			#endif
			current_node->balance_factor -= balance_delta;
		} else {
			#ifdef DEBUG
			printf("Incrementing bf of %s to %d (from %d, coming from right)\n", current_node->value, current_node->balance_factor - balance_delta, current_node->balance_factor);
			#endif
			current_node->balance_factor += balance_delta;
		}

		if (abs(current_node->balance_factor) > 1) {
			avltree_rebalance(tree, current_node);
			break;
		}

		if (
			// ((current_node->left == prev_node && current_node->right != NULL) ||
			// (current_node->right == prev_node && current_node->left != NULL)) &&
			((current_node->left == prev_node && old_balance_factor > 0) ||
			(current_node->right == prev_node && old_balance_factor < 0))
			// prev_node == node
		) {
			break;
		}
	}
	
}

void avltree_insert(AVL_Tree* tree, int key, const char* value) {
	assert(tree != NULL);

	AVL_Node* prev_node = NULL;
	AVL_Node** insertion_point = &tree->root;

	// Walk down the tree to find an insertion location
	while (*insertion_point != NULL) {
		prev_node = *insertion_point;

		if ((*insertion_point)->key > key) {
			insertion_point = &((*insertion_point)->left);
		} else {
			insertion_point = &((*insertion_point)->right);
		}
	}

	#ifdef DEBUG
	if (prev_node != NULL) {
		printf("Inserting %s of %s\n", *insertion_point == prev_node->left ? "left" : "right", prev_node->value);
	}
	#endif
	*insertion_point = avlnode_new(prev_node, key, value);
	if (prev_node == NULL) {
		tree->root = *insertion_point;
	}

	avltree_propagate_balance_insertion(tree, *insertion_point);
}

AVL_Node* avltree_find_node(AVL_Tree tree, int key) {
	AVL_Node* current_node = tree.root;
	
	while (current_node != NULL) {
		if (key == current_node->key) {
			return current_node;
		} else if (key > current_node->key) {
			current_node = current_node->right;
		} else {
			current_node = current_node->left;
		}
	}

	return NULL;
}

void avltree_propagate_balance_removal(AVL_Tree* tree, AVL_Node* node, bool was_left_node) {
	if (node->parent == NULL) {
		return;
	}

	int old_balance_factor = node->parent->balance_factor;

	if (was_left_node) {
		node->parent->balance_factor += 1;
	} else {
		node->parent->balance_factor -= 1;
	}

	if (abs(node->parent->balance_factor) > 1) {
		avltree_rebalance(tree, node->parent);
		return;
	}

	// if (old_balance_factor == 0) {
	// 	return;
	// }
	if (was_left_node && old_balance_factor < 0) {
		return;
	}
	if (!was_left_node && old_balance_factor > 0) {
		return;
	}

	int balance_delta = -1;

	AVL_Node* current_node = node->parent;
	while(current_node->parent != NULL) {
		AVL_Node* prev_node = current_node;
		current_node = current_node->parent;

		int old_balance_factor = current_node->balance_factor;

		if (current_node->left == prev_node) {
			#ifdef DEBUG
			printf("Decrementing bf of %s to %d (from %d, coming from left)\n", current_node->value, current_node->balance_factor - balance_delta, current_node->balance_factor);
			#endif
			current_node->balance_factor -= balance_delta;
		} else {
			#ifdef DEBUG
			printf("Decrementing bf of %s to %d (from %d, coming from right)\n", current_node->value, current_node->balance_factor + balance_delta, current_node->balance_factor);
			#endif
			current_node->balance_factor += balance_delta;
		}

		if (abs(current_node->balance_factor) > 1) {
			avltree_rebalance(tree, current_node);
			break;
		}
	}
}

void avltree_remove_leaf(AVL_Tree* tree, AVL_Node* leaf) {
	assert(tree != NULL);
	assert(leaf != NULL);

	if (leaf->parent == NULL) {
		tree->root = NULL;
		return;
	}

	bool was_left_node;
	if (leaf->parent->left == leaf) {
		was_left_node = true;
		leaf->parent->left = NULL;
	} else {
		was_left_node = false;
		leaf->parent->right = NULL;
	}

	avltree_propagate_balance_removal(tree, leaf, was_left_node);
}

void avltree_remove_helper(AVL_Tree* tree, AVL_Node* removal_node) {
	if (removal_node->left == NULL && removal_node->right == NULL) {
		avltree_remove_leaf(tree, removal_node);
	} else if (removal_node->left == NULL) {
		if (removal_node->parent == NULL) {
			tree->root = removal_node->right;
			removal_node->right->parent = NULL;
			return;
		} else {
			bool was_left_node;
			if (removal_node->parent->left == removal_node) {
				removal_node->parent->left = removal_node->right;
				removal_node->right->parent = removal_node->parent;

				was_left_node = true;
			} else {
				removal_node->parent->right = removal_node->right;
				removal_node->right->parent = removal_node->parent;

				was_left_node = false;
			}
			avltree_propagate_balance_removal(tree, removal_node, was_left_node);
		}
	} else if (removal_node->right == NULL) {
		if (removal_node->parent == NULL) {
			tree->root = removal_node->left;
			removal_node->left->parent = NULL;
			return;
		} else {
			bool was_left_node;
			if (removal_node->parent->right == removal_node) {
				removal_node->parent->right = removal_node->left;
				removal_node->left->parent = removal_node->parent;

				was_left_node = false;
			} else {
				removal_node->parent->left = removal_node->left;
				removal_node->left->parent = removal_node->parent;

				was_left_node = true;
			}
			avltree_propagate_balance_removal(tree, removal_node, was_left_node);
		}
	} else {
		AVL_Node* successor = avlnode_successor(removal_node);
		assert(successor != NULL);

		avltree_remove_helper(tree, successor);

		successor->left = removal_node->left;
		if (successor->left != NULL) {
			successor->left->parent = successor;
		}

		successor->right = removal_node->right;
		if (successor->right != NULL) {
			successor->right->parent = successor;
		}

		successor->balance_factor = removal_node->balance_factor;

		if (removal_node->parent == NULL) {
			tree->root = successor;
			successor->parent = NULL;
		} else {
			successor->parent = removal_node->parent;

			if (removal_node->parent->left == removal_node) {
				removal_node->parent->left = successor;
			} else {
				removal_node->parent->right = successor;
			}
		}
	}
}

void avltree_remove(AVL_Tree* tree, int key) {
	AVL_Node* removal_node = avltree_find_node(*tree, key);
	if (removal_node == NULL) {
		return;
	}

	avltree_remove_helper(tree, removal_node);
	avlnode_free(removal_node);
}

const char* avltree_find(AVL_Tree tree, int key) {
	AVL_Node* search_result = avltree_find_node(tree, key);
	if (search_result == NULL) {
		return NULL;
	} else {
		return search_result->value;
	}
}

void avltree_show_helper(AVL_Node* node, int current_height, bool* is_frist_print) {
	if (!*is_frist_print) {
		printf(" ");
	} else {
		*is_frist_print = false;
	}

	if (node == NULL) {
		printf("NULL");
		return;
	}

	int node_height = avlnode_height(node);

	#ifdef DEBUG
	printf("%d:%s:%d:%d", node->key, node->value, node_height, node->balance_factor);
	#else
	printf("%d:%s:%d", node->key, node->value, node_height);
	#endif
	avltree_show_helper(node->left, current_height + 1, is_frist_print);
	avltree_show_helper(node->right, current_height + 1, is_frist_print);
}

void avltree_show(AVL_Tree tree) {
	bool is_first_print = true;
	avltree_show_helper(tree.root, 1, &is_first_print);
	printf("\n");
}

////////////////////////////////////////////////////////////////////////////////
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
	
	avltree_destroy(tree);
}

