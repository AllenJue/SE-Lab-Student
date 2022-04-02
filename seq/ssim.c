/***********************************************************************
 *
 * ssim.c - Sequential Y86-64 simulator
 *
 * Copyright (c) 2002, 2015. Bryant and D. O'Hallaron, All rights reserved.
 * May not be used, modified, or copied without permission.
 * 
 * Updated 2021: M. Hinton
 ***********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include "isa.h"
#include "sim.h"

/***************
 * Begin Globals
 ***************/

char simname[] = "Y86-64 Processor: SEQ";

/* Parameters modifed by the command line */
char *object_filename;   /* The input object file name. */
FILE *object_file;       /* Input file handle */
int verbosity = 2;    /* Verbosity level [TTY only] (-v) */
word_t instr_limit = 10000; /* Instruction limit [TTY only] (-l) */

/* keep a copy of mem and reg for diff display */
mem_t mem0, reg0;

/***************************
 * Begin function prototypes
 ***************************/

static void usage(char *name);           /* Print helpful usage message */
static void run_tty_sim();               /* Run simulator in TTY mode */
static void sim_interactive();

/*************************
 * End function prototypes
 *************************/


/*******************************************************************
 * Part 1: This part is the initial entry point that handles general
 * initialization. It parses the command line and does any necessary
 * setup to run in TTY mode, and then starts the
 * simulation.
 * Do not change any of these.
 *******************************************************************/

/*
 * sim_main - main simulator routine. This function is called from the
 * main() routine.
 */
int sim_main(int argc, char **argv)
{
    int i;
    int c;
    int interactive = 0;

    /* Parse the command line arguments */
    while ((c = getopt(argc, argv, "ihtl:v:")) != -1) {
	switch(c) {
	case 'h':
	    usage(argv[0]);
	    break;
	case 'l':
	    instr_limit = atoll(optarg);
	    break;
	case 'v':
	    verbosity = atoi(optarg);
	    if (verbosity < 0 || verbosity > 3) {
		printf("Invalid verbosity %d\n", verbosity);
		usage(argv[0]);
	    }
	    break;
    case 'i':
        interactive = true;
        break;
	default:
	    printf("Invalid option '%c'\n", c);
	    usage(argv[0]);
	    break;
	}
    }


    /* Do we have too many arguments? */
    if (optind < argc - 1) {
	printf("Too many command line arguments:");
	for (i = optind; i < argc; i++)
	    printf(" %s", argv[i]);
	printf("\n");
	usage(argv[0]);
    }


    /* The single unflagged argument should be the object file name */
    object_filename = NULL;
    object_file = NULL;
    if (optind < argc) {
        object_filename = argv[optind];
        object_file = fopen(object_filename, "r");
        if (!object_file) {
            fprintf(stderr, "Couldn't open object file %s\n", object_filename);
            exit(1);
        }
    }

    if (object_filename == NULL) {
        fprintf(stderr, "No object file specified\n");
        exit(1);
    }

    if(interactive) {
        sim_interactive();
    } else {
        run_tty_sim();
    }

    exit(0);
}


int main(int argc, char *argv[]) {return sim_main(argc,argv);}


/*
 * run_tty_sim - Run the simulator in TTY mode
 */
