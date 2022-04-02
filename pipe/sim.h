
/********** Typedefs ************/

/* Pipeline stage identifiers for stage operation control */
typedef enum { FETCH_STAGE, DECODE_STAGE, EXECUTE_STAGE, MEMORY_STAGE, WRITEBACK_STAGE } stage_id_t;

/********** Defines **************/

/* Get ra out of one byte regid field */
#define GET_RA(r) HI4(r)

/* Get rb out of one byte regid field */
#define GET_RB(r) LO4(r)

/*************** Simulation Control Functions ***********/

/* Bubble next execution of specified stage */
void sim_bubble_stage(stage_id_t stage);

/* Stall stage (has effect at next update) */
void sim_stall_stage(stage_id_t stage);

/* Initialize simulator */
void sim_init();

/* Reset simulator state, including register, instruction, and data memories */
void sim_reset();

/*
  Run pipeline until one of following occurs:
  - A status error is encountered in WB.
  - max_instr instructions have completed through WB
  - max_cycle cycles have been simulated

  Return number of instructions executed.
  if statusp nonnull, then will be set to status of final instruction
  if ccp nonnull, then will be set to condition codes of final instruction
*/
word_t sim_run_pipe(word_t max_instr, word_t max_cycle, byte_t *statusp, cc_t *ccp);

/* If dumpfile set nonNULL, lots of status info printed out */
void sim_set_dumpfile(FILE *file);

/*
 * sim_log dumps a formatted string to the dumpfile, if it exists
 * accepts variable argument list
 */
void sim_log( const char *format, ... );

