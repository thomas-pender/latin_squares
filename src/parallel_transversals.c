# include <config.h>

# include <stdio.h>
# include <stdlib.h>
# include <stddef.h>
# include <stdint.h>
# include <limits.h>
# include <errno.h>
# include <error.h>
# include <pthread.h>

# define NTHREADS 3
# define ARR_MAX 100

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

// thread args -----------------------------------------------------------------

typedef struct {
  size_t index;
  unsigned long long counts[ARR_MAX];
} counts_arr_t;

typedef struct {
  int first, last;
  counts_arr_t counts;
} args_t;

static inline
void arr_init(counts_arr_t *counts)
{
  counts->index = 0;
  for ( unsigned i = 0; i < ARR_MAX; i++ )
    counts->counts[i] = 0;
}

static inline
void arr_inc(counts_arr_t *arr)
{
  if ( arr->counts[arr->index] == ULLONG_MAX ) {
    arr->index++;
    if ( arr->index >= ARR_MAX ) {
      fputs("ERROR: counts too large\n", stderr);
      exit(1);
    }
  }
  arr->counts[arr->index]++;
}

static inline
void arr_print(counts_arr_t *counts)
{
  for ( unsigned i = 0; i <= counts->index; i++ )
    printf("%llu\n", counts->counts[i]);
}

// algorithm X -----------------------------------------------------------------

typedef struct {
  int left, right, down, up, top;
} node_t;

int n, N, Z, NOPTS, NSOLS;
const int SETSIZE = 3;
int **L = NULL;

// initialize ------------------------------------------------------------------

void initialize(node_t **table, int **solution)
{
  int i, j, k, c, r, l, p;

  *table = (node_t*)malloc((Z + 1) * sizeof(node_t));
  *solution = (int*)malloc(NSOLS * sizeof(int));

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
    /* column item in option */
    c = (k % n) + 1;
    (*table)[i].top = c;
    (*table)[i].left = i + SETSIZE - 1;
    (*table)[i].right = i + 1;
    p = (*table)[c].up;
    (*table)[i].up = p;
    (*table)[p].down = i;
    (*table)[i].down = c;
    (*table)[c].up = i;
    (*table)[c].top++;
    i++;
    c--;

    /* row item in option */
    r = (k / n) + n + 1;
    (*table)[i].top = r;
    (*table)[i].left = i - 1;
    (*table)[i].right = i + 1;
    p = (*table)[r].up;
    (*table)[i].up = p;
    (*table)[p].down = i;
    (*table)[i].down = r;
    (*table)[r].up = i;
    (*table)[r].top++;
    i++;
    r -= n + 1;

    /* square entry item in option */
    l = L[r][c] + (2 * n);
    (*table)[i].top = l;
    (*table)[i].left = i - 1;
    (*table)[i].right = i - SETSIZE + 1;
    p = (*table)[l].up;
    (*table)[i].up = p;
    (*table)[p].down = i;
    (*table)[i].down = l;
    (*table)[l].up = i;
    (*table)[l].top++;
    i++;
  }
}

