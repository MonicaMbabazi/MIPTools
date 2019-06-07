#include "pathway_design.h"

int main (int argc, char** argv) {
  design_spec_t spec;
  result_t result;
  design_state_t states;
  result_t leafres;
  seqstate_t seqs;
  
  struct timeval curtime;
  char * input_fn = NULL;
  char * output_fn = NULL;
  char * output_fn_leaves = NULL;
  int input_len;
  // FILE * outf;
  // FILE * leafoutf;
  char * input_prefix = NULL;
  int rank = 0;

  if (argc != 2) {
    fprintf(stderr, "Expected 1 argument. Received %i\n", argc - 1);
    fprintf(stderr, "Usage: %s <design_prefix>\n", argv[0]);
    goto error;
  }
  
  if (rank == 0) {
    input_len = strlen(argv[1]);

    input_prefix = (char *) malloc((input_len + 2) * sizeof(char));
    input_fn = (char *) malloc((input_len + 6) * sizeof(char));
    output_fn = (char *) malloc((input_len + 10) * sizeof(char));

    check_mem(input_prefix);
    check_mem(input_fn);
    check_mem(output_fn);
    strncpy(input_prefix, argv[1], input_len + 2);
    strncpy(input_fn, argv[1], input_len + 2);
    strncpy(output_fn, argv[1], input_len + 2);

    input_prefix[input_len] = '\0';

    strncat(input_fn, ".np", 6);
    strncat(output_fn, "_0.npo", 10);
    spec.opts.seed = 0;

    check(ERR_OK == parse_design(input_fn, &spec), 
        "Error parsing file");

    gettimeofday(&curtime, NULL);
    if (spec.opts.seed == 0) {
      init_genrand(curtime.tv_usec);
      spec.opts.seed = curtime.tv_usec;
    } else {
      init_genrand(spec.opts.seed);
    }

    spec.opts.file_prefix = input_prefix;

    fprintf(stdout, "Seed: %u\n", spec.opts.seed);

    fflush(stdout);

    spec.opts.start_time = curtime.tv_sec + (1e-6 * curtime.tv_usec);
    check(ERR_OK == init_result(&result, &spec),
        "Error initializing result structure");

    init_seqstate(&seqs, &spec);

    check(ERR_OK == init_seqs_random(&seqs, &spec.seqs, &spec.opts),
        "Error initializing sequences");

    // Actually do the design and record the time it takes
    // check(ERR_OK == optimize_tubes(&result, &seqs, &spec), "Error optimizing sequences");

    init_decomposition(&spec);
    init_states(&states, &spec);
    init_result(&leafres, &spec);

    get_decomposition(&states, 20, &spec);

    update_result(&leafres, &states, &seqs, &spec);
    print_design_result(stdout, &leafres, &seqs, 0, &spec, "decomp");
    int i_struc;
    for (i_struc = 0; i_struc < spec.n_strucs; i_struc++) {
      check(ERR_OK == ppair_structure(&(leafres.strucs[i_struc]), 
            &(spec.strucs[i_struc]), &(states.states[i_struc]), &seqs, &spec),
          "Error getting ppair structure");
    }

    free_states(&states);

    init_states(&states, &spec);

    check(ERR_OK == get_decomposition(&states, 20, &spec), 
        "Error decomposing");

    free_result(&leafres);
    init_result(&leafres, &spec);

    update_result(&leafres, &states, &seqs, &spec);

    print_design_result(stdout, &leafres, &seqs, 0, &spec, "decomp");

    update_result(&result, &states, &seqs, &spec);
    // print_design_result(stdout, &result, &seqs, 0, &spec, "decomp");

    // copy_result(&result, &leafres);

    // print_design_result(stdout, &result, &seqs, 0, &spec, "decomp");

    optimize_forest(&result, &seqs, &spec);
    // optimize_leaves(&result, &seqs, &states, 0.01, &spec);

    print_design_result(stdout, &result, &seqs, 0, &spec, "decomp");

    free_result(&leafres);
    free_states(&states);

    // outf = fopen(output_fn, "w");
    // print_design_result(outf, &result, &seqs, 0, &spec, "decomp");
    // fclose(outf);
    // outf = NULL;

    // if (spec.opts.print_leaves) {
    //   output_fn_leaves = (char *) malloc((input_len + 16) * sizeof(char));
    //   strncpy(output_fn_leaves, argv[1], input_len + 1);
    //   strncat(output_fn_leaves, "_leaves_0.npo", 16);

    //   leafoutf = fopen(output_fn_leaves, "w");

    //   init_states(&states, &spec);
    //   init_result(&leafres, &spec);

    //   get_decomposition(&states, 20, &spec);
    //   update_result(&leafres, &states, &seqs, &spec);

    //   print_design_result(leafoutf, &leafres, &seqs, 0, &spec, "decomp");

    //   free_result(&leafres);
    //   free_states(&states);
    //   fclose(leafoutf);
    //   leafoutf = NULL;
    // }
    free_design_spec(&spec);
    free_seqstate(&seqs);
    free_result(&result);
  } else {
  }

  free(input_prefix);
  free(input_fn);
  free(output_fn);
  free(output_fn_leaves);

  return 0;
error:
  log_err("Error occurred in %s. Exiting", argv[0]);
  free(input_prefix);
  free(input_fn);
  free(output_fn);
  free(output_fn_leaves);
  return 2;
}
