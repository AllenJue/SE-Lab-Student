#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "isa.h"
#ifdef CACHE_ENABLED
#include "cache.h"

extern cache_t* cache;
#endif

/* Bytes Per Line = Block size of memory */
#define BPL 32

struct {
    char *name;
    int id;
} reg_table[REG_ERR+1] = 
{
    {"%rax",   REG_RAX},
    {"%rcx",   REG_RCX},
    {"%rdx",   REG_RDX},
    {"%rbx",   REG_RBX},
    {"%rsp",   REG_RSP},
    {"%rbp",   REG_RBP},
    {"%rsi",   REG_RSI},
    {"%rdi",   REG_RDI},
    {"%r8",   REG_R8},
    {"%r9",   REG_R9},
    {"%r10",   REG_R10},
    {"%r11",   REG_R11},
    {"%r12",   REG_R12},
    {"%r13",   REG_R13},
    {"%r14",   REG_R14},
    {"----",   REG_NONE},
    {"----",   REG_ERR}
};


reg_id_t find_register(char *name)
{
    int i;
    for (i = 0; i < REG_NONE; i++)
	if (!strcmp(name, reg_table[i].name))
	    return reg_table[i].id;
    return REG_ERR;
}

char *reg_name(reg_id_t id)
{
    if (id >= 0 && id < REG_NONE)
	return reg_table[id].name;
    else
	return reg_table[REG_NONE].name;
}

/* Is the given register ID a valid program register? */
int reg_valid(reg_id_t id)
{
  return id >= 0 && id < REG_NONE && reg_table[id].id == id;
}

instr_t instruction_set[] = 
{
    {"nop",    HPACK(I_NOP, F_NONE), 1, NO_ARG, 0, 0, NO_ARG, 0, 0 },
    {"halt",   HPACK(I_HALT, F_NONE), 1, NO_ARG, 0, 0, NO_ARG, 0, 0 },
    {"rrmovq", HPACK(I_RRMOVQ, F_NONE), 2, R_ARG, 1, 1, R_ARG, 1, 0 },
    /* Conditional move instructions are variants of RRMOVQ */
    {"cmovle", HPACK(I_RRMOVQ, C_LE), 2, R_ARG, 1, 1, R_ARG, 1, 0 },
    {"cmovl", HPACK(I_RRMOVQ, C_L), 2, R_ARG, 1, 1, R_ARG, 1, 0 },
    {"cmove", HPACK(I_RRMOVQ, C_E), 2, R_ARG, 1, 1, R_ARG, 1, 0 },
    {"cmovne", HPACK(I_RRMOVQ, C_NE), 2, R_ARG, 1, 1, R_ARG, 1, 0 },
    {"cmovge", HPACK(I_RRMOVQ, C_GE), 2, R_ARG, 1, 1, R_ARG, 1, 0 },
    {"cmovg", HPACK(I_RRMOVQ, C_G), 2, R_ARG, 1, 1, R_ARG, 1, 0 },
    /* arg1hi indicates number of bytes */
    {"irmovq", HPACK(I_IRMOVQ, F_NONE), 10, I_ARG, 2, 8, R_ARG, 1, 0 },
    {"rmmovq", HPACK(I_RMMOVQ, F_NONE), 10, R_ARG, 1, 1, M_ARG, 1, 0 },
    {"mrmovq", HPACK(I_MRMOVQ, F_NONE), 10, M_ARG, 1, 0, R_ARG, 1, 1 },
    {"addq",   HPACK(I_ALU, A_ADD), 2, R_ARG, 1, 1, R_ARG, 1, 0 },
    {"subq",   HPACK(I_ALU, A_SUB), 2, R_ARG, 1, 1, R_ARG, 1, 0 },
    {"andq",   HPACK(I_ALU, A_AND), 2, R_ARG, 1, 1, R_ARG, 1, 0 },
    {"xorq",   HPACK(I_ALU, A_XOR), 2, R_ARG, 1, 1, R_ARG, 1, 0 },
    /* arg1hi indicates number of bytes */
    {"jmp",    HPACK(I_JMP, C_YES), 9, I_ARG, 1, 8, NO_ARG, 0, 0 },
    {"jle",    HPACK(I_JMP, C_LE), 9, I_ARG, 1, 8, NO_ARG, 0, 0 },
    {"jl",     HPACK(I_JMP, C_L), 9, I_ARG, 1, 8, NO_ARG, 0, 0 },
    {"je",     HPACK(I_JMP, C_E), 9, I_ARG, 1, 8, NO_ARG, 0, 0 },
    {"jne",    HPACK(I_JMP, C_NE), 9, I_ARG, 1, 8, NO_ARG, 0, 0 },
    {"jge",    HPACK(I_JMP, C_GE), 9, I_ARG, 1, 8, NO_ARG, 0, 0 },
    {"jg",     HPACK(I_JMP, C_G), 9, I_ARG, 1, 8, NO_ARG, 0, 0 },
    {"call",   HPACK(I_CALL, F_NONE),    9, I_ARG, 1, 8, NO_ARG, 0, 0 },
    {"ret",    HPACK(I_RET, F_NONE), 1, NO_ARG, 0, 0, NO_ARG, 0, 0 },
    {"pushq",  HPACK(I_PUSHQ, F_NONE) , 2, R_ARG, 1, 1, NO_ARG, 0, 0 },
    {"popq",   HPACK(I_POPQ, F_NONE) ,  2, R_ARG, 1, 1, NO_ARG, 0, 0 },
    {"leaq",   HPACK(I_LEAQ, F_NONE), 10, M_ARG, 1, 0, R_ARG, 1, 1 },
    {"vecadd", HPACK(I_VECADD, F_NONE), 2, R_ARG, 1, 1, R_ARG, 1, 0 },
    {"shlq", HPACK(I_SHF, S_HL), 2, R_ARG, 1, 1, R_ARG, 1, 0 },
    {"shrq", HPACK(I_SHF, S_HR), 2, R_ARG, 1, 1, R_ARG, 1, 0 },
    {"sarq", HPACK(I_SHF, S_AR), 2, R_ARG, 1, 1, R_ARG, 1, 0 },

    /* For allocation instructions, arg1hi indicates number of bytes */
    {".byte",  0x00, 1, I_ARG, 0, 1, NO_ARG, 0, 0 },
    {".word",  0x00, 2, I_ARG, 0, 2, NO_ARG, 0, 0 },
    {".long",  0x00, 4, I_ARG, 0, 4, NO_ARG, 0, 0 },
    {".quad",  0x00, 8, I_ARG, 0, 8, NO_ARG, 0, 0 },
    {NULL,     0   , 0, NO_ARG, 0, 0, NO_ARG, 0, 0 }
};

instr_t invalid_instr =
    {"XXX",     0   , 0, NO_ARG, 0, 0, NO_ARG, 0, 0 };

instr_ptr find_instr(char *name)
{
    int i;
    for (i = 0; instruction_set[i].name; i++)
	if (strcmp(instruction_set[i].name,name) == 0)
	    return &instruction_set[i];
    return NULL;
}

/* Return name of instruction given its encoding */
char *iname(int instr) {
    int i;
    for (i = 0; instruction_set[i].name; i++) {
	if (instr == instruction_set[i].code)
	    return instruction_set[i].name;
    }
    return "<bad>";
}


instr_ptr bad_instr()
{
    return &invalid_instr;
}


mem_t init_mem(int len)
{

    mem_t result = (mem_t) malloc(sizeof(mem_rec));
    len = ((len+BPL-1)/BPL)*BPL;
    result->len = len;
    result->contents = (byte_t *) calloc(len, 1);
    return result;
}

void clear_mem(mem_t m)
{
    memset(m->contents, 0, m->len);
}

void free_mem(mem_t m)
{
    free((void *) m->contents);
    free((void *) m);
}

mem_t copy_mem(mem_t oldm)
{
    mem_t newm = init_mem(oldm->len);
    memcpy(newm->contents, oldm->contents, oldm->len);
    return newm;
}

int hex2dig(char c)
{
    if (isdigit((int)c))
	return c - '0';
    if (isupper((int)c))
	return c - 'A' + 10;
    else
	return c - 'a' + 10;
}

