# include <stdio.h>
# include <stdlib.h>
# include <stddef.h>
# include <errno.h>
# include <error.h>

typedef struct {
  int left, right, down, up, top;
} node_t;

int n, N, Z, NOPTS, NSOLS, SETSIZE;
int *SOLUTION = NULL, **TRANSVERSALS = NULL, **L = NULL, **SYM = NULL;
node_t *TABLE = NULL;

// initialize ------------------------------------------------------------------

void initialize(void)
{
  int i, j, k, item, p;

  TABLE = (node_t*)malloc((Z + 1) * sizeof(node_t));
  SOLUTION = (int*)malloc(NSOLS * sizeof(int));

  L = (int**)malloc(n * sizeof(int*));
  SYM = (int**)malloc(n * sizeof(int*));
  for ( i = 0; i < n; i++ ) {
    L[i] = (int*)malloc(n * sizeof(int));
    SYM[i] = (int*)malloc(n * sizeof(int));
  }

  /* initialize left/right links in header */
  for ( i = 1; i <= N; i++ ) {
    /* horizontal links */
    TABLE[i - 1].right = i;
    TABLE[i].left = i - 1;

    /* vertical */
    TABLE[i].up = i;
    TABLE[i].down = i;

    /* length */
    TABLE[i].top = 0;
  }
  TABLE[0].left = N;
  TABLE[N].right = 0;

  /* initialize links in body */
  for ( k = 0; k < NOPTS; k++ ) {
    /* first item in option */
    item = TRANSVERSALS[k][0];
    TABLE[i].top = item;
    TABLE[i].left = i + SETSIZE - 1;
    p = TABLE[item].up;
    TABLE[i].up = p;
    TABLE[p].down = i;
    TABLE[i].down = item;
    TABLE[item].up = i;
    TABLE[item].top++;
    i++;

    /* rest of list item */
    for ( j = 1; j < SETSIZE; j++, i++ ) {
      item = TRANSVERSALS[k][j];
      TABLE[i].top = item;
      TABLE[i - 1].right = i;
      TABLE[i].left = i - 1;
      p = TABLE[item].up;
      TABLE[i].up = p;
      TABLE[p].down = i;
      TABLE[i].down = item;
      TABLE[item].up = i;
      TABLE[item].top++;
    }
    TABLE[i - 1].right = i - SETSIZE;
  }
}

static inline
void destroy(void)
{
  free(TABLE);
  free(SOLUTION);

  for ( int i = 0; i < n; i++ ) {
    free(L[i]);
    free(SYM[i]);
  }
  free(L); free(SYM);
}

// covering/uncovering----------------------------------------------------------

void cover(int c)
{
  int i, j, r, l, u, d;

  /* cover c in header */
  l = TABLE[c].left;
  r = TABLE[c].right;
  TABLE[r].left = l;
  TABLE[l].right = r;

  /* hide every item in any option containing c */
  for ( i = TABLE[c].down; i != c; i = TABLE[i].down )
    for ( j = TABLE[i].right; j != i; j = TABLE[j].right ) {
      u = TABLE[j].up;
      d = TABLE[j].down;
      TABLE[d].up = u;
      TABLE[u].down = d;
      TABLE[TABLE[j].top].top--;
    }
}

void uncover(int c)
{
  int i, j, r, l, u, d;

  /* unhide every item in any option containing c */
  for ( i = TABLE[c].up; i != c; i = TABLE[i].up )
    for ( j = TABLE[i].left; j != i; j = TABLE[j].left ) {
      TABLE[TABLE[j].top].top++;
      u = TABLE[j].up;
      d = TABLE[j].down;
      TABLE[d].up = j;
      TABLE[u].down = j;
    }

  /* uncover c in header */
  r = TABLE[c].right;
  l = TABLE[c].left;
  TABLE[r].left = c;
  TABLE[l].right = c;
}

// find min --------------------------------------------------------------------

int min(void)
{
  int c, j, s = Z;
  for ( j = TABLE[0].right; j != 0; j = TABLE[j].right )
    if ( TABLE[j].top < s ) {
      c = j;
      s = TABLE[j].top;
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

void copy_sol(void)
{
  int i, j, s, k;
  for ( i = 0, k = 0; i < NSOLS; i++, k = 0 ) {
    s = SOLUTION[i];
    j = s;
    do {
      SYM[i][k++] = TABLE[j].top;
      j = TABLE[j].right;
    } while (j != s);
  }

  for ( i = 0; i < n; i++ )
    qsort(SYM[i], n, sizeof(SYM[i][0]), arr_cmp);
  qsort(SYM, n, sizeof(SYM[0]), square_cmp);
}

static inline
void make_arr(void)
{
  int i, j, c, r;
  for ( i = 0; i < n; i++ )
    for ( j = 0; j < n; j++ ) {
      c = (SYM[i][j] - 1) % n;
      r = (SYM[i][j] - c - 1) / n;
      L[r][c] = i + 1;
    }
}

static inline
void print_arr(void)
{
  int i, j;
  for ( i = 0; i < n; i++ )
    for ( j = 0; j < n ; j++ )
      printf("%d ", L[i][j]);
  printf("\n");
}

static inline
void print(void)
{
  copy_sol();
  make_arr();
  print_arr();
  fflush(stdout);
}

// algorithm X -----------------------------------------------------------------

void dfs(unsigned k)
{
  if ( TABLE[0].right == 0 ) {
    print();
    return;
  }

  int j, c, r, l, d, u;

  c = min();
  cover(c);

  for ( r = TABLE[c].down; r != c; r = TABLE[r].down ) {
    SOLUTION[k] = r;
    for ( j = TABLE[r].right; j != r; j = TABLE[j].right )
      cover(TABLE[j].top);
    dfs(k + 1);
    r = SOLUTION[k];
    c = TABLE[r].top;
    for ( j = TABLE[r].left; j != r; j = TABLE[j].left )
      uncover(TABLE[j].top);
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
