# include <config.h>

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

// driver ----------------------------------------------------------------------

int main(int argc, char **argv)
{
  if ( argc < 3 )
    error(1, errno, "USAGE: ./find_transversal_depth <side> <ntransversals> < <ifile>");

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
  printf("depth = %d\n", TABLE[min()].top);
  destroy();

  exit(0);
}