static void run_tty_sim()
{
    word_t icount = 0;
    status = STAT_AOK;
    cc_t result_cc = 0;
    word_t byte_cnt = 0;
    state_ptr isa_state = NULL;

    /* Initializations */
    if (verbosity >= 2)
	sim_set_dumpfile(stdout);
    sim_init();

    /* Emit simulator name */
    printf("%s\n", simname);

    byte_cnt = load_mem(mem, object_file, 1);
    if (byte_cnt == 0) {
	fprintf(stderr, "No lines of code found\n");
	exit(1);
    } else if (verbosity >= 2) {
	printf("%lld bytes of code read\n", byte_cnt);
    }
    fclose(object_file);

	isa_state = new_state(0);
	free_mem(isa_state->r);
	free_mem(isa_state->m);
	isa_state->m = copy_mem(mem);
	isa_state->r = copy_mem(reg);
	isa_state->cc = cc;

    mem0 = copy_mem(mem);
    reg0 = copy_mem(reg);

    icount = sim_run(instr_limit, &status, &result_cc);
    if (verbosity > 0) {
        printf("%lld instructions executed\n", icount);
        printf("Status = %s\n", stat_name(status));
        printf("Condition Codes: %s\n", cc_name(result_cc));
        printf("Changed Register State:\n");
        diff_reg(reg0, reg, stdout);
        printf("Changed Memory State:\n");
        diff_mem(mem0, mem, stdout);
    }

	byte_t e = STAT_AOK;
	int step;
	bool match = true;

	for (step = 0; step < instr_limit && e == STAT_AOK; step++) {
	    e = step_state(isa_state, stdout);
	}

	if (diff_reg(isa_state->r, reg, NULL)) {
	    match = false;
	    if (verbosity > 0) {
            printf("ISA Register != Pipeline Register File\n");
            diff_reg(isa_state->r, reg, stdout);
	    }
	}
	if (diff_mem(isa_state->m, mem, NULL)) {
	    match = false;
	    if (verbosity > 0) {
            printf("ISA Memory != Pipeline Memory\n");
            diff_mem(isa_state->m, mem, stdout);
	    }
	}
	if (isa_state->cc != result_cc) {
	    match = false;
	    if (verbosity > 0) {
            printf("ISA Cond. Codes (%s) != Pipeline Cond. Codes (%s)\n",
                cc_name(isa_state->cc), cc_name(result_cc));
	    }
	}
	if (match) {
	    printf("ISA Check Succeeds\n");
	} else {
	    printf("ISA Check Fails\n");
	}
}



/*
 * usage - print helpful diagnostic information
 */
static void usage(char *name)
{
    printf("Usage: %s [-hi] [-l m] [-v n] file.yo\n", name);
    printf("   -h     Print this message\n");
    printf("   -l m   Set instruction limit to m [non interactive mode only] (default %lld)\n", instr_limit);
    printf("   -v n   Set verbosity level to 0 <= n <= 3 (default %d)\n", verbosity);
    printf("   -i     Runs the simulator in interactive mode\n");
    exit(0);
}



/*********************************************************
 * Part 2: This part contains the core simulator routines.
 * You only need to modify function sim_step()
 *********************************************************/

/**********************
 * Begin Part 2 Globals
 **********************/

/*
 * Variables related to hardware units in the processor
 */
mem_t mem;  /* Instruction and data memory */

/* Other processor state */
mem_t reg;               /* Register file */
cc_t cc = DEFAULT_CC;    /* Condition code register */
cc_t cc_in = DEFAULT_CC; /* Input to condition code register */

/* Program Counter */
word_t pc = 0; /* Program counter value */
word_t pc_in = 0;/* Input to program counter */

/* Intermediate values */
byte_t imem_icode = I_NOP;
byte_t imem_ifun = F_NONE;
byte_t icode = I_NOP;
word_t ifun = 0;
byte_t instr = HPACK(I_NOP, F_NONE);
word_t ra = REG_NONE;
word_t rb = REG_NONE;
word_t valc = 0;
word_t valp = 0;
bool imem_error;
bool instr_invalid;

word_t srcA = REG_NONE;
word_t srcB = REG_NONE;
word_t destE = REG_NONE;
word_t destM = REG_NONE;
word_t vala = 0;
word_t valb = 0;
word_t vale = 0;

bool bcond = false;
bool cond = false;
word_t valm = 0;
bool dmem_error;

bool mem_write = false;
word_t mem_addr = 0;
word_t mem_data = 0;
byte_t status = STAT_AOK;

/* Log file */
FILE *dumpfile = NULL;


/********************
 * End Part 2 Globals
 ********************/


static int initialized = 0;
void sim_init()
{

    /* Create memory and register files */
    initialized = 1;
    mem = init_mem(MEM_SIZE);
    reg = init_reg();
    sim_reset();
    clear_mem(mem);
}

void sim_reset()
{
    if (!initialized)
	sim_init();
    clear_mem(reg);

	pc_in = 0;
    cc = DEFAULT_CC;
    cc_in = DEFAULT_CC;
    destE = REG_NONE;
    destM = REG_NONE;
    mem_write = false;
    mem_addr = 0;
    mem_data = 0;

    /* Reset intermediate values to clear display */
    icode = I_NOP;
    ifun = 0;
    instr = HPACK(I_NOP, F_NONE);
    ra = REG_NONE;
    rb = REG_NONE;
    valc = 0;
    valp = 0;

    srcA = REG_NONE;
    srcB = REG_NONE;
    destE = REG_NONE;
    destM = REG_NONE;
    vala = 0;
    valb = 0;
    vale = 0;

    cond = false;
    bcond = false;
    valm = 0;
}

