# include <config.h>

# include <stdio.h>
# include <stdlib.h>
# include <stddef.h>
# include <errno.h>
# include <error.h>
# include <pthread.h>

# define NTHREADS 3

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
  int left, right, down, up, top;
} node_t;

typedef struct {
  node_t *table;
  int *restrict solution, *restrict*restrict L, *restrict*restrict sym;
  int first, last;
} args_t;

int n, N, Z, NOPTS, NSOLS, SETSIZE, FIRST, LAST;
int **TRANSVERSALS = NULL;

// initialize ------------------------------------------------------------------

void initialize(node_t **table,
                int *restrict*restrict solution,
                int *restrict*restrict*restrict L,
                int *restrict*restrict*restrict sym)
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
  (*table)[0].left = N;
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
void destroy(args_t *args)
{
  if ( args->table != NULL ) free(args->table);
  if ( args->solution != NULL ) free(args->solution);

  for ( int i = 0; i < n; i++ ) {
    if ( args->L[i] != NULL ) free(args->L[i]);
    if ( args->sym[i] != NULL ) free(args->sym[i]);
  }
  if ( args->L != NULL ) free((int**)(args->L));
  if ( args->sym != NULL ) free((int**)(args->sym));
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

void copy_sol(args_t *args)
{
  int i, j, s, k;
  for ( i = 0, k = 0; i < NSOLS; i++, k = 0 ) {
    s = args->solution[i];
    j = s;
    do {
      args->sym[i][k++] = args->table[j].top;
      j = args->table[j].right;
    } while (j != s);
  }

  for ( i = 0; i < n; i++ )
    qsort(args->sym[i], n, sizeof(args->sym[i][0]), arr_cmp);
  qsort((int**)(args->sym), n, sizeof(args->sym[0]), square_cmp);
}

static inline
void make_arr(args_t *args)
{
  int i, j, c, r;
  for ( i = 0; i < n; i++ )
    for ( j = 0; j < n; j++ ) {
      c = (args->sym[i][j] - 1) % n;
      r = (args->sym[i][j] - c - 1) / n;
      args->L[r][c] = i + 1;
    }
}

static inline
void print_arr(args_t *args)
{
  int i, j;
  for ( i = 0; i < n; i++ )
    for ( j = 0; j < n ; j++ )
      printf("%d ", args->L[i][j]);
  printf("\n");
}

static inline
void print(args_t *args)
{
  copy_sol(args);
  make_arr(args);

  pthread_mutex_lock(&mtx);
  print_arr(args);
  fflush(stdout);
  pthread_mutex_unlock(&mtx);
}

// algorithm X -----------------------------------------------------------------

void dfs(args_t *args, int k)
{
  if ( args->table[0].right == 0 ) {
    print(args);
    return;
  }

  int j, c, r, l, d, u;

  c = min(args->table);
  cover(args->table, c);

  if ( k == 0 ) {
    int p;
    for ( p = 0, r = args->table[c].down; r != c && p < args->first; r = args->table[r].down, p++ );
    for ( ; r != c && p < args->last; r = args->table[r].down, p++ ) {
      args->solution[k] = r;
      for ( j = args->table[r].right; j != r; j = args->table[j].right )
        cover(args->table, args->table[j].top);
      dfs(args, k + 1);
      r = args->solution[k];
      c = args->table[r].top;
      for ( j = args->table[r].left; j != r; j = args->table[j].left )
        uncover(args->table, args->table[j].top);
    }
  }
  else {
    for ( r = args->table[c].down; r != c; r = args->table[r].down ) {
      args->solution[k] = r;
      for ( j = args->table[r].right; j != r; j = args->table[j].right )
        cover(args->table, args->table[j].top);
      dfs(args, k + 1);
      r = args->solution[k];
      c = args->table[r].top;
      for ( j = args->table[r].left; j != r; j = args->table[j].left )
        uncover(args->table, args->table[j].top);
    }
  }

  uncover(args->table, c);
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

// find parameters -------------------------------------------------------------

static inline
int min_nopts(node_t *table)
{
  return table[min(table)].top;
}

// thread func -----------------------------------------------------------------

void *thread_func(void *_args)
{
  args_t *args = (args_t*)_args;
  dfs(args, 0);
  return NULL;
}

// driver ----------------------------------------------------------------------

int main(int argc, char **argv)
{
  if ( argc < 5 )
    error(1, errno, "USAGE: ./mates <side> <ntransversals> <first> <last> < <ifile>");

  if ( sscanf(argv[1], "%d", &n) == EOF )
    error(1, errno, "ERROR: could not read <side>");
  if ( sscanf(argv[2], "%d", &NOPTS) == EOF )
    error(1, errno, "ERROR: could not read <ntransversals>");
  if ( sscanf(argv[3], "%d", &FIRST) == EOF )
    error(1, errno, "ERROR: could not read <first>");
  if ( sscanf(argv[4], "%d", &LAST) == EOF )
    error(1, errno, "ERROR: could not read <last>");

  N = n * n;
  SETSIZE = n;
  Z = N + (NOPTS * SETSIZE);
  NSOLS = n;

  unsigned i;
  args_t args[NTHREADS];
  pthread_t threads[NTHREADS];

  read_transversals();
  for ( i = 0; i < NTHREADS; i++ )
    initialize(&(args[i].table), &(args[i].solution),
               &(args[i].L), &(args[i].sym));
  free_transversals();

  int nopts = LAST - FIRST;
  int block_len = nopts / NTHREADS;

  args[0].first = FIRST;
  args[0].last = FIRST + block_len;
  for ( i = 1; i < NTHREADS - 1; i++ ) {
    args[i].first = args[i - 1].last;
    args[i].last = args[i - 1].last + block_len;
  }
  args[NTHREADS - 1].first = args[NTHREADS - 2].last;
  args[NTHREADS - 1].last = LAST;

  int throw;
  for ( i = 0; i < NTHREADS; i++ )
    if ( (throw = pthread_create(&threads[i], NULL, thread_func, &args[i])) != 0 )
      error(1, throw, "ERROR: pthread_create failed:%d", i);
  for ( i = 0; i < NTHREADS; i++ )
    if ( (throw = pthread_join(threads[i], NULL)) != 0 )
      error(1, throw, "ERROR: pthread_join failed:%d", i);

  for ( i = 0; i < NTHREADS; i++ ) destroy(&args[i]);

  exit(0);
}
