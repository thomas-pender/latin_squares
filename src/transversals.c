# include <stdio.h>
# include <stdlib.h>
# include <stddef.h>
# include <errno.h>
# include <error.h>

# include <assert.h>

typedef struct {
  int left, right, down, up, top;
} node_t;

/* unsigned long long count = 0; */
int n, N, Z, NOPTS, NSOLS;
const int SETSIZE = 3;
int *SOLUTION = NULL, **L = NULL;
node_t *TABLE = NULL;

// initialize ------------------------------------------------------------------

void initialize(void)
{
  int i, j, k, c, r, l, p;

  TABLE = (node_t*)malloc((Z + 1) * sizeof(node_t));
  SOLUTION = (int*)malloc(NSOLS * sizeof(int));

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
    /* column item in option */
    c = (k % n) + 1;
    TABLE[i].top = c;
    TABLE[i].left = i + SETSIZE - 1;
    TABLE[i].right = i + 1;
    p = TABLE[c].up;
    TABLE[i].up = p;
    TABLE[p].down = i;
    TABLE[i].down = c;
    TABLE[c].up = i;
    TABLE[c].top++;
    i++;
    c--;

    /* row item in option */
    r = (k / n) + n + 1;
    TABLE[i].top = r;
    TABLE[i].left = i - 1;
    TABLE[i].right = i + 1;
    p = TABLE[r].up;
    TABLE[i].up = p;
    TABLE[p].down = i;
    TABLE[i].down = r;
    TABLE[r].up = i;
    TABLE[r].top++;
    i++;
    r -= n + 1;

    /* square entry item in option */
    l = L[r][c] + (2 * n);
    TABLE[i].top = l;
    TABLE[i].left = i - 1;
    TABLE[i].right = i - SETSIZE + 1;
    p = TABLE[l].up;
    TABLE[i].up = p;
    TABLE[p].down = i;
    TABLE[i].down = l;
    TABLE[l].up = i;
    TABLE[l].top++;
    i++;
  }
}

static inline
void destroy(void)
{
  free(TABLE);
  free(SOLUTION);
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

static inline
int set(int s)
{
  if ( TABLE[s].top <= n ) return TABLE[s].right;
  if ( TABLE[s].top > (2 * n) ) return TABLE[s].left;
  return s;
}

static inline
void print(void)
{
  int r, c, l;
  for ( int i = 0; i < NSOLS; i++ ) {
    r = set(SOLUTION[i]);
    c = TABLE[r].left;
    l = TABLE[c].left;
    printf("%d %d %d ", TABLE[r].top - n, TABLE[c].top, TABLE[l].top - (2 * n));
  }
  printf("\n");
  fflush(stdout);
}

// algorithm X -----------------------------------------------------------------

void dfs(int k)
{
  if ( TABLE[0].right == 0 ) {
    /* count++; */
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

// driver ----------------------------------------------------------------------

int main(int argc, char **argv)
{
  if ( argc < 2 )
    error(1, errno, "USAGE: ./transversals <side> < <ifile>");

  if ( sscanf(argv[1], "%d", &n) == EOF )
    error(1, errno, "ERROR: could not read <side>");

  N = 3 * n;
  NOPTS = n * n;

  Z = N + (NOPTS * SETSIZE);
  NSOLS = n;

  read_square();
  if ( check_latin_square() == 0 )
    error(1, errno, "ERROR: not a latin square");

  initialize();
  destroy_square();
  dfs(0);
  destroy();

  /* printf("%llu\n", count); */

  exit(0);
}