/*****************************************************************
 * This is the only function you need to modify for SEQ simulator.
 * It executes one instruction but split it into multiple stages.
 * For each stage, you should update the corresponding intermediate
 * values. At the end of this function, you should make sure the
 * global state values [mem, reg, cc, pc] are updated correctly,
 * and then return the correct status.
 *****************************************************************/

static byte_t sim_step()
{
    status = STAT_AOK;
    instr_invalid = imem_error = dmem_error = false;

    /*********************** Fetch stage ************************/

		byte_t tempB;
		imem_error |= !get_byte_val(mem, pc, &instr);
		icode = HI4(instr);
        ifun = LO4(instr);
		ra = REG_NONE;
		rb = REG_NONE;
		valc = 0;

		switch (instr) {
			case HPACK(I_NOP, F_NONE):
				valp = pc + 1;
				break;
			case HPACK(I_HALT, F_NONE):
				valp = pc + 1;
				break;

			case HPACK(I_RRMOVQ, F_NONE):
			case HPACK(I_RRMOVQ, C_LE):
			case HPACK(I_RRMOVQ, C_L):
			case HPACK(I_RRMOVQ, C_E):
			case HPACK(I_RRMOVQ, C_NE):
			case HPACK(I_RRMOVQ, C_GE):
			case HPACK(I_RRMOVQ, C_G):
				imem_error |= !get_byte_val(mem, pc + 1, &tempB);
				ra = HI4(tempB);
				rb = LO4(tempB);
				valp = pc + 2;
				break;

			case HPACK(I_IRMOVQ, F_NONE):
				imem_error |= !get_byte_val(mem, pc + 1, &tempB);
				rb = LO4(tempB);
				imem_error |= !get_word_val(mem, pc + 2, &valc);
				valp = pc + 10;
				break;

			case HPACK(I_RMMOVQ, F_NONE):
				imem_error |= !get_byte_val(mem, pc + 1, &tempB);
				ra = HI4(tempB);
				rb = LO4(tempB);
				imem_error |= !get_word_val(mem, pc + 2, &valc);
				valp = pc + 10;
				break;

			case HPACK(I_MRMOVQ, F_NONE):
				imem_error |= !get_byte_val(mem, pc + 1, &tempB);
				ra = HI4(tempB);
				rb = LO4(tempB);
				imem_error |= !get_word_val(mem, pc + 2, &valc);
				valp = pc + 10;
				break;

			case HPACK(I_ALU, A_ADD):
			case HPACK(I_ALU, A_SUB):
			case HPACK(I_ALU, A_AND):
			case HPACK(I_ALU, A_XOR):
				imem_error |= !get_byte_val(mem, pc + 1, &tempB);
				ra = HI4(tempB);
				rb = LO4(tempB);
				valp = pc + 2;
				break;

			case HPACK(I_JMP, C_YES):
			case HPACK(I_JMP, C_LE):
			case HPACK(I_JMP, C_L):
			case HPACK(I_JMP, C_E):
			case HPACK(I_JMP, C_NE):
			case HPACK(I_JMP, C_GE):
			case HPACK(I_JMP, C_G):
				imem_error |= !get_word_val(mem, pc + 1, &valc);
				valp = pc + 9;
				break;

			case HPACK(I_CALL, F_NONE):
				imem_error |= !get_word_val(mem, pc + 1, &valc);
				valp = pc + 9;
				break;

			case HPACK(I_RET, F_NONE):
				valp = pc + 1;
				break;

			case HPACK(I_PUSHQ, F_NONE):
				imem_error |= !get_byte_val(mem, pc + 1, &tempB);
				ra = HI4(tempB);
				rb = LO4(tempB);
				valp = pc + 2;
				break;

			case HPACK(I_POPQ, F_NONE):
				imem_error |= !get_byte_val(mem, pc + 1, &tempB);
				ra = HI4(tempB);
				rb = LO4(tempB);
				valp = pc + 2;
				break;

			default:
				instr_invalid = true;
				printf("Invalid instruction\n");
				break;
		}

    sim_log("IF: Fetched %s at 0x%llx.  ra=%s, rb=%s, valC = 0x%llx\n",
	    iname(HPACK(icode,ifun)), pc, reg_name(ra), reg_name(rb), valc);

    /*********************** Decode stage ************************/
    srcA = REG_NONE;
    srcB = REG_NONE;
    destE = REG_NONE;
    destM = REG_NONE;
    vala = 0;
    valb = 0;
		switch (icode) {
			case I_HALT: break;

			case I_NOP: break;

			case I_RRMOVQ: // aka CMOVQ
				srcA = ra;
				destE = rb;
				break;

			case I_IRMOVQ:
				destE = rb;
				break;

			case I_RMMOVQ:
				srcA = ra;
				srcB = rb;
				break;

			case I_MRMOVQ:
				srcB = rb;
				destM = ra;
				break;

			case I_ALU:
				srcA = ra;
				srcB = rb;
				destE = rb;
				break;

			case I_JMP: break;

			case I_CALL:
				srcB = REG_RSP;
				destE = REG_RSP;
				break;

			case I_RET:
				srcA = REG_RSP;
				srcB = REG_RSP;
				destE = REG_RSP;
				break;

			case I_PUSHQ:
				srcA = ra;
				srcB = REG_RSP;
				destE = REG_RSP;
				break;

			case I_POPQ:
				srcA = REG_RSP;
				srcB = REG_RSP;
				destE = REG_RSP;
				destM = ra;
				break;

			default:
				printf("icode is not valid (%d)", icode);
				break;
		}

		vala = get_reg_val(reg, srcA);
		valb = get_reg_val(reg, srcB);

    /*********************** Execute stage **********************/

    vale = 0;
    cc_in = cc;
		bool cnd = false;

		switch (icode) {
			case I_HALT: break;

			case I_NOP: break;

			case I_RRMOVQ: // aka CMOVQ
				cnd = cond_holds(cc, ifun);
				vale = vala;
				if (!cnd) {
					destE = REG_NONE;
				}
				break;

			case I_IRMOVQ:
				vale = valc;
				break;

			case I_RMMOVQ:
				vale = valb + valc;
				break;

			case I_MRMOVQ:
				vale = valb + valc;
				break;

			case I_ALU:
				vale = compute_alu(ifun, vala, valb);
				cc_in = compute_cc(ifun, vala, valb);
				break;

			case I_JMP:
				cnd = cond_holds(cc, ifun);
				break;

			case I_CALL:
				vale = valb - 8;
				break;

			case I_RET:
				vale = valb + 8;
				break;

			case I_PUSHQ:
				vale = valb - 8;
				break;

			case I_POPQ:
				vale = valb + 8;
				break;

			default:
				printf("icode is not valid (%d)", icode);
				break;
		}

    /*********************** Memory stage ***********************/

    valm = 0;
    mem_write = false;
    mem_addr = 0;
    mem_data = 0;
    status = STAT_AOK;

		switch (icode) {
			case I_HALT:
				status = STAT_HLT;
				break;

			case I_NOP: break;

			case I_RRMOVQ: break; // aka CMOVQ

			case I_IRMOVQ: break;

			case I_RMMOVQ:
				mem_write = true;
				mem_addr = vale;
				mem_data = vala;
				break;

			case I_MRMOVQ:
				dmem_error |= !get_word_val(mem, vale, &valm);
				break;

			case I_ALU: break;

			case I_JMP: break;

			case I_CALL:
				mem_write = true;
				mem_addr = vale;
				mem_data = valp;
				break;

			case I_RET:
				dmem_error |= !get_word_val(mem, vala, &valm);
				break;

			case I_PUSHQ:
				mem_write = true;
				mem_addr = vale;
				mem_data = vala;
				break;

			case I_POPQ:
				dmem_error |= !get_word_val(mem, vala, &valm);
				break;

			default:
				printf("icode is not valid (%d)", icode);
				break;
		}

		if (mem_write && !instr_invalid) {
			dmem_error |= !set_word_val(mem, mem_addr, mem_data);
	        sim_log("Wrote 0x%llx to address 0x%llx\n", mem_data, mem_addr);
        }


    /****************** Program Counter Update ******************/

    pc_in = 0;

		switch (icode) {
			case I_HALT:
				pc_in = valp;
				break;

			case I_NOP:
				pc_in = valp;
				break;

			case I_RRMOVQ: // aka CMOVQ
				pc_in = valp;
				break;

			case I_IRMOVQ:
				pc_in = valp;
				break;

			case I_RMMOVQ:
				pc_in = valp;
				break;

			case I_MRMOVQ:
				pc_in = valp;
				break;

			case I_ALU:
				pc_in = valp;
				break;

			case I_JMP:
				pc_in = cnd ? valc : valp;
				break;

			case I_CALL:
				pc_in = valc;
				break;

			case I_RET:
				pc_in = valm;
				break;

			case I_PUSHQ:
				pc_in = valp;
				break;

			case I_POPQ:
				pc_in = valp;
				break;

			default:
				printf("icode is not valid (%d)", icode);
				break;

		}

    pc = pc_in;
    cc = cc_in;
    /* Writeback */
    if (destE != REG_NONE && !imem_error && !dmem_error && !instr_invalid)
        set_reg_val(reg, destE, vale);
    if (destM != REG_NONE && !dmem_error && !imem_error && !instr_invalid)
        set_reg_val(reg, destM, valm);



    return instr_invalid
			? STAT_INS
			: (dmem_error || imem_error)
			? STAT_ADR
			: status;
}

