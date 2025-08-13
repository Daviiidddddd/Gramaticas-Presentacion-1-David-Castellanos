// cfg_recognizer_fixed.c
// Compile: gcc cfg_recognizer_fixed.c -o cfg_recognizer
// Usage: ./cfg_recognizer grammar.txt tests.txt
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_NT 128
#define MAX_SYM_NAME 64
#define MAX_LINE 4096
#define MAX_INPUT 2048

typedef struct {
    int is_nt;
    char name[MAX_SYM_NAME];
} Symbol;

typedef struct {
    int nsyms;
    Symbol *syms;
} Production;

typedef struct {
    char name[MAX_SYM_NAME];
    Production *prods;
    int nprods;
} NonTerminal;

NonTerminal nts[MAX_NT];
int n_nts = 0;
char start_symbol[MAX_SYM_NAME];

int find_nt(const char *name) {
    int i;
    for (i = 0; i < n_nts; i++) {
        if (strcmp(nts[i].name, name) == 0) return i;
    }
    return -1;
}

int add_nt_if_missing(const char *name) {
    int idx = find_nt(name);
    if (idx >= 0) return idx;
    if (n_nts >= MAX_NT) { fprintf(stderr, "Too many nonterminals\n"); exit(1); }
    strcpy(nts[n_nts].name, name);
    nts[n_nts].nprods = 0;
    nts[n_nts].prods = NULL;
    return n_nts++;
}

void add_production(const char *lhs, Symbol *syms, int nsyms) {
    int idx = add_nt_if_missing(lhs);
    NonTerminal *nt = &nts[idx];
    nt->prods = realloc(nt->prods, sizeof(Production) * (nt->nprods + 1));
    Production *p = &nt->prods[nt->nprods++];
    p->nsyms = nsyms;
    p->syms = malloc(sizeof(Symbol) * nsyms);
    {
        int i;
        for (i = 0; i < nsyms; i++) p->syms[i] = syms[i];
    }
}

char *trim(char *s) {
    while (*s && isspace((unsigned char)*s)) s++;
    char *end = s + strlen(s) - 1;
    while (end >= s && isspace((unsigned char)*end)) { *end = 0; end--; }
    return s;
}

void load_grammar(const char *path) {
    FILE *f = fopen(path, "r");
    if (!f) { perror("fopen"); exit(1); }
    char line[MAX_LINE];
    while (fgets(line, sizeof(line), f)) {
        char *t = trim(line);
        if (*t == 0 || *t == '#') continue;
        strcpy(start_symbol, t);
        break;
    }
    if (start_symbol[0] == 0) { fprintf(stderr, "Empty grammar file\n"); exit(1); }
    while (fgets(line, sizeof(line), f)) {
        char *s = trim(line);
        if (*s == 0 || *s == '#') continue;
        char *arrow = strstr(s, "->");
        if (!arrow) continue;
        *arrow = 0;
        char lhs[MAX_SYM_NAME];
        strcpy(lhs, trim(s));
        char *rhs = trim(arrow + 2);
        char *alt = strtok(rhs, "|");
        while (alt) {
            char seq[MAX_LINE];
            strcpy(seq, trim(alt));
            if (strcmp(seq, "ε") == 0 || strcmp(seq, "e") == 0 || strcmp(seq, "") == 0) {
                Symbol *arr = NULL;
                add_production(lhs, arr, 0);
            } else {
                Symbol temp[MAX_LINE];
                int tcount = 0;
                int i = 0, L = strlen(seq);
                while (i < L) {
                    if (isspace((unsigned char)seq[i])) { i++; continue; }
                    if (isupper((unsigned char)seq[i])) {
                        char name[MAX_SYM_NAME]; int j = 0;
                        while (i < L && (isalnum((unsigned char)seq[i]) || seq[i] == '_')) { name[j++] = seq[i++]; }
                        name[j] = 0;
                        temp[tcount].is_nt = 1;
                        strcpy(temp[tcount].name, name);
                        tcount++;
                    } else {
                        char tname[2]; tname[0] = seq[i]; tname[1] = 0;
                        temp[tcount].is_nt = 0;
                        strcpy(temp[tcount].name, tname);
                        tcount++; i++;
                    }
                }
                Symbol *arr = malloc(sizeof(Symbol) * tcount);
                {
                    int k;
                    for (k = 0; k < tcount; k++) arr[k] = temp[k];
                }
                add_production(lhs, arr, tcount);
            }
            alt = strtok(NULL, "|");
        }
    }
    fclose(f);
}