#define LINELEN 4096
int load_mem(mem_t m, FILE *infile, int report_error)
{
    /* Read contents of .yo file */
    char buf[LINELEN];
    char c, ch, cl;
    int byte_cnt = 0;
    int lineno = 0;
    word_t bytepos = 0; 
    while (fgets(buf, LINELEN, infile)) {
	int cpos = 0;
	lineno++;
	/* Skip white space */
	while (isspace((int)buf[cpos]))
	    cpos++;

	if (buf[cpos] != '0' ||
	    (buf[cpos+1] != 'x' && buf[cpos+1] != 'X'))
	    continue; /* Skip this line */      
	cpos+=2;

	/* Get address */
	bytepos = 0;
	while (isxdigit((int)(c=buf[cpos]))) {
	    cpos++;
	    bytepos = bytepos*16 + hex2dig(c);
	}

	while (isspace((int)buf[cpos]))
	    cpos++;

	if (buf[cpos++] != ':') {
	    if (report_error) {
		fprintf(stderr, "Error reading file. Expected colon\n");
		fprintf(stderr, "Line %d:%s\n", lineno, buf);
		fprintf(stderr,
			"Reading '%c' at position %d\n", buf[cpos], cpos);
	    }
	    return 0;
	}

	while (isspace((int)buf[cpos]))
	    cpos++;

	/* Get code */
	while (isxdigit((int)(ch=buf[cpos++])) && 
	       isxdigit((int)(cl=buf[cpos++]))) {
	    byte_t byte = 0;
	    if (bytepos >= m->len) {
		if (report_error) {
		    fprintf(stderr,
			    "Error reading file. Invalid address. 0x%llx\n",
			    bytepos);
		    fprintf(stderr, "Line %d:%s\n", lineno, buf);
		}
		return 0;
	    }
	    byte = hex2dig(ch)*16+hex2dig(cl);
	    m->contents[bytepos++] = byte;
	    byte_cnt++;
	}
    }
    return byte_cnt;
}

#ifdef CACHE_ENABLED

static bool get_byte_val(mem_t m, word_t pos, byte_t *dest)
{
    if (pos < 0 || pos >= m->len)
	return false;
    *dest = m->contents[pos];
    return true;
}

static bool get_word_val(mem_t m, word_t pos, word_t *dest)
{
    int i;
    word_t val;
    if (pos < 0 || pos + 8 > m->len)
	return false;
    val = 0;
    for (i = 0; i < 8; i++) {
	word_t b =  m->contents[pos+i] & 0xFF;
	val = val | (b <<(8*i));
    }
    *dest = val;
    return true;
}

static bool set_word_val(mem_t m, word_t pos, word_t val)
{
    int i;
    if (pos < 0 || pos + 8 > m->len)
	return false;
    for (i = 0; i < 8; i++) {
	m->contents[pos+i] = (byte_t) val & 0xFF;
	val >>= 8;
    }
    return true;
}

bool diff_mem(mem_t oldm, mem_t newm, FILE *outfile, bool check_cache)
{
    word_t pos;
    int len = oldm->len;
    bool diff = false;
    if (newm->len < len)
	len = newm->len;

    for (pos = 0; (!diff || outfile) && pos < len; pos += 8) {
        word_t ov = 0;  word_t nv = 0;
		if(check_hit(cache, pos, false) && check_cache) {
			get_word_cache(cache, pos, &nv);
		} else {
			get_word_val(newm, pos, &nv);
		}
	get_word_val(oldm, pos, &ov);
	if (nv != ov) {
	    diff = true;
	    if (outfile)
		fprintf(outfile, "0x%.4llx:\t0x%.16llx\t0x%.16llx\n", pos, ov, nv);
	}
    }
    return diff;
}


bool get_byte_val_I(mem_t m, word_t pos, byte_t *dest)
{
    if (pos < 0 || pos >= m->len)
	return false;
    *dest = m->contents[pos];
    return true;
}

bool get_word_val_I(mem_t m, word_t pos, word_t *dest)
{
    int i;
    word_t val;
    if (pos < 0 || pos + 8 > m->len)
	return false;
    val = 0;
    for (i = 0; i < 8; i++) {
	word_t b =  m->contents[pos+i] & 0xFF;
	val = val | (b <<(8*i));
    }
    *dest = val;
    return true;
}

// Read and Write Cache blocks to memory.

static void write_block(mem_t m, word_t pos, void *block) {
    size_t B = pow(2, cache->b);
	char *block_c = (char*) block;
	for(int i = 0; i < B; i++) {
		m->contents[pos + i] = block_c[i];
	}
}

static void read_block(mem_t m, word_t pos, void *block) {
    size_t B = pow(2, cache->b);
	char *block_c = (char*) block;
	for(int i = 0; i < B; i++) {
		block_c[i] = m->contents[pos + i];
	}
}

bool inflight = false;
size_t inflight_cycles = 0;
word_t inflight_pos = 0;

// Accesses Memory. Memory has a five cycle delay unless a cache hit occurs.