/*
  Run processor until one of following occurs:
  - An error status is encountered in WB.
  - max_instr instructions have completed through WB

  Return number of instructions executed.
  if statusp nonnull, then will be set to status of final instruction
  if ccp nonnull, then will be set to condition codes of final instruction
*/
word_t sim_run(word_t max_instr, byte_t *statusp, cc_t *ccp)
{
    word_t icount = 0;
    byte_t run_status = STAT_AOK;
    while (icount < max_instr) {
        if (verbosity == 3) {
            sim_log("-------- Step %d --------\n", icount + 1);
        }
        run_status = sim_step();
        icount++;

        /* print step-wise diff if verbosity = 3 */
        if (verbosity == 3) {
            sim_log("Status '%s', CC %s\n", stat_name(status), cc_name(cc_in));
            sim_log("Changes to registers:\n");
            diff_reg(reg0, reg, stdout);

            printf("\nChanges to memory:\n");
            diff_mem(mem0, mem, stdout);
            printf("\n");
        }

        if (run_status != STAT_AOK)
            break;
    }
    if (statusp)
	*statusp = run_status;
    if (ccp)
	*ccp = cc;
    return icount;
}

/*
 * help - Prints the help information for the Trace Runner.
 */
void help() {
    printf("----------------ssim Help-----------------------\n");
    printf("go                -  run program to completion\n");
    printf("next n            -  advance n instructions\n");
    printf("memory            -  display differences in memory\n");
    printf("registers         -  display differences in registers\n");
    printf("arch              -  display processor state\n");
    printf("undo n            -  steps back n instructions\n");
    printf("quit              -  exit the program\n\n");
}

