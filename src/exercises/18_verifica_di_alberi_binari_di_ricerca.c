#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <limits.h>

////////////////////////////////////////////////////////////////////////////////
typedef struct Node {
        int key;
        struct Node* left;
        struct Node* right;
} Node;

Node* node_new(int key) {
        Node* n = malloc(sizeof(Node));
        assert(n != NULL);
        n->key = key;
        n->left = n->right = NULL;
        return n;
}

void free_tree(Node* root) {
        if (!root) return;
        free_tree(root->left);
        free_tree(root->right);
        free(root);
}

Node* decode_tree(char** tokens, int* pos) {
        if (strcmp(tokens[*pos], "NULL") == 0) {
                (*pos)++;
                return NULL;
        }
        int key = atoi(tokens[*pos]);
        (*pos)++;
        Node* r = node_new(key);
        r->left  = decode_tree(tokens, pos);
        r->right = decode_tree(tokens, pos);
        return r;
}

bool is_BST(Node* r, long min, long max) {
        if (!r) return true;
        if (r->key <= min || r->key >= max) return false;
        return is_BST(r->left,  min,    r->key)
                && is_BST(r->right, r->key, max);
}

////////////////////////////////////////////////////////////////////////////////
int main(void) {

        char line[10000];
        if (fgets(line, sizeof(line), stdin) == NULL) {
                return 0;
        }

        char* tokens[1000];
        int n = 0;
        char* tok = strtok(line, " \t\r\n");
        while (tok) {
                tokens[n++] = tok;
                tok = strtok(NULL, " \t\r\n");
        }

        int pos = 0;
        Node* root = decode_tree(tokens, &pos);
        bool ok = is_BST(root, LONG_MIN, LONG_MAX);

        printf("%d\n", ok ? 1 : 0);
        free_tree(root);
        return 0;
}