static mem_status_t access_memory(mem_t m, uword_t pos, operation_t operation, size_t size) {
	
    size_t B = pow(2, cache->b);
	uword_t current_address = pos; 

    for (size_t i = 0; i < size; i++) {
        if (!check_hit(cache, current_address, operation)) {
            uword_t block_address = current_address & ~(B-1);
            if(inflight_pos != block_address || !inflight) {
                inflight_pos = block_address;
                inflight_cycles = cache->d;
                inflight = true;
            }

            inflight_cycles--;
            if(inflight_cycles > 0) {
                return IN_FLIGHT;
            }

            inflight = false;

            void *block = calloc(B, 1);
            read_block(m, block_address, block);

            evicted_line_t *evicted = handle_miss(cache, block_address, operation, block);

            if (evicted->valid && evicted->dirty) {
                write_block(m, evicted->addr & ~(B-1), evicted->data);
            }

            free(block);
            free(evicted->data);
            free(evicted);
        }
        current_address++;
    }
    
    return READY;
}

// Data Memory Functions. First checks than cache. On miss, five cycle delay is forced.

mem_status_t get_word_val_D(mem_t m, word_t pos, word_t *dest)
{
	if (pos < 0 || pos + 8 > m->len)
		return ERROR;

    mem_status_t status = access_memory(m, pos, READ, sizeof(word_t));
	if(status == READY) {
		get_word_cache(cache, pos, dest);
	}
    return status;
}

mem_status_t set_byte_val_D(mem_t m, word_t pos, byte_t val)
{
    if (pos < 0 || pos >= m->len)
		return ERROR;

    mem_status_t status = access_memory(m, pos, WRITE, sizeof(byte_t));
	if(status == READY) {
		set_byte_cache(cache, pos, val);
	}
	return status;
}

mem_status_t set_word_val_D(mem_t m, word_t pos, word_t val)
{
    if (pos < 0 || pos + 8 > m->len)
		return ERROR;

	mem_status_t status = access_memory(m, pos, WRITE, sizeof(word_t));
	if(status == READY) {
		set_word_cache(cache, pos, val);
	}
	return status;
}

mem_status_t get_byte_val_D(mem_t m, word_t pos, byte_t *dest)
{
    if (pos < 0 || pos >= m->len)
		return ERROR;

	mem_status_t status = access_memory(m, pos, READ, sizeof(byte_t));
	if(status == READY) {
		get_byte_cache(cache, pos, dest);
	}
	return status;
}
#else 
bool get_byte_val(mem_t m, word_t pos, byte_t *dest)
{
    if (pos < 0 || pos >= m->len)
	return false;
    *dest = m->contents[pos];
    return true;
}

bool get_word_val(mem_t m, word_t pos, word_t *dest)
{
    int i;
    word_t val;
    if (pos < 0 || pos + 8 > m->len)
	return false;
    val = 0;
    for (i = 0; i < 8; i++) {
	word_t b =  m->contents[pos+i] & 0xFF;
	val = val | (b <<(8*i));
    }
    *dest = val;
    return true;
}

bool set_byte_val(mem_t m, word_t pos, byte_t val)
{
    if (pos < 0 || pos >= m->len)
	return false;
    m->contents[pos] = val;
    return true;
}

bool set_word_val(mem_t m, word_t pos, word_t val)
{
    int i;
    if (pos < 0 || pos + 8 > m->len)
	return false;
    for (i = 0; i < 8; i++) {
	m->contents[pos+i] = (byte_t) val & 0xFF;
	val >>= 8;
    }
    return true;
}

bool diff_mem(mem_t oldm, mem_t newm, FILE *outfile)
{
    word_t pos;
    int len = oldm->len;
    bool diff = false;
    if (newm->len < len)
	len = newm->len;
    for (pos = 0; (!diff || outfile) && pos < len; pos += 8) {
        word_t ov = 0;  word_t nv = 0;
	get_word_val(oldm, pos, &ov);
	get_word_val(newm, pos, &nv);
	if (nv != ov) {
	    diff = true;
	    if (outfile)
		fprintf(outfile, "0x%.4llx:\t0x%.16llx\t0x%.16llx\n", pos, ov, nv);
	}
    }
    return diff;
}

#endif

void dump_memory(FILE *outfile, mem_t m, word_t pos, int len)
{
    int i, j;
    while (pos % BPL) {
	pos --;
	len ++;
    }

    len = ((len+BPL-1)/BPL)*BPL;

    if (pos+len > m->len)
	len = m->len-pos;

    for (i = 0; i < len; i+=BPL) {
	word_t val = 0;
	fprintf(outfile, "0x%.4llx:", pos+i);
	for (j = 0; j < BPL; j+= 8) {
	    get_word_val(m, pos+i+j, &val);
	    fprintf(outfile, " %.16llx", val);
	}
    }
}

mem_t init_reg()
{
    return init_mem(128);
}

void free_reg(mem_t r)
{
    free_mem(r);
}

mem_t copy_reg(mem_t oldr)
{
    return copy_mem(oldr);
}