typedef struct seq_restore_struct {
    processor_state_t state;
    struct seq_restore_struct *next;
} seq_restore_t;

static seq_restore_t *create_restore_point(word_t icount, stat_t old_status, word_t old_pc, cc_t old_cc, mem_t previous_memory, mem_t previous_register) {
    seq_restore_t *restore_point = malloc(sizeof(seq_restore_t));
    restore_point->state.cc = old_cc;
    restore_point->state.icount = icount;
    restore_point->state.memory = create_memory_restore(previous_memory, mem);
    restore_point->state.registers = create_memory_restore(previous_register, reg);
    restore_point->state.status = old_status;
    restore_point->state.pc = old_pc;
    return restore_point;
}

static void free_restore_point(seq_restore_t *restore_point) {
    if (restore_point->state.memory->positions != NULL)
        free(restore_point->state.memory->positions);
    if (restore_point->state.memory->values != NULL)
        free(restore_point->state.memory->values);
    free(restore_point->state.memory);

    if (restore_point->state.registers->positions != NULL)
        free(restore_point->state.registers->positions);
    if (restore_point->state.registers->values != NULL)
        free(restore_point->state.registers->values);
    free(restore_point->state.registers);

    free(restore_point);
}

void sim_interactive()
{
    word_t icount = 0;
    word_t ucount = 0;
    status = STAT_AOK;
    word_t byte_cnt = 0;
    int instructions_to_run, instructions_to_undo;
    word_t icount_stored = 0;

	sim_set_dumpfile(stdout);
    sim_init();

    /* Emit simulator name */
    printf("%s\n", simname);

    byte_cnt = load_mem(mem, object_file, 1);
    if (byte_cnt == 0) {
	    fprintf(stderr, "No lines of code found\n");
	    exit(1);
    } else if (verbosity >= 2) {
	    printf("%lld bytes of code read\n", byte_cnt);
    }
    fclose(object_file);

    mem0 = copy_mem(mem);
    reg0 = copy_mem(reg);

    mem_t previous_memory = copy_mem(mem);
    mem_t previous_registers = copy_mem(reg);

    seq_restore_t *restore_head = NULL;

    char buffer[20];
    byte_t run_status = STAT_AOK;

    while(1) {
        printf("SEQ> ");

        int size = scanf("%s", buffer);
            if (size == 0) {
            buffer[0] = 'x';
        }
        printf("\n");

        switch(buffer[0]) {
        case 'G':
        case 'g':
            if(run_status == STAT_AOK || run_status == STAT_BUB) {
                icount_stored = icount;
                while (run_status == STAT_AOK) {
                    previous_memory = copy_mem(mem);
                    previous_registers = copy_mem(reg);
                    word_t old_pc = pc;
                    stat_t old_status = run_status;
                    cc_t old_cc = cc;
                    run_status = sim_step();
                    seq_restore_t *new_restore_point = create_restore_point(icount, old_status, old_pc, old_cc, previous_memory, previous_registers);
                    icount++;
                    new_restore_point->next = restore_head;
                    restore_head = new_restore_point;
                }

                printf("Simulator Ran %lld instructions\n", icount - icount_stored);
            } else {
                printf("Simulator is in a non AOK state\n");
            }

            printf("Simulator ran to completion\n");
            break;

        case 'h':
        case 'H':
            help();
            break;

        case 'Q':
        case 'q':
            printf("Bye.\n");
            exit(0);

        case 'N':
        case 'n':
            size = scanf("%d", &instructions_to_run);
            if (size == 0) {
                break;
            }

            if(run_status == STAT_AOK || run_status == STAT_BUB) {
                icount_stored = icount;
                while (run_status == STAT_AOK && instructions_to_run--) {
                    previous_memory = copy_mem(mem);
                    previous_registers = copy_mem(reg);
                    word_t old_pc = pc;
                    stat_t old_status = run_status;
                    cc_t old_cc = cc;
                    run_status = sim_step();
                    seq_restore_t *new_restore_point = create_restore_point(icount, old_status, old_pc, old_cc, previous_memory, previous_registers);
                    icount++;
                    new_restore_point->next = restore_head;
                    restore_head = new_restore_point;
                }

                printf("Simulator ran %lld instructions\n", icount - icount_stored);
            } else {
                printf("Simulator is in a non AOK state\n");
            }

            if (run_status != STAT_AOK) {
                printf("Simulator ran to completion\n");
            }

            break;

        case 'M':
        case 'm':
            diff_mem(mem0, mem, stdout);
            break;

        case 'R':
        case 'r':
            diff_reg(reg0, reg, stdout);
            break;

        case 'U':
        case 'u':
            size = scanf("%d", &instructions_to_undo);
            if (size == 0) {
                break;
            }
            ucount = 0;
            while (restore_head != NULL && instructions_to_undo--) {
                cc = restore_head->state.cc;
                icount = restore_head->state.icount;
                apply_restore(mem, restore_head->state.memory);
                apply_restore(reg, restore_head->state.registers);
                pc = restore_head->state.pc;
                status = restore_head->state.status;
                run_status = status;
                seq_restore_t *temp = restore_head;
                restore_head = restore_head->next;
                free_restore_point(temp);
                ucount++;
            }
            printf("Instructions undone: %lld\n", ucount);

        case 'A':
        case 'a':
            printf("Status: %s\n", stat_name(status));
            printf("PC: %llx\n", pc);
            printf("CC: %s\n", cc_name(cc));
            printf("Instructions Completed: %lld\n", icount);
            dump_reg(stdout, reg);
            break;


        default:
            printf("Invalid Command\n");
            break;
        }
    }
}

/* If dumpfile set nonNULL, lots of status info printed out */
void sim_set_dumpfile(FILE *df)
{
    dumpfile = df;
}

/*
 * sim_log dumps a formatted string to the dumpfile, if it exists
 * accepts variable argument list
 */
void sim_log( const char *format, ... ) {
    if (dumpfile) {
	va_list arg;
	va_start( arg, format );
	vfprintf( dumpfile, format, arg );
	va_end( arg );
    }
}