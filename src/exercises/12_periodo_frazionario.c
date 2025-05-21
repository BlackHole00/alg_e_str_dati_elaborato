#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

////////////////////////////////////////////////////////////////////////////////
int periodo_lineare(const char* s) {
        int n = strlen(s);
        int* pi = malloc((n+1) * sizeof(int));
        assert(pi != NULL);
        pi[0] = 0;

        for (int i = 1; i < n; i++) {
                int j = pi[i - 1];
                
                while (j > 0 && s[j] != s[i]) {
                        j = pi[j - 1];
                }
                if (s[j] == s[i]) {
                        j++;
                }
                pi[i] = j;
        }

        int r = pi[n - 1];
        free(pi);
        return n - r;
}

////////////////////////////////////////////////////////////////////////////////
int main(void) {
        char line[100000];
        if (!fgets(line, sizeof(line), stdin)) {
                return 0;
        }
        line[strcspn(line, "\r\n")] = '\0';

        int p = periodo_lineare(line);
        printf("%d\n", p);

        return 0;
}