bool diff_reg(mem_t oldr, mem_t newr, FILE *outfile)
{
    word_t pos;
    int len = oldr->len;
    bool diff = false;
    if (newr->len < len)
	len = newr->len;
    for (pos = 0; (!diff || outfile) && pos < len; pos += 8) {
        word_t ov = 0;
        word_t nv = 0;
	get_word_val(oldr, pos, &ov);
	get_word_val(newr, pos, &nv);
	if (nv != ov) {
	    diff = true;
	    if (outfile)
		fprintf(outfile, "%s:\t0x%.16llx\t0x%.16llx\n",
			reg_table[pos/8].name, ov, nv);
	}
    }
    return diff;
}


memory_restore_t *create_memory_restore(mem_t old, mem_t new)
{
    word_t pos;
    int len = old->len;
    size_t diff_count = 0;
    if (new->len < len)
	len = new->len;
    for (pos = 0;  pos < len; pos += 8) {
        word_t ov = 0;
        word_t nv = 0;
	    get_word_val(old, pos, &ov);
	    get_word_val(new, pos, &nv);
	    if (nv != ov) {
            diff_count++;
	    }
    }
    memory_restore_t *restore = malloc(sizeof(memory_restore_t));
    restore->num_values = diff_count;
    restore->positions = NULL;
    restore->values = NULL;
    if (diff_count == 0) {
        return restore;
    }

    restore->positions = calloc(diff_count, sizeof(word_t));
    restore->values = calloc(diff_count, sizeof(word_t));
    diff_count = 0;
    for (pos = 0;  pos < len; pos += 8) {
        word_t ov = 0;
        word_t nv = 0;
	    get_word_val(old, pos, &ov);
	    get_word_val(new, pos, &nv);
	    if (nv != ov) {
            restore->positions[diff_count] = pos;
            restore->values[diff_count] = ov;
            diff_count++;
	    }
    }
    return restore;
}

void apply_restore(mem_t mem, memory_restore_t *restore)
{
    for (int i = 0;  i < restore->num_values; i++) {
        set_word_val(mem, restore->positions[i], restore->values[i]);
    }
}


word_t get_reg_val(mem_t r, reg_id_t id)
{
    word_t val = 0;
    if (id >= REG_NONE)
		return 0;
    get_word_val(r,id*8, &val);
    return val;
}

void set_reg_val(mem_t r, reg_id_t id, word_t val)
{
    if (id < REG_NONE) {
		set_word_val(r,id*8,val);
    }
}
     
void dump_reg(FILE *outfile, mem_t r) {
    reg_id_t id;
    for (id = 0; reg_valid(id); id++) {
        word_t val = 0;
	    get_word_val(r, id*8, &val);
	    fprintf(outfile, "%s:  %llx\n", reg_table[id].name, val);
    }
}

void dump_reg_display(FILE *outfile, mem_t r) {
    reg_id_t id;
    for (id = 0; reg_valid(id); id++) {
        word_t val = 0;
	    get_word_val(r, id*8, &val);
	    fprintf(outfile, "%4s:  %llx ", reg_table[id].name, val);
        if ((id+1) % 4 == 0) {
            fprintf(outfile, "\n");
        }
    }
    fprintf(outfile, "\n");
}

struct {
    char symbol;
    int id;
} alu_table[A_NONE+1] = 
{
    {'+',   A_ADD},
    {'-',   A_SUB},
    {'&',   A_AND},
    {'^',   A_XOR},
    {'?',   A_NONE}
};

char op_name(alu_t op)
{
    if (op < A_NONE)
	return alu_table[op].symbol;
    else
	return alu_table[A_NONE].symbol;
}

word_t compute_alu(alu_t op, word_t argA, word_t argB)
{
    word_t val;
    switch(op) {
    case A_ADD:
	val = argA+argB;
	break;
    case A_SUB:
	val = argB-argA;
	break;
    case A_AND:
	val = argA&argB;
	break;
    case A_XOR:
	val = argA^argB;
	break;
    default:
	val = 0;
    }
    return val;
}

cc_t compute_cc(alu_t op, word_t argA, word_t argB)
{
    word_t val = compute_alu(op, argA, argB);
    bool zero = (val == 0);
    bool sign = ((word_t)val < 0);
    bool ovf;
    switch(op) {
    case A_ADD:
        ovf = (((word_t) argA < 0) == ((word_t) argB < 0)) &&
  	       (((word_t) val < 0) != ((word_t) argA < 0));
	break;
    case A_SUB:
        ovf = (((word_t) argA > 0) == ((word_t) argB < 0)) &&
	       (((word_t) val < 0) != ((word_t) argB < 0));
	break;
    case A_AND:
    case A_XOR:
	ovf = false;
	break;
    default:
	ovf = false;
    }
    return PACK_CC(zero,sign,ovf);
    
}

