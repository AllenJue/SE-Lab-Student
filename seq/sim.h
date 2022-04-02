
/********** Defines **************/

/* Get ra out of one byte regid field */
#define GET_RA(r) HI4(r)

/* Get rb out of one byte regid field */
#define GET_RB(r) LO4(r)


/************ Global state declaration ****************/

/* Both instruction and data memory */
extern mem_t mem;

/* Register file */
extern mem_t reg;
/* Condition code register */
extern cc_t cc;
/* Program counter */
extern word_t pc;

/* Intermdiate stage values that must be used by control functions */
extern byte_t imem_icode;
extern byte_t imem_ifun;
extern byte_t icode;
extern word_t ifun;
extern word_t ra;
extern word_t rb;
extern word_t valc;
extern word_t valp;
extern bool imem_error;
extern bool instr_valid;
extern word_t vala;
extern word_t valb;
extern word_t vale;
extern bool bcond;
extern bool cond;
extern word_t valm;
extern bool dmem_error;
extern byte_t status;

/* Log file */
extern FILE *dumpfile;


/* Sets the simulator name (called from main routine in HCL file) */
void set_simname(char *name);

/* Initialize simulator */
void sim_init();

/* Reset simulator state, including register, instruction, and data memories */
void sim_reset();

/*
  Run processor until one of following occurs:
  - An status error is encountered
  - max_instr instructions have completed

  Return number of instructions executed.
  if statusp nonnull, then will be set to status of final instruction
  if ccp nonnull, then will be set to condition codes of final instruction
*/
word_t sim_run(word_t max_instr, byte_t *statusp, cc_t *ccp);

/* If dumpfile set nonNULL, lots of status info printed out */
void sim_set_dumpfile(FILE *file);

/*
 * sim_log dumps a formatted string to the dumpfile, if it exists
 * accepts variable argument list
 */
void sim_log( const char *format, ... );

								       