int max_inplen = 0;
int ***memo_reach = NULL;

void alloc_memo(int n) {
    int i, j;
    max_inplen = n;
    memo_reach = malloc(sizeof(int **) * n_nts);
    for (i = 0; i < n_nts; i++) {
        memo_reach[i] = malloc(sizeof(int *) * (n + 1));
        for (j = 0; j <= n; j++) {
            memo_reach[i][j] = NULL;
        }
    }
}

int *make_zero_array(int n) {
    int *a = malloc(sizeof(int) * (n + 1));
    int i;
    for (i = 0; i <= n; i++) a[i] = 0;
    return a;
}

void union_into(int *target, int *src, int n) {
    int i;
    for (i = 0; i <= n; i++) if (src[i]) target[i] = 1;
}

int *compute_reachable(int ntid, int pos, const char *inp, int n);

int *match_sequence(Symbol *syms, int nsyms, int pos, const char *inp, int n) {
    int *curr = make_zero_array(n);
    curr[pos] = 1;
    {
        int k;
        for (k = 0; k < nsyms; k++) {
            int *next = make_zero_array(n);
            int p;
            for (p = 0; p <= n; p++) if (curr[p]) {
                if (p > n) continue;
                if (syms[k].is_nt) {
                    int idx = find_nt(syms[k].name);
                    if (idx < 0) continue;
                    int *child = compute_reachable(idx, p, inp, n);
                    if (child) union_into(next, child, n);
                } else {
                    if (p < n && inp[p] == syms[k].name[0]) next[p + 1] = 1;
                }
            }
            free(curr);
            curr = next;
            {
                int any = 0;
                int ii;
                for (ii = 0; ii <= n; ii++) if (curr[ii]) { any = 1; break; }
                if (!any) break;
            }
        }
    }
    return curr;
}

int *compute_reachable(int ntid, int pos, const char *inp, int n) {
    if (memo_reach[ntid][pos] != NULL) return memo_reach[ntid][pos];
    int *res = make_zero_array(n);
    NonTerminal *nt = &nts[ntid];
    {
        int i;
        for (i = 0; i < nt->nprods; i++) {
            Production *p = &nt->prods[i];
            if (p->nsyms == 0) {
                res[pos] = 1;
            } else {
                int *r = match_sequence(p->syms, p->nsyms, pos, inp, n);
                union_into(res, r, n);
                free(r);
            }
        }
    }
    memo_reach[ntid][pos] = res;
    return res;
}

int recognize_string(const char *inp) {
    int n = strlen(inp);
    alloc_memo(n);
    int start_id = find_nt(start_symbol);
    if (start_id < 0) { fprintf(stderr, "Start symbol not defined\n"); exit(1); }
    int *res = compute_reachable(start_id, 0, inp, n);
    int ok = res[n];
    {
        int i, j;
        for (i = 0; i < n_nts; i++) {
            for (j = 0; j <= n; j++) {
                if (memo_reach[i][j] != NULL) {
                    free(memo_reach[i][j]);
                    memo_reach[i][j] = NULL;
                }
            }
            free(memo_reach[i]);
            memo_reach[i] = NULL;
        }
        free(memo_reach);
        memo_reach = NULL;
    }
    return ok;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s grammar.txt tests.txt\n", argv[0]);
        return 1;
    }
    load_grammar(argv[1]);
    FILE *ft = fopen(argv[2], "r");
    if (!ft) { perror("fopen tests"); return 1; }
    char line[MAX_INPUT + 8];
    while (fgets(line, sizeof(line), ft)) {
        size_t L = strlen(line);
        while (L > 0 && (line[L - 1] == '\n' || line[L - 1] == '\r')) { line[--L] = 0; }
        int ok = recognize_string(line);
        if (ok) printf("acepta\n"); else printf("NO acepta\n");
    }
    fclose(ft);
    return 0;
}