char *cc_names[8] = {
    "Z=0 S=0 O=0",
    "Z=0 S=0 O=1",
    "Z=0 S=1 O=0",
    "Z=0 S=1 O=1",
    "Z=1 S=0 O=0",
    "Z=1 S=0 O=1",
    "Z=1 S=1 O=0",
    "Z=1 S=1 O=1"};

char *cc_name(cc_t c)
{
    int ci = c;
    if (ci < 0 || ci > 7)
	return "???????????";
    else
	return cc_names[c];
}

/* Status types */

char *stat_names[] = { "BUB", "AOK", "HLT", "ADR", "INS", "PIP" };

char *stat_name(stat_t e)
{
    if (e < 0 || e > STAT_PIP)
	return "Invalid Status";
    return stat_names[e];
}

/**************** Implementation of ISA model ************************/

state_ptr new_state(int memlen)
{
    state_ptr result = (state_ptr) malloc(sizeof(state_rec));
    result->pc = 0;
    result->r = init_reg();
    result->m = init_mem(memlen);
    result->cc = DEFAULT_CC;
    return result;
}

void free_state(state_ptr s)
{
    free_reg(s->r);
    free_mem(s->m);
    free((void *) s);
}

state_ptr copy_state(state_ptr s) {
    state_ptr result = (state_ptr) malloc(sizeof(state_rec));
    result->pc = s->pc;
    result->r = copy_reg(s->r);
    result->m = copy_mem(s->m);
    result->cc = s->cc;
    return result;
}

/* Branch logic */
bool cond_holds(cc_t cc, cond_t bcond) {
    bool zf = GET_ZF(cc);
    bool sf = GET_SF(cc);
    bool of = GET_OF(cc);
    bool jump = false;
    
    switch(bcond) {
    case C_YES:
	jump = true;
	break;
    case C_LE:
	jump = (sf^of)|zf;
	break;
    case C_L:
	jump = sf^of;
	break;
    case C_E:
	jump = zf;
	break;
    case C_NE:
	jump = zf^1;
	break;
    case C_GE:
	jump = sf^of^1;
	break;
    case C_G:
	jump = (sf^of^1)&(zf^1);
	break;
    default:
	jump = false;
	break;
    }
    return jump;
}