static inline
void destroy(node_t **table, int **solution)
{
  if ( *table != NULL ) free(*table);
  if ( *solution != NULL ) free(*solution);
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

// algorithm X -----------------------------------------------------------------

void dfs(node_t *table, int *solution, int k, args_t *args)
{
  if ( table[0].right == 0 ) {
    arr_inc(&(args->counts));
    return;
  }

  int j, c, r, l, d, u;

  c = min(table);
  cover(table, c);

  if ( k == 0 ) {
    int p;
    for ( p = 1, r = table[c].down; r != c && p < args->first; r = table[r].down, p++ );
    for ( ; r != c && p <= args->last; r = table[r].down, p++ ) {
      solution[k] = r;
      for ( j = table[r].right; j != r; j = table[j].right )
        cover(table, table[j].top);
      dfs(table, solution, k + 1, args);
      r = solution[k];
      c = table[r].top;
      for ( j = table[r].left; j != r; j = table[j].left )
        uncover(table, table[j].top);
    }
  }
  else {
    for ( r = table[c].down; r != c; r = table[r].down ) {
      solution[k] = r;
      for ( j = table[r].right; j != r; j = table[j].right )
        cover(table, table[j].top);
      dfs(table, solution, k + 1, args);
      r = solution[k];
      c = table[r].top;
      for ( j = table[r].left; j != r; j = table[j].left )
        uncover(table, table[j].top);
    }
  }

  uncover(table, c);
}

// latin square routines -------------------------------------------------------

void read_square(void)
{
  int i, j;

  L = (int**)malloc(n * sizeof(int*));
  for ( i = 0; i < n; i++ ) {
    L[i] = (int*)malloc(n * sizeof(int));
    for ( j = 0; j < n; j++ )
      if ( scanf("%d", &L[i][j]) == EOF )
        error(1, errno, "ERROR: failed to read entry (%d, %d) of latin square",
              i + 1, j + 1);
  }
}

static inline
void destroy_square(void)
{
  for ( int i = 0; i < n; i++ )
    if ( L[i] ) free(L[i]);
  if ( L ) free(L);
}

unsigned check_latin_square(void)
{
  int i, j, l, **row_counts, **col_counts;

  row_counts = (int**)malloc(n * sizeof(int*));
  col_counts = (int**)malloc(n * sizeof(int*));
  for ( i = 0; i < n; i++ ) {
    row_counts[i] = (int*)calloc(n + 1, sizeof(int));
    col_counts[i] = (int*)calloc(n + 1, sizeof(int));
  }

  for ( i = 0; i < n; i++ )
    for ( j = 0; j < n; j++ ) {
      l = L[i][j];
      row_counts[i][l]++;
      col_counts[j][l]++;
      if ( row_counts[i][l] > 1 || col_counts[j][l] > 1 )
        return 0U;
    }

  for ( i = 0; i < n; i++ ) {
    free(row_counts[i]);
    free(col_counts[i]);
  }
  free(row_counts);
  free(col_counts);

  return 1U;
}

// thread func -----------------------------------------------------------------

void *thread_func(void *_args) {
  args_t args = *(args_t*)_args;
  arr_init(&(args.counts));

  node_t *table = NULL;
  int *solution = NULL;

  initialize(&table, &solution);
  dfs(table, solution, 0, &args);
  destroy(&table, &solution);

  pthread_mutex_lock(&mtx);
  arr_print(&(args.counts));
  pthread_mutex_unlock(&mtx);

  return NULL;
}

// driver ----------------------------------------------------------------------

int main(int argc, char **argv)
{
  if ( argc < 4 )
    error(1, errno, "USAGE: ./parallel_transversals <side> <first> <last> < <ifile>");

  int first, last, block_len, i;

  if ( sscanf(argv[1], "%d", &n) == EOF )
    error(1, errno, "ERROR: could not read <side>");
  if ( sscanf(argv[2], "%d", &first) == EOF )
    error(1, errno, "ERROR: could not read <first>");
  if ( sscanf(argv[3], "%d", &last) == EOF )
    error(1, errno, "ERROR: could not read <last>");

  block_len = (last - first + 1) / NTHREADS;

  N = 3 * n;
  NOPTS = n * n;

  Z = N + (NOPTS * SETSIZE);
  NSOLS = n;

  read_square();
  if ( check_latin_square() == 0 )
    error(1, errno, "ERROR: not a latin square");

  pthread_t threads[NTHREADS];
  args_t args[NTHREADS];

  args[0].first = first;
  args[0].last = first + block_len;
  for ( i = 1; i < NTHREADS - 1; i++ ) {
    args[i].first = args[i - 1].last + 1;
    args[i].last = args[i].first + block_len;
  }
  args[NTHREADS - 1].first = args[NTHREADS - 2].last + 1;
  args[NTHREADS - 1].last = last;

  for ( i = 0; i < NTHREADS; i++ )
    pthread_create(&threads[i], NULL, thread_func, &args[i]);
  for ( i = 0; i < NTHREADS; i++ )
    pthread_join(threads[i], NULL);

  destroy_square();

  exit(0);
}
