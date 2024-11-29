# include <config.h>

# include <stdio.h>
# include <stddef.h>
# include <stdlib.h>
# include <errno.h>
# include <error.h>

# include <cliquer-1.21/cliquer.h>

boolean pr(set_t s, graph_t *G, clique_options *opts)
{
  set_print(s);
  printf("\n");
  fflush(stdout);
  return TRUE;
}

unsigned check_form(int **L, int n)
{
  int i, j;
  for ( i = 0; i < n; i++ )
    for ( j = 0; j < n; j++ )
      if ( L[i][j] < 1 || L[i][j] > n )
        return 0;
  return 1;
}

unsigned orthogonal(int *restrict*restrict L1,
                    int *restrict*restrict L2,
                    int n)
{
  int i, j, val;
  int *sym = (int*)calloc(n * n, sizeof(int));

  for ( i = 0; i < n; i++ )
    for ( j = 0; j < n; j++ ) {
      val = (L1[i][j] - 1) + ((L2[i][j] - 1) * n);
      if ( (val < 0) || (val > (n * n) - 1) || (sym[val] > 0) )
        return 0;
      else sym[val]++;
    }
  return 1;
}

void read_squares(int ***L, int N, int n)
{
  L = (int***)malloc(N * sizeof(int**));
  int i, j, k;
  for ( i = 0; i < N; i++ ) {
    L[i] = (int**)malloc(n * sizeof(int*));
    for ( j = 0; j < n; j++ ) {
      L[i][j] = (int*)malloc(n * sizeof(int));
      for ( k = 0; k < n; k++ )
        if ( scanf("%d", &L[i][j][k]) == EOF )
          error(1, errno, "ERROR: failed to read %d:%d:%d", i, j, k);
    }
  }
}

void free_squares(int ***L, int N, int n)
{
  int i, j;
  if ( L != NULL ) {
    for ( i = 0; i < N; i++ ) {
      if ( L[i] != NULL )
        for ( j = 0; j < n; j++ )
          if ( L[i][j] != NULL )
            free(L[i][j]);
      free(L[i]);
    }
    free(L);
  }
}

int main(int argc, char **argv)
{
  if ( argc < 3 )
    error(1, errno, "USAGE: ./moles <side> <nsquares> < <ifile>");

  int n, N;
  if ( sscanf(argv[1], "%d", &n) == EOF )
    error(1, errno, "ERROR: failed to read <side>");
  if ( sscanf(argv[2], "%d", &N) == EOF )
    error(1, errno, "ERROR: failed to read <nsquares>");

  int ***L = NULL;
  read_squares(L, N, n);

  int i, j;
  graph_t *G = graph_new(N);
  for ( i = 0; i < N - 1; i++ )
    for ( j = i + 1; j < N; j++ )
      if ( orthogonal(L[i], L[2], n) > 0 )
        GRAPH_ADD_EDGE(G, i, j);

  ASSERT(graph_test(G, NULL));

  clique_options opts = {
    .reorder_function = reorder_by_default,
    .reorder_map = NULL,
    .time_function = NULL,
    .output = NULL,
    .user_function = NULL,
    .user_data = NULL,
    .clique_list = NULL,
    .clique_list_length = 0,
  };

  int maximum_size = clique_unweighted_max_weight(G, &opts);
  opts.user_function = pr;
  int num_cliques = clique_unweighted_find_all(G, maximum_size, maximum_size, FALSE, &opts);

  graph_free(G);

  free_squares(L, N, n);

  exit(0);
}
