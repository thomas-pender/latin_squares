# include <config.h>

# include <stdio.h>
# include <stdlib.h>
# include <stddef.h>
# include <errno.h>
# include <error.h>
# include <pthread.h>

# define NTHREADS 3

typedef struct {
  int left, right, down, up, top;
} node_t;

int n, N, Z, NOPTS, NSOLS, SETSIZE;
int **TRANSVERSALS = NULL;

// initialize ------------------------------------------------------------------

void initialize(node_t **table,
                int *restrict*restrict solution,
                int *restrict*restrict* L,
                int *restrict*restrict* sym)
{
  int i, j, k, item, p;

  *table = (node_t*)malloc((Z + 1) * sizeof(node_t));
  *solution = (int*)malloc(NSOLS * sizeof(int));

  *L = (int**)malloc(n * sizeof(int*));
  *sym = (int**)malloc(n * sizeof(int*));
  for ( i = 0; i < n; i++ ) {
    (*L)[i] = (int*)malloc(n * sizeof(int));
    (*sym)[i] = (int*)malloc(n * sizeof(int));
  }

  /* initialize left/right links in header */
  for ( i = 1; i <= N; i++ ) {
    /* horizontal links */
    (*table)[i - 1].right = i;
    (*table)[i].left = i - 1;

    /* vertical */
    (*table)[i].up = i;
    (*table)[i].down = i;

    /* length */
    (*table)[i].top = 0;
  }
  (*t)able[0].left = N;
  (*table)[N].right = 0;

  /* initialize links in body */
  for ( k = 0; k < NOPTS; k++ ) {
    /* first item in option */
    item = TRANSVERSALS[k][0];
    (*table)[i].top = item;
    (*table)[i].left = i + SETSIZE - 1;
    p = (*table)[item].up;
    (*table)[i].up = p;
    (*table)[p].down = i;
    (*table)[i].down = item;
    (*table)[item].up = i;
    (*table)[item].top++;
    i++;

    /* rest of list item */
    for ( j = 1; j < SETSIZE; j++, i++ ) {
      item = TRANSVERSALS[k][j];
      (*table)[i].top = item;
      (*table)[i - 1].right = i;
      (*table)[i].left = i - 1;
      p = (*table)[item].up;
      (*table)[i].up = p;
      (*table)[p].down = i;
      (*table)[i].down = item;
      (*table)[item].up = i;
      (*table)[item].top++;
    }
    (*table)[i - 1].right = i - SETSIZE;
  }
}

static inline
void destroy(node_t **table, int **solution, int ***L, int ***sym)
{
  if ( *table != NULL ) free(*table);
  if ( *solution != NULL ) free(*solution);

  for ( int i = 0; i < n; i++ ) {
    free((*L)[i]);
    free((*sym)[i]);
  }
  free(*L); free(*sym);
}

// covering/uncovering----------------------------------------------------------

void cover(node_t *table, int c)
{
  int i, j, r, l, u, d;

  /* cover c in header */
  l = table[c].left;
  r = table[c].right;
  table[r].left = l;
  table[l].right = r;

  /* hide every item in any option containing c */
  for ( i = table[c].down; i != c; i = table[i].down )
    for ( j = table[i].right; j != i; j = table[j].right ) {
      u = table[j].up;
      d = table[j].down;
      table[d].up = u;
      table[u].down = d;
      table[table[j].top].top--;
    }
}

void uncover(node_t *table, int c)
{
  int i, j, r, l, u, d;

  /* unhide every item in any option containing c */
  for ( i = table[c].up; i != c; i = table[i].up )
    for ( j = table[i].left; j != i; j = table[j].left ) {
      table[table[j].top].top++;
      u = table[j].up;
      d = table[j].down;
      table[d].up = j;
      table[u].down = j;
    }

  /* uncover c in header */
  r = table[c].right;
  l = table[c].left;
  table[r].left = c;
  table[l].right = c;
}

// find min --------------------------------------------------------------------

int min(node_t *table)
{
  int c, j, s = Z;
  for ( j = table[0].right; j != 0; j = table[j].right )
    if ( table[j].top < s ) {
      c = j;
      s = table[j].top;
    }
  return c;
}

// print solution --------------------------------------------------------------

int arr_cmp(const void *a, const void *b)
{
  return *(int*)a - *(int*)b;
}