/* Execute single instruction.  Return status. */
stat_t step_state(state_ptr s, FILE *error_file)
{
    word_t argA, argB;
    byte_t byte0 = 0;
    byte_t byte1 = 0;
    itype_t hi0;
    alu_t  lo0;
    reg_id_t hi1 = REG_NONE;
    reg_id_t lo1 = REG_NONE;
    bool ok1 = true;
    word_t cval = 0;
    word_t okc = true;
    word_t val, dval;
    bool need_regids;
    bool need_imm;
    word_t ftpc = s->pc;  /* Fall-through PC */
    char *t, *u, *v;

    if (!get_byte_val(s->m, ftpc, &byte0)) {
	if (error_file)
	    fprintf(error_file,
		    "PC = 0x%llx, Invalid instruction address\n", s->pc);
	return STAT_ADR;
    }
    ftpc++;

    hi0 = HI4(byte0);
    lo0 = LO4(byte0);

    need_regids =
	(hi0 == I_RRMOVQ || hi0 == I_ALU || hi0 == I_PUSHQ ||
	 hi0 == I_POPQ || hi0 == I_IRMOVQ || hi0 == I_RMMOVQ ||
	 hi0 == I_MRMOVQ || hi0 == I_LEAQ || hi0 == I_VECADD || hi0 == I_SHF);

    if (need_regids) {
	ok1 = get_byte_val(s->m, ftpc, &byte1);
	ftpc++;
	hi1 = HI4(byte1);
	lo1 = LO4(byte1);
    }

    need_imm =
	(hi0 == I_IRMOVQ || hi0 == I_RMMOVQ || hi0 == I_MRMOVQ ||
	 hi0 == I_JMP || hi0 == I_CALL  || hi0 == I_LEAQ);

    if (need_imm) {
	okc = get_word_val(s->m, ftpc, &cval);
	ftpc += 8;
    }

    switch (byte0) {
    case HPACK(I_NOP, F_NONE):
	s->pc = ftpc;
	break;
    case HPACK(I_HALT, F_NONE):
	return STAT_HLT;
	break;
    case HPACK(I_RRMOVQ, F_NONE): 
    case HPACK(I_RRMOVQ, C_LE): 
    case HPACK(I_RRMOVQ, C_L): 
    case HPACK(I_RRMOVQ, C_E): 
    case HPACK(I_RRMOVQ, C_NE): 
    case HPACK(I_RRMOVQ, C_GE): 
    case HPACK(I_RRMOVQ, C_G): 
	if (!ok1) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%llx, Invalid instruction address\n", s->pc);
	    return STAT_ADR;
	}
	if (!reg_valid(hi1)) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%llx, Invalid register ID 0x%.1x\n",
			s->pc, hi1);
	    return STAT_INS;
	}
	if (!reg_valid(lo1)) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%llx, Invalid register ID 0x%.1x\n",
			s->pc, lo1);
	    return STAT_INS;
	}
	val = get_reg_val(s->r, hi1);
	if (cond_holds(s->cc, lo0))
	  set_reg_val(s->r, lo1, val);
	s->pc = ftpc;
	break;
	case HPACK(I_IRMOVQ, F_NONE):
	if (!ok1) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%llx, Invalid instruction address\n", s->pc);
	    return STAT_ADR;
	}
	if (!okc) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%llx, Invalid instruction address",
			s->pc);
	    return STAT_INS;
	}
	if (!reg_valid(lo1)) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%llx, Invalid register ID 0x%.1x\n",
			s->pc, lo1);
	    return STAT_INS;
	}
	set_reg_val(s->r, lo1, cval);
	s->pc = ftpc;
	break;
    case HPACK(I_RMMOVQ, F_NONE):
	if (!ok1) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%llx, Invalid instruction address\n", s->pc);
	    return STAT_ADR;
	}
	if (!okc) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%llx, Invalid instruction address\n", s->pc);
	    return STAT_INS;
	}
	if (!reg_valid(hi1)) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%llx, Invalid register ID 0x%.1x\n",
			s->pc, hi1);
	    return STAT_INS;
	}
	if (reg_valid(lo1)) 
	    cval += get_reg_val(s->r, lo1);
	val = get_reg_val(s->r, hi1);
	if (!set_word_val(s->m, cval, val)) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%llx, Invalid data address 0x%llx\n",
			s->pc, cval);
	    return STAT_ADR;
	}
	s->pc = ftpc;
	break;
    case HPACK(I_MRMOVQ, F_NONE): 
	if (!ok1) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%llx, Invalid instruction address\n", s->pc);
	    return STAT_ADR;
	}
	if (!okc) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%llx, Invalid instruction addres\n", s->pc);
	    return STAT_INS;
	}
	if (!reg_valid(hi1)) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%llx, Invalid register ID 0x%.1x\n",
			s->pc, hi1);
	    return STAT_INS;
	}
	if (reg_valid(lo1)) 
	    cval += get_reg_val(s->r, lo1);
	if (!get_word_val(s->m, cval, &val))
	    return STAT_ADR;
	set_reg_val(s->r, hi1, val);
	s->pc = ftpc;
	break;
    case HPACK(I_ALU, A_ADD): 
    case HPACK(I_ALU, A_SUB): 
    case HPACK(I_ALU, A_AND): 
    case HPACK(I_ALU, A_XOR): 
	if (!ok1) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%llx, Invalid instruction address\n", s->pc);
	    return STAT_ADR;
	}
	argA = get_reg_val(s->r, hi1);
	argB = get_reg_val(s->r, lo1);
	val = compute_alu(lo0, argA, argB);
	set_reg_val(s->r, lo1, val);
	s->cc = compute_cc(lo0, argA, argB);
	s->pc = ftpc;
	break;
    case HPACK(I_JMP, C_YES): 
    case HPACK(I_JMP, C_LE): 
    case HPACK(I_JMP, C_L): 
    case HPACK(I_JMP, C_E): 
    case HPACK(I_JMP, C_NE): 
    case HPACK(I_JMP, C_GE): 
    case HPACK(I_JMP, C_G): 
	if (!ok1) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%llx, Invalid instruction address\n", s->pc);
	    return STAT_ADR;
	}
	if (!okc) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%llx, Invalid instruction address\n", s->pc);
	    return STAT_ADR;
	}
	if (cond_holds(s->cc, lo0))
	    s->pc = cval;
	else
	    s->pc = ftpc;
	break;
    case HPACK(I_CALL, F_NONE):
	if (!ok1) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%llx, Invalid instruction address\n", s->pc);
	    return STAT_ADR;
	}
	if (!okc) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%llx, Invalid instruction address\n", s->pc);
	    return STAT_ADR;
	}
	val = get_reg_val(s->r, REG_RSP) - 8;
	if (!set_word_val(s->m, val, ftpc)) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%llx, Invalid stack address 0x%llx\n", s->pc, val);
	    return STAT_ADR;
	}
    set_reg_val(s->r, REG_RSP, val);
	s->pc = cval;
	break;
    case HPACK(I_RET, F_NONE):
	/* Return Instruction.  Pop address from stack */
	dval = get_reg_val(s->r, REG_RSP);
	if (!get_word_val(s->m, dval, &val)) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%llx, Invalid stack address 0x%llx\n",
			s->pc, dval);
	    return STAT_ADR;
	}
	set_reg_val(s->r, REG_RSP, dval + 8);
	s->pc = val;
	break;
    case HPACK(I_PUSHQ, F_NONE):
	if (!ok1) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%llx, Invalid instruction address\n", s->pc);
	    return STAT_ADR;
	}
	if (!reg_valid(hi1)) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%llx, Invalid register ID 0x%.1x\n", s->pc, hi1);
	    return STAT_INS;
	}
	val = get_reg_val(s->r, hi1);
	dval = get_reg_val(s->r, REG_RSP) - 8;
	if  (!set_word_val(s->m, dval, val)) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%llx, Invalid stack address 0x%llx\n", s->pc, dval);
	    return STAT_ADR;
	}
	s->pc = ftpc;
    set_reg_val(s->r, REG_RSP, dval);
	break;
    case HPACK(I_POPQ, F_NONE):
	if (!ok1) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%llx, Invalid instruction address\n", s->pc);
	    return STAT_ADR;
	}
	if (!reg_valid(hi1)) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%llx, Invalid register ID 0x%.1x\n", s->pc, hi1);
	    return STAT_INS;
	}
	dval = get_reg_val(s->r, REG_RSP);
	if (!get_word_val(s->m, dval, &val)) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%llx, Invalid stack address 0x%llx\n",
			s->pc, dval);
	    return STAT_ADR;
	}
    set_reg_val(s->r, REG_RSP, dval+8);
	set_reg_val(s->r, hi1, val);
	s->pc = ftpc;
	break;
    case HPACK(I_LEAQ, F_NONE):
	if (!ok1) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%llx, Invalid instruction address\n", s->pc);
	    return STAT_ADR;
	}
	if (!okc) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%llx, Invalid instruction addres\n", s->pc);
	    return STAT_INS;
	}
	if (!reg_valid(hi1)) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%llx, Invalid register ID 0x%.1x\n",
			s->pc, hi1);
	    return STAT_INS;
	}
	if (reg_valid(lo1)) 
	    cval += get_reg_val(s->r, lo1);
	set_reg_val(s->r, hi1, cval);
	s->pc = ftpc;
	break;
    case HPACK(I_VECADD, F_NONE):
	if (!ok1) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%llx, Invalid instruction address\n", s->pc);
	    return STAT_ADR;
	}
	argA = get_reg_val(s->r, hi1);
	argB = get_reg_val(s->r, lo1);
	int j = 1;
    int x = 0;
    word_t out = 0;
    v = (char*)&argA;
    t = (char*)&argB;
    u = (char*)&out;
    for (size_t i = 0; i < 8; i++) {
        u[i] = v[i] + t[i];
        j &= (u[i] == 0);
        x |= ((u[i] >> 7) != 0);
    }
    set_reg_val(s->r, lo1, out);
    s->cc = PACK_CC(j, x, 0);
    s->pc = ftpc;
	break;
    case HPACK(I_SHF, S_HL):
    case HPACK(I_SHF, S_HR):
    case HPACK(I_SHF, S_AR):
	if (!ok1) {
	    if (error_file)
		fprintf(error_file,
			"PC = 0x%llx, Invalid instruction address\n", s->pc);
	    return STAT_ADR;
	}
	argA = (byte_t)get_reg_val(s->r, hi1);
	argB = get_reg_val(s->r, lo1);
    switch((shf_t)lo0) {
        case S_AR:
            out = argB >> argA;
            break;
        case S_HL:
            out = argB << argA;
            break;
        case S_HR:
            out = ((unsigned long long)argB) >> argA;
            break;
        case S_NONE:
            out = 0;
            break;
    }
    set_reg_val(s->r, lo1, out);
    s->cc = PACK_CC(!out, (out >> 63) & 0x1, 0);
    s->pc = ftpc;
	break;
    default:
	if (error_file)
	    fprintf(error_file,
		    "PC = 0x%llx, Invalid instruction %.2x\n", s->pc, byte0);
	return STAT_INS;
    }
    return STAT_AOK;
}
