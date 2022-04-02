/* 
 * stages.h - Defines the layout of the pipe registers
 * Declares the functions that implement the pipeline stages
*/

/********** Pipeline register contents **************/

/* Program Counter */
typedef struct {
    word_t predPC;
    stat_t status;
} fetch_ele, *fetch_ptr;

/* IF/ID Pipe Register */
typedef struct {
    byte_t icode;  /* Single byte instruction code */
    byte_t ifun;    /* ALU/JMP qualifier */
    byte_t ra; /* Register ra ID */
    byte_t rb; /* Register rb ID */
    word_t valc;  /* Instruction word encoding immediate data */
    word_t valp; /* Incremented program counter */
    stat_t status;
    /* The following is included for debugging */
    word_t stage_pc;
} decode_ele, *decode_ptr;

/* ID/EX Pipe Register */
typedef struct {
    byte_t icode;        /* Instruction code */
    byte_t ifun;        /* ALU/JMP qualifier */
    word_t valc;        /* Immediate data */
    word_t vala;        /* valA */
    word_t valb;        /* valB */
    byte_t srca;  /* Source Reg ID for valA */
    byte_t srcb;  /* Source Reg ID for valB */
    byte_t deste; /* Destination register for valE */
    byte_t destm; /* Destination register for valM */
    stat_t status;
    /* The following is included for debugging */
    word_t stage_pc;
} execute_ele, *execute_ptr;

/* EX/MEM Pipe Register */
typedef struct {
    byte_t icode;        /* Instruction code */
    byte_t ifun;          /* ALU/JMP qualifier */
    bool takebranch;  /* Taken branch signal */
    word_t vale;        /* valE */
    word_t vala;        /* valA */
    byte_t deste; /* Destination register for valE */
    byte_t destm; /* Destination register for valM */
    byte_t srca;  /* Source register for valA */
    stat_t status;
    /* The following is included for debugging */
    word_t stage_pc;
} memory_ele, *memory_ptr;

/* Mem/WB Pipe Register */
typedef struct {
    byte_t icode;        /* Instruction code */
    byte_t ifun;         /* ALU/JMP qualifier */
    word_t vale;         /* valE */
    word_t valm;         /* valM */
    byte_t deste; /* Destination register for valE */
    byte_t destm; /* Destination register for valM */
    stat_t status;
    /* The following is included for debugging */
    word_t stage_pc;
} writeback_ele, *writeback_ptr;

/************ Global Declarations ********************/

extern fetch_ele bubble_fetch;
extern decode_ele bubble_decode;
extern execute_ele bubble_execute;
extern memory_ele bubble_memory;
extern writeback_ele bubble_writeback;

/************ Function declarations *******************/

/* Stage functions */
void do_fetch_stage();
void do_decode_stage();  
void do_execute_stage();
void do_memory_stage();
void do_writeback_stage();

/* Set stalling conditions for different stages */
void do_stall_check();