int square_cmp(const void *_a, const void *_b)
{
  int *a = *(int**)_a, *b = *(int**)_b;
  for ( unsigned i = 0; i < n; i++ ) {
    if ( a[i] < b[i] ) return -1;
    if ( a[i] > b[i] ) return 1;
  }
  return 0;
}

void copy_sol(node_t *table, int *restrict solution, int *restrict* sym)
{
  int i, j, s, k;
  for ( i = 0, k = 0; i < NSOLS; i++, k = 0 ) {
    s = solution[i];
    j = s;
    do {
      sym[i][k++] = table[j].top;
      j = table[j].right;
    } while (j != s);
  }

  for ( i = 0; i < n; i++ )
    qsort(sym[i], n, sizeof(sym[i][0]), arr_cmp);
  qsort(sym, n, sizeof(sym[0]), square_cmp);
}

static inline
void make_arr(int *restrict*restrict sym, int *restrict*restrict L)
{
  int i, j, c, r;
  for ( i = 0; i < n; i++ )
    for ( j = 0; j < n; j++ ) {
      c = (sym[i][j] - 1) % n;
      r = (sym[i][j] - c - 1) / n;
      L[r][c] = i + 1;
    }
}

static inline
void print_arr(int **L)
{
  int i, j;
  for ( i = 0; i < n; i++ )
    for ( j = 0; j < n ; j++ )
      printf("%d ", L[i][j]);
  printf("\n");
}

static inline
void print(node_t *table, int *restrict*restrict sym, int *restrict*restrict L)
{
  copy_sol(table, solution, sym);
  make_arr(sym, L);
  print_arr(L);
  fflush(stdout);
}

// algorithm X -----------------------------------------------------------------

void dfs(node_t *table,
         int *restrict solution,
         int *restrict*restrict sym,
         int *restrict*restrict L,
         unsigned k)
{
  if ( table[0].right == 0 ) {
    print();
    return;
  }

  int j, c, r, l, d, u;

  c = min();
  cover(c);

  for ( r = table[c].down; r != c; r = table[r].down ) {
    solution[k] = r;
    for ( j = table[r].right; j != r; j = table[j].right )
      cover(table[j].top);
    dfs(table, solution, sym, L, k + 1);
    r = solution[k];
    c = table[r].top;
    for ( j = table[r].left; j != r; j = table[j].left )
      uncover(table[j].top);
  }

  uncover(c);
}

// transversals ----------------------------------------------------------------

static inline
void read_transversals(void)
{
  TRANSVERSALS = (int**)malloc(NOPTS * sizeof(int*));

  int i, j, r, c, throw;
  for ( i = 0, j = 0; i < NOPTS; i++, j = 0 ) {
    TRANSVERSALS[i] = (int*)malloc(SETSIZE * sizeof(int));

    for ( j = 0; j < SETSIZE; j++ ) {
      if ( scanf("%d", &r) == EOF )
        error(1, errno, "ERROR: failed to read from transversal %d\n", i + 1);
      if ( scanf("%d", &c) == EOF )
        error(1, errno, "ERROR: failed to read from transversal %d\n", i + 1);
      if ( scanf("%d", &throw) == EOF )
        error(1, errno, "ERROR: failed to read from transversal %d\n", i + 1);

      TRANSVERSALS[i][j] = (c - 1) + ((r - 1) * n) + 1;
    }
  }
}

static inline
void free_transversals(void)
{
  if ( TRANSVERSALS != NULL ) {
    for ( size_t i = 0; i < NOPTS; i++ )
      if ( TRANSVERSALS[i] != NULL ) free(TRANSVERSALS[i]);
    free(TRANSVERSALS);
  }
}

// driver ----------------------------------------------------------------------

int main(int argc, char **argv)
{
  if ( argc < 3 )
    error(1, errno, "USAGE: ./mates <side> <ntransversals> < <ifile>");

  if ( sscanf(argv[1], "%d", &n) == EOF )
    error(1, errno, "ERROR: could not read <side>");
  if ( sscanf(argv[2], "%d", &NOPTS) == EOF )
    error(1, errno, "ERROR: could not read <ntransversals>");

  N = n * n;
  SETSIZE = n;
  Z = N + (NOPTS * SETSIZE);
  NSOLS = n;

  read_transversals();
  initialize();
  free_transversals();
  dfs(0);
  destroy();

  exit(0);
}
