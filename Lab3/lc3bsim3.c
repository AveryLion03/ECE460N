/*
    Name 1: Avery Atchley
*/

/***************************************************************/
/*                                                             */
/*   LC-3b Simulator                                           */
/*                                                             */
/*   EE 460N                                                   */
/*   The University of Texas at Austin                         */
/*                                                             */
/***************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

/***************************************************************/
/*                                                             */
/* Files:  ucode        Microprogram file                      */
/*         isaprogram   LC-3b machine language program file    */
/*                                                             */
/***************************************************************/

/***************************************************************/
/* These are the functions you'll have to write.               */
/***************************************************************/

void eval_micro_sequencer();
void cycle_memory();
void eval_bus_drivers();
void drive_bus();
void latch_datapath_values();

/***************************************************************/
/* A couple of useful definitions.                             */
/***************************************************************/
#define FALSE 0
#define TRUE  1

/***************************************************************/
/* Use this to avoid overflowing 16 bits on the bus.           */
/***************************************************************/
#define Low16bits(x) ((x) & 0xFFFF)

/***************************************************************/
/* Definition of the control store layout.                     */
/***************************************************************/
#define CONTROL_STORE_ROWS 64
#define INITIAL_STATE_NUMBER 18

/***************************************************************/
/* Definition of bit order in control store word.              */
/***************************************************************/
enum CS_BITS {                                                  
    IRD,
    COND1, COND0,
    J5, J4, J3, J2, J1, J0,
    LD_MAR,
    LD_MDR,
    LD_IR,
    LD_BEN,
    LD_REG,
    LD_CC,
    LD_PC,
    GATE_PC,
    GATE_MDR,
    GATE_ALU,
    GATE_MARMUX,
    GATE_SHF,
    PCMUX1, PCMUX0,
    DRMUX,
    SR1MUX,
    ADDR1MUX,
    ADDR2MUX1, ADDR2MUX0,
    MARMUX,
    ALUK1, ALUK0,
    MIO_EN,
    R_W,
    DATA_SIZE,
    LSHF1,
    CONTROL_STORE_BITS
} CS_BITS;

/***************************************************************/
/* Functions to get at the control bits.                       */
/***************************************************************/
int GetIRD(int *x)           { return(x[IRD]); }
int GetCOND(int *x)          { return((x[COND1] << 1) + x[COND0]); }
int GetJ(int *x)             { return((x[J5] << 5) + (x[J4] << 4) +
				      (x[J3] << 3) + (x[J2] << 2) +
				      (x[J1] << 1) + x[J0]); }
int GetLD_MAR(int *x)        { return(x[LD_MAR]); }
int GetLD_MDR(int *x)        { return(x[LD_MDR]); }
int GetLD_IR(int *x)         { return(x[LD_IR]); }
int GetLD_BEN(int *x)        { return(x[LD_BEN]); }
int GetLD_REG(int *x)        { return(x[LD_REG]); }
int GetLD_CC(int *x)         { return(x[LD_CC]); }
int GetLD_PC(int *x)         { return(x[LD_PC]); }
int GetGATE_PC(int *x)       { return(x[GATE_PC]); }
int GetGATE_MDR(int *x)      { return(x[GATE_MDR]); }
int GetGATE_ALU(int *x)      { return(x[GATE_ALU]); }
int GetGATE_MARMUX(int *x)   { return(x[GATE_MARMUX]); }
int GetGATE_SHF(int *x)      { return(x[GATE_SHF]); }
int GetPCMUX(int *x)         { return((x[PCMUX1] << 1) + x[PCMUX0]); }
int GetDRMUX(int *x)         { return(x[DRMUX]); }
int GetSR1MUX(int *x)        { return(x[SR1MUX]); }
int GetADDR1MUX(int *x)      { return(x[ADDR1MUX]); }
int GetADDR2MUX(int *x)      { return((x[ADDR2MUX1] << 1) + x[ADDR2MUX0]); }
int GetMARMUX(int *x)        { return(x[MARMUX]); }
int GetALUK(int *x)          { return((x[ALUK1] << 1) + x[ALUK0]); }
int GetMIO_EN(int *x)        { return(x[MIO_EN]); }
int GetR_W(int *x)           { return(x[R_W]); }
int GetDATA_SIZE(int *x)     { return(x[DATA_SIZE]); } 
int GetLSHF1(int *x)         { return(x[LSHF1]); }

/***************************************************************/
/* The control store rom.                                      */
/***************************************************************/
int CONTROL_STORE[CONTROL_STORE_ROWS][CONTROL_STORE_BITS];

/***************************************************************/
/* Main memory.                                                */
/***************************************************************/
/* MEMORY[A][0] stores the least significant byte of word at word address A
   MEMORY[A][1] stores the most significant byte of word at word address A 
   There are two write enable signals, one for each byte. WE0 is used for 
   the least significant byte of a word. WE1 is used for the most significant 
   byte of a word. */

#define WORDS_IN_MEM    0x08000 
#define MEM_CYCLES      5
int MEMORY[WORDS_IN_MEM][2];

/***************************************************************/

/***************************************************************/

/***************************************************************/
/* LC-3b State info.                                           */
/***************************************************************/
#define LC_3b_REGS 8

int RUN_BIT;	/* run bit */
int BUS;	/* value of the bus */

typedef struct System_Latches_Struct{

int PC,		/* program counter */
    MDR,	/* memory data register */
    MAR,	/* memory address register */
    IR,		/* instruction register */
    N,		/* n condition bit */
    Z,		/* z condition bit */
    P,		/* p condition bit */
    BEN;        /* ben register */

int READY;	/* ready bit */
  /* The ready bit is also latched as you dont want the memory system to assert it 
     at a bad point in the cycle*/

int REGS[LC_3b_REGS]; /* register file. */

int MICROINSTRUCTION[CONTROL_STORE_BITS]; /* The microintruction */

int STATE_NUMBER; /* Current State Number - Provided for debugging */ 
} System_Latches;

/* Data Structure for Latch */

System_Latches CURRENT_LATCHES, NEXT_LATCHES;

/***************************************************************/
/* A cycle counter.                                            */
/***************************************************************/
int CYCLE_COUNT;

/***************************************************************/
/*                                                             */
/* Procedure : help                                            */
/*                                                             */
/* Purpose   : Print out a list of commands.                   */
/*                                                             */
/***************************************************************/
void help() {                                                    
    printf("----------------LC-3bSIM Help-------------------------\n");
    printf("go               -  run program to completion       \n");
    printf("run n            -  execute program for n cycles    \n");
    printf("mdump low high   -  dump memory from low to high    \n");
    printf("rdump            -  dump the register & bus values  \n");
    printf("?                -  display this help menu          \n");
    printf("quit             -  exit the program                \n\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : cycle                                           */
/*                                                             */
/* Purpose   : Execute a cycle                                 */
/*                                                             */
/***************************************************************/
void cycle() {                                                

  eval_micro_sequencer();   
  cycle_memory();
  eval_bus_drivers();
  drive_bus();
  latch_datapath_values();

  CURRENT_LATCHES = NEXT_LATCHES;

  CYCLE_COUNT++;
}

/***************************************************************/
/*                                                             */
/* Procedure : run n                                           */
/*                                                             */
/* Purpose   : Simulate the LC-3b for n cycles.                 */
/*                                                             */
/***************************************************************/
void run(int num_cycles) {                                      
    int i;

    if (RUN_BIT == FALSE) {
	printf("Can't simulate, Simulator is halted\n\n");
	return;
    }

    printf("Simulating for %d cycles...\n\n", num_cycles);
    for (i = 0; i < num_cycles; i++) {
	if (CURRENT_LATCHES.PC == 0x0000) {
	    RUN_BIT = FALSE;
	    printf("Simulator halted\n\n");
	    break;
	}
	cycle();
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : go                                              */
/*                                                             */
/* Purpose   : Simulate the LC-3b until HALTed.                 */
/*                                                             */
/***************************************************************/
void go() {                                                     
    if (RUN_BIT == FALSE) {
	printf("Can't simulate, Simulator is halted\n\n");
	return;
    }

    printf("Simulating...\n\n");
    while (CURRENT_LATCHES.PC != 0x0000)
	cycle();
    RUN_BIT = FALSE;
    printf("Simulator halted\n\n");
}

/***************************************************************/ 
/*                                                             */
/* Procedure : mdump                                           */
/*                                                             */
/* Purpose   : Dump a word-aligned region of memory to the     */
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void mdump(FILE * dumpsim_file, int start, int stop) {          
    int address; /* this is a byte address */

    printf("\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
    printf("-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
	printf("  0x%.4x (%d) : 0x%.2x%.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    printf("\n");

    /* dump the memory contents into the dumpsim file */
    fprintf(dumpsim_file, "\nMemory content [0x%.4x..0x%.4x] :\n", start, stop);
    fprintf(dumpsim_file, "-------------------------------------\n");
    for (address = (start >> 1); address <= (stop >> 1); address++)
	fprintf(dumpsim_file, " 0x%.4x (%d) : 0x%.2x%.2x\n", address << 1, address << 1, MEMORY[address][1], MEMORY[address][0]);
    fprintf(dumpsim_file, "\n");
    fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : rdump                                           */
/*                                                             */
/* Purpose   : Dump current register and bus values to the     */   
/*             output file.                                    */
/*                                                             */
/***************************************************************/
void rdump(FILE * dumpsim_file) {                               
    int k; 

    printf("\nCurrent register/bus values :\n");
    printf("-------------------------------------\n");
    printf("Cycle Count  : %d\n", CYCLE_COUNT);
    printf("PC           : 0x%.4x\n", CURRENT_LATCHES.PC);
    printf("IR           : 0x%.4x\n", CURRENT_LATCHES.IR);
    printf("STATE_NUMBER : 0x%.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    printf("BUS          : 0x%.4x\n", BUS);
    printf("MDR          : 0x%.4x\n", CURRENT_LATCHES.MDR);
    printf("MAR          : 0x%.4x\n", CURRENT_LATCHES.MAR);
    printf("CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    printf("Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
	printf("%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
    printf("\n");

    /* dump the state information into the dumpsim file */
    fprintf(dumpsim_file, "\nCurrent register/bus values :\n");
    fprintf(dumpsim_file, "-------------------------------------\n");
    fprintf(dumpsim_file, "Cycle Count  : %d\n", CYCLE_COUNT);
    fprintf(dumpsim_file, "PC           : 0x%.4x\n", CURRENT_LATCHES.PC);
    fprintf(dumpsim_file, "IR           : 0x%.4x\n", CURRENT_LATCHES.IR);
    fprintf(dumpsim_file, "STATE_NUMBER : 0x%.4x\n\n", CURRENT_LATCHES.STATE_NUMBER);
    fprintf(dumpsim_file, "BUS          : 0x%.4x\n", BUS);
    fprintf(dumpsim_file, "MDR          : 0x%.4x\n", CURRENT_LATCHES.MDR);
    fprintf(dumpsim_file, "MAR          : 0x%.4x\n", CURRENT_LATCHES.MAR);
    fprintf(dumpsim_file, "CCs: N = %d  Z = %d  P = %d\n", CURRENT_LATCHES.N, CURRENT_LATCHES.Z, CURRENT_LATCHES.P);
    fprintf(dumpsim_file, "Registers:\n");
    for (k = 0; k < LC_3b_REGS; k++)
	fprintf(dumpsim_file, "%d: 0x%.4x\n", k, CURRENT_LATCHES.REGS[k]);
    fprintf(dumpsim_file, "\n");
    fflush(dumpsim_file);
}

/***************************************************************/
/*                                                             */
/* Procedure : get_command                                     */
/*                                                             */
/* Purpose   : Read a command from standard input.             */  
/*                                                             */
/***************************************************************/
void get_command(FILE * dumpsim_file) {                         
    char buffer[20];
    int start, stop, cycles;

    printf("LC-3b-SIM> ");

    scanf("%s", buffer);
    printf("\n");

    switch(buffer[0]) {
    case 'G':
    case 'g':
	go();
	break;

    case 'M':
    case 'm':
	scanf("%i %i", &start, &stop);
	mdump(dumpsim_file, start, stop);
	break;

    case '?':
	help();
	break;
    case 'Q':
    case 'q':
	printf("Bye.\n");
	exit(0);

    case 'R':
    case 'r':
	if (buffer[1] == 'd' || buffer[1] == 'D')
	    rdump(dumpsim_file);
	else {
	    scanf("%d", &cycles);
	    run(cycles);
	}
	break;

    default:
	printf("Invalid Command\n");
	break;
    }
}

/***************************************************************/
/*                                                             */
/* Procedure : init_control_store                              */
/*                                                             */
/* Purpose   : Load microprogram into control store ROM        */ 
/*                                                             */
/***************************************************************/
void init_control_store(char *ucode_filename) {                 
    FILE *ucode;
    int i, j, index;
    char line[200];

    printf("Loading Control Store from file: %s\n", ucode_filename);

    /* Open the micro-code file. */
    if ((ucode = fopen(ucode_filename, "r")) == NULL) {
	printf("Error: Can't open micro-code file %s\n", ucode_filename);
	exit(-1);
    }

    /* Read a line for each row in the control store. */
    for(i = 0; i < CONTROL_STORE_ROWS; i++) {
	if (fscanf(ucode, "%[^\n]\n", line) == EOF) {
	    printf("Error: Too few lines (%d) in micro-code file: %s\n",
		   i, ucode_filename);
	    exit(-1);
	}

	/* Put in bits one at a time. */
	index = 0;
    // printf("Control Store index %d:", i);
	for (j = 0; j < CONTROL_STORE_BITS; j++) {
	    /* Needs to find enough bits in line. */
	    if (line[index] == '\0') {
		printf("Error: Too few control bits in micro-code file: %s\nLine: %d\n",
		       ucode_filename, i);
		exit(-1);
	    }
	    if (line[index] != '0' && line[index] != '1') {
		printf("Error: Unknown value in micro-code file: %s\nLine: %d, Bit: %d\n",
		       ucode_filename, i, j);
		exit(-1);
	    }

	    /* Set the bit in the Control Store. */
	    CONTROL_STORE[i][j] = (line[index] == '0') ? 0:1;
        // printf("%d", CONTROL_STORE[i][j]);
	    index++;
	}
    printf("\n");
	/* Warn about extra bits in line. */
	if (line[index] != '\0')
	    printf("Warning: Extra bit(s) in control store file %s. Line: %d\n",
		   ucode_filename, i);
    }
    printf("\n");
}

/************************************************************/
/*                                                          */
/* Procedure : init_memory                                  */
/*                                                          */
/* Purpose   : Zero out the memory array                    */
/*                                                          */
/************************************************************/
void init_memory() {                                           
    int i;

    for (i=0; i < WORDS_IN_MEM; i++) {
	MEMORY[i][0] = 0;
	MEMORY[i][1] = 0;
    }
}

/**************************************************************/
/*                                                            */
/* Procedure : load_program                                   */
/*                                                            */
/* Purpose   : Load program and service routines into mem.    */
/*                                                            */
/**************************************************************/
void load_program(char *program_filename) {                   
    FILE * prog;
    int ii, word, program_base;

    /* Open program file. */
    prog = fopen(program_filename, "r");
    if (prog == NULL) {
	printf("Error: Can't open program file %s\n", program_filename);
	exit(-1);
    }

    /* Read in the program. */
    if (fscanf(prog, "%x\n", &word) != EOF)
	program_base = word >> 1;
    else {
	printf("Error: Program file is empty\n");
	exit(-1);
    }

    ii = 0;
    while (fscanf(prog, "%x\n", &word) != EOF) {
	/* Make sure it fits. */
	if (program_base + ii >= WORDS_IN_MEM) {
	    printf("Error: Program file %s is too long to fit in memory. %x\n",
		   program_filename, ii);
	    exit(-1);
	}

	/* Write the word to memory array. */
	MEMORY[program_base + ii][0] = word & 0x00FF;
	MEMORY[program_base + ii][1] = (word >> 8) & 0x00FF;
	ii++;
    }

    if (CURRENT_LATCHES.PC == 0) CURRENT_LATCHES.PC = (program_base << 1);

    printf("Read %d words from program into memory.\n\n", ii);
}

/***************************************************************/
/*                                                             */
/* Procedure : initialize                                      */
/*                                                             */
/* Purpose   : Load microprogram and machine language program  */ 
/*             and set up initial state of the machine.        */
/*                                                             */
/***************************************************************/
void initialize(char *ucode_filename, char *program_filename, int num_prog_files) { 
    int i;
    init_control_store(ucode_filename);

    init_memory();
    for ( i = 0; i < num_prog_files; i++ ) {
	load_program(program_filename);
	while(*program_filename++ != '\0');
    }
    CURRENT_LATCHES.Z = 1;
    CURRENT_LATCHES.STATE_NUMBER = INITIAL_STATE_NUMBER;
    memcpy(CURRENT_LATCHES.MICROINSTRUCTION, CONTROL_STORE[INITIAL_STATE_NUMBER], sizeof(int)*CONTROL_STORE_BITS);

    NEXT_LATCHES = CURRENT_LATCHES;

    RUN_BIT = TRUE;
}

/***************************************************************/
/*                                                             */
/* Procedure : main                                            */
/*                                                             */
/***************************************************************/
int main(int argc, char *argv[]) {                              
    FILE * dumpsim_file;

    /* Error Checking */
    if (argc < 3) {
	printf("Error: usage: %s <micro_code_file> <program_file_1> <program_file_2> ...\n",
	       argv[0]);
	exit(1);
    }

    printf("LC-3b Simulator\n\n");

    initialize(argv[1], argv[2], argc - 2);

    if ( (dumpsim_file = fopen( "dumpsim", "w" )) == NULL ) {
	printf("Error: Can't open dumpsim file\n");
	exit(-1);
    }

    while (1)
	get_command(dumpsim_file);

}

/***************************************************************/
/* Do not modify the above code.
   You are allowed to use the following global variables in your
   code. These are defined above.

   CONTROL_STORE
   MEMORY
   BUS

   CURRENT_LATCHES
   NEXT_LATCHES

   You may define your own local/global variables and functions.
   You may use the functions to get at the control bits defined
   above.

   Begin your code here 	  			       */
/***************************************************************/
// Count for Memory cycles
volatile int num_cycle = 1;
/*
0 = Not in Memory Cycle
1 = In Memory Cycle
*/
volatile int mem_cycle = 0; 
int16_t ALU_Val, SHF_Val, MDR_Val, PC_Val, MAR_Val, MEM_Val, BEN_Val; // Temp register values for each instruction to reference
   /* 
   * Evaluate the address of the next state according to the 
   * micro sequencer logic. Latch the next microinstruction.
   * 
****************************************************************/
void eval_micro_sequencer() {
    if(mem_cycle != 0 && (CURRENT_LATCHES.READY != 1)) return; // Don't do anything if we are in memory cycle. Fetch next instruction when Ready bit is set
    int j = GetJ(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]);
    // printf("J bits: 0x%0.4x\n", j);
    // printf("MicroInstruction: 0x%i\n", CURRENT_LATCHES.STATE_NUMBER);
    if(GetIRD(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]) == 1){
        NEXT_LATCHES.STATE_NUMBER = ((CURRENT_LATCHES.IR & 0xF000) >> 12 ) | 0x000000; // Extend to 6 bits
        // SET BEN
        NEXT_LATCHES.BEN = (((CURRENT_LATCHES.IR & 0x0800) >> 11) & CURRENT_LATCHES.N)| (((CURRENT_LATCHES.IR & 0x0400) >> 10) & CURRENT_LATCHES.Z) | (((CURRENT_LATCHES.IR & 0x0200) >> 9) & CURRENT_LATCHES.P);
     }
    else{
        int j2 = ((GetCOND(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]) == 0b10) && CURRENT_LATCHES.BEN) ? 1 : 0;
        j2 = j2 << 2;
        int j1 = ((GetCOND(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]) == 0b01) && CURRENT_LATCHES.READY) ? 1 : 0;
        j1 = j1 << 1;
        int j0 = ((GetCOND(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]) == 0b11) && (((CURRENT_LATCHES.IR & 0x0800) >> 11) == 1)) ? 1: 0; 
        j = j | j2 | j1 | j0; // update microsequencer
        NEXT_LATCHES.STATE_NUMBER = Low16bits(j);
        // printf("J Bits: %d\nJ2 bit: %d\nJ1 bit: %d\nJ0 bit: %d\n", j,j2,j1,j0);
        // printf("MicroInstruction: %d\n", CURRENT_LATCHES.STATE_NUMBER);
        // NEXT_LATCHES.BEN = (((CURRENT_LATCHES.IR & 0x0800) >> 11) & CURRENT_LATCHES.N)| (((CURRENT_LATCHES.IR & 0x0400) >> 10) & CURRENT_LATCHES.Z) | (((CURRENT_LATCHES.IR & 0x0200) >> 9) & CURRENT_LATCHES.P);
    }
}


void cycle_memory() {
    if(num_cycle == 1){
        NEXT_LATCHES.READY = 0;
    } 
    if(GetMIO_EN(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]) && ((CURRENT_LATCHES.READY != 1) || num_cycle == 5)){
        // Set mem_cycle!
        mem_cycle = 1;
        // printf("Cycle Number: %i\n", num_cycle);
        // Do stuff
        // Cycle 5 times (num Cycles)
        // Set Ready Bit
        if(num_cycle < 4){
            // printf("Current Num_Cycle: %i\n", num_cycle);
            num_cycle++;
        }
        else if(num_cycle == 4){
            num_cycle++;
            NEXT_LATCHES.READY = 1;
        }
        else{
            int mem_loc;
            num_cycle = 1;
            mem_cycle = 0; // No longer in Mem Cycle!
            NEXT_LATCHES.READY = 0;
            if(GetMIO_EN(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER])){
                // Read from Memory -> Ignore Memory Mapped Implementation
                mem_loc = CURRENT_LATCHES.MAR / 2; // Split for LSB and MSB
                // NEED TO CHECK FOR MAR[0] !!!!!
                if(GetR_W(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]) == 0){
                    // Byte
                    if(GetDATA_SIZE(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]) == 0){
                        if(CURRENT_LATCHES.MAR & 0b001 == 1){ // MSB
                            MEM_Val = (MEMORY[mem_loc][1]);
                            if(MEM_Val & 0x0080){
                                MEM_Val |= 0xFF00;
                            }
                        }
                        else{ // LSB
                            MEM_Val = (MEMORY[mem_loc][0]);
                            if(MEM_Val & 0x0080){
                                MEM_Val |= 0xFF00;
                            }
                        }
                    }
                    // Word
                    else{
                        MEM_Val = (MEMORY[mem_loc][1] << 8) | (MEMORY[mem_loc][0]); // Store into temp value until 6th cycle
                    }
                }
                // Write into Memory
                else if(GetR_W(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]) == 1){
                    // WE0 WE1, MAR[0], DATA.SIZE
                    int WE = 0b00;
                    if(CURRENT_LATCHES.MAR & 0x0001 == 1){
                        // Set WE1
                        if((GetDATA_SIZE(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]) == 0)){
                            // printf("Byte reference\n");
                            WE |= 0b10;
                        }
                    }
                    else{
                        // Set WE1
                        // printf("Data Size: %d\n", GetDATA_SIZE(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]));
                        if((GetDATA_SIZE(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]) == 1)){ // WORD chosen
                            // printf("WORD reference\n");
                            WE |= 0b10;
                        }
                        // printf("Byte reference\n");
                        // Set WE0
                        WE |= 0b01;
                    }

                    // Write into memory
                    int temp_val;
                    // Ignore, return home
                    if(WE == 0b00) return;
                    // WE0 only enabled
                    else if(WE == 0b01){
                        mem_loc = CURRENT_LATCHES.MAR / 2;
                        MEMORY[mem_loc][0] = CURRENT_LATCHES.MDR & 0x00FF;
                    }
                    else if(WE == 0b10){
                        mem_loc = CURRENT_LATCHES.MAR / 2;
                        MEMORY[mem_loc][1] = CURRENT_LATCHES.MDR & 0x00FF;
                    }
                    else if(WE == 0b11){
                        mem_loc = CURRENT_LATCHES.MAR / 2;
                        MEMORY[mem_loc][0] = CURRENT_LATCHES.MDR & 0x00FF;
                        MEMORY[mem_loc][1] = ((CURRENT_LATCHES.MDR & 0xFF00) >> 8) & 0x00FF;
                    }
                }
            }
            else{
                return;
            }

            // NEXT_LATCHES.MDR = MEMORY[] 
        }
    }
    else{
        num_cycle = 1; // Reset memory cycle
        NEXT_LATCHES.READY = 0; // Reset Ready Bit
        mem_cycle = 0;
    }
  /* 
   * This function emulates memory and the WE logic. 
   * Keep track of which cycle of MEMEN we are dealing with.  
   * If fourth, we need to latch Ready bit at the end of 
   * cycle to prepare microsequencer for the fifth cycle.  
   */
}



void eval_bus_drivers() {
    if(mem_cycle != 0) return; // Don't do anything if we are in a memory cycle (i.e., 1-5)
    // printf("On Eval Bus Drivers\n");
  /* 
   * Datapath routine emulating operations before driving the bus.
   * Evaluate the input of tristate drivers 
   *         Gate_MARMUX,
   *		 Gate_PC,
   *		 Gate_ALU,
   *		 Gate_SHF,
   *		 Gate_MDR.
   */    

/****************ALU****************/
    if(GetGATE_ALU((CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]))){
        // Read the ALUK and carry out the ALU 
        int16_t sr1 = 0, sr2 = 0;
        // SR1
            if(GetSR1MUX(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]) == 0){
                sr1 = (CURRENT_LATCHES.IR & 0x0E00) >> 9;
                sr1 = (CURRENT_LATCHES.REGS[sr1]);

            }
            else if(GetSR1MUX(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]) == 1){
                sr1 = (CURRENT_LATCHES.IR & 0x01C0) >> 6;
                sr1 = (CURRENT_LATCHES.REGS[sr1]);
            }
        //ADD   
        if(GetALUK(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]) == 0b00){
            // SR2
            if(CURRENT_LATCHES.IR & 0x0020){
                sr2 = CURRENT_LATCHES.IR & 0x001F;
                if(sr2 & 0x0010){
                    sr2 |= 0xFFE0; // Sign Extend
                }
            }
            else{
                sr2 = CURRENT_LATCHES.IR & 0x0007;
                sr2 = CURRENT_LATCHES.REGS[sr2];
            }
            ALU_Val = sr1 + sr2;
            ALU_Val = Low16bits(ALU_Val);
        }
        // AND
        else if(GetALUK(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]) == 0b01){
            // SR2
            if(CURRENT_LATCHES.IR & 0x0020){
                sr2 = CURRENT_LATCHES.IR & 0x001F;
                if(sr2 & 0x0010){
                    sr2 |= 0xFFE0; // Sign Extend
                }
            }
            else{
                sr2 = CURRENT_LATCHES.IR & 0x0007;
                sr2 = CURRENT_LATCHES.REGS[sr2];
            }
            ALU_Val = sr1 & sr2;
            ALU_Val = Low16bits(ALU_Val);
        }
        // PASS
        else if(GetALUK(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]) == 0b11){
            ALU_Val = sr1;
            ALU_Val = Low16bits(ALU_Val);
        }
        // XOR
        else if(GetALUK(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]) == 0b10){
            // SR2
            if(CURRENT_LATCHES.IR & 0x0020){
                sr2 = CURRENT_LATCHES.IR & 0x001F;
                if(sr2 & 0x0010){
                    sr2 |= 0xFFE0; // Sign Extend
                }
            }
            else{
                sr2 = CURRENT_LATCHES.IR & 0x0007;
                sr2 = CURRENT_LATCHES.REGS[sr2];
            }
            ALU_Val = sr1 ^ sr2;    
            ALU_Val = Low16bits(ALU_Val);    
        }
        else if(GetALUK(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]) == 0b11){
            ALU_Val = sr1;
            ALU_Val = Low16bits(ALU_Val);
        }
        ALU_Val = Low16bits(ALU_Val);
    }
/****************PC****************/
    // Do Nothing?
    if(GetGATE_ALU((CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]))){
        PC_Val = Low16bits(CURRENT_LATCHES.PC);
    }
/****************SHF****************/
    // Takes SR1 and shifts it by 
    if(GetGATE_SHF(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER])){
        int16_t sr1 = (CURRENT_LATCHES.IR & 0x01C0) >> 6;
        sr1 = CURRENT_LATCHES.REGS[sr1];
        int ammt4 = CURRENT_LATCHES.IR & 0x000F;
        int direction = (CURRENT_LATCHES.IR & 0x0010) >> 4;
        if(direction == 0){
            sr1 = sr1 << ammt4;
        }
        else if(direction == 1){
            int a = (CURRENT_LATCHES.IR & 0x0020) >> 5;
            if(a == 0){
                sr1 = (uint16_t)sr1 >> ammt4; // Logical right shift
            }
            else{
                int sign_bit = sr1 & 0x8000; // Extract the sign bit
                for(int i = 0; i < ammt4; i++){
                    sr1 = (sr1 >> 1) | sign_bit; // Arithmetic right shift
                }
            }
        }
        SHF_Val = Low16bits(sr1);
    }


/****************MDR****************/
    if(GetGATE_MDR(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER])){
        // Logic - if Data_Size = 1 pass word (i.e., all of MDR)
        //         if Data_Size = 0, pass byte dependent on MAR[0]
        if(GetDATA_SIZE(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]) == 0){
            if(CURRENT_LATCHES.MAR & 0x0001){
                MDR_Val = CURRENT_LATCHES.MDR & 0xFF00 >> 8;
                if(MDR_Val & 0x0080){
                    MDR_Val |= 0xFF00;
                }
            }
            else{
                MDR_Val = CURRENT_LATCHES.MDR & 0x00FF;
                if(MDR_Val & 0x0080){
                    MDR_Val |= 0xFF00;
                }
            }
        }
        else{
            MDR_Val = CURRENT_LATCHES.MDR;
        }
        MDR_Val = Low16bits(MDR_Val);
    }

/****************MAR****************/
    if(GetGATE_MARMUX(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER])){
        // Read MAR Mux
        if(GetMARMUX(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]) == 0){
            MAR_Val = (CURRENT_LATCHES.IR & 0x00FF) << 1;
        }
        else if(GetMARMUX(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]) == 1){
            int16_t val_addr1, val_addr2;

            // ADDR1 MUX
            if(GetADDR1MUX(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]) == 0){
                if(GetSR1MUX(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]) == 0){
                    val_addr1 = (CURRENT_LATCHES.IR & 0x0E00) >> 9;
                    // printf("Register R%i\n", val_addr1);
                    val_addr1 = CURRENT_LATCHES.REGS[val_addr1];
                }
                else if(GetSR1MUX(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]) == 1){
                    val_addr1 = (CURRENT_LATCHES.IR & 0x01C0) >> 6;
                    // printf("Register R%i\n", val_addr1);
                    val_addr1 = CURRENT_LATCHES.REGS[val_addr1];
                }
            }
            else if(GetADDR1MUX(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]) == 1){
                val_addr1 = CURRENT_LATCHES.PC;
            }
            
            // ADDR2 MUX
            val_addr2 = GetADDR2MUX(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]);
            if(GetADDR2MUX(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]) == 0){
                val_addr2 = (CURRENT_LATCHES.IR & 0x07FF);
                if(val_addr2 & 0x0700){
                    val_addr2 |= 0xF800;
                }
            }
            else if(GetADDR2MUX(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]) == 1){
                val_addr2 = (CURRENT_LATCHES.IR & 0x01FF);
                if(val_addr2 & 0x0100){
                    val_addr2 |= 0xFE00;
                }
            }
            else if(GetADDR2MUX(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]) == 2){
                val_addr2 = (CURRENT_LATCHES.IR & 0x003F);  
                if(val_addr2 & 0x0020){
                    val_addr2 |= 0xFFC0;
                } 
            }
            else if(GetADDR2MUX(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]) == 3){
                val_addr2 = 0x0000;
            }
            if(GetLSHF1(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]) == 1){
                val_addr2 = val_addr2 << 1;
            }
            val_addr2 = Low16bits(val_addr2);
            // printf("Addr1 Mux: 0x%0.4x and Addr2 Mux: 0x%0.4x\n", val_addr1, val_addr2);
            MAR_Val = val_addr1 + val_addr2;
            // printf("MAR Val: 0x%0.4x\n", MAR_Val);
        }
        MAR_Val = Low16bits(MAR_Val);
    }
}


void drive_bus() {
    if(mem_cycle != 0) {
        BUS = 0x0000;
        return; // Don't do anything if we are in a memory cycle
    }
    
    if(GetGATE_ALU(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER])){
        BUS = Low16bits(ALU_Val);
    }
    else if(GetGATE_MARMUX(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER])){
        BUS = Low16bits(MAR_Val);
    }
    else if(GetGATE_PC(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER])){
        BUS = Low16bits(CURRENT_LATCHES.PC);
    } 
    else if(GetGATE_SHF(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER])){
        BUS = Low16bits(SHF_Val);
    } 
    else if(GetGATE_MDR(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER])){
        if(GetDATA_SIZE(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]) == 0){
            if(CURRENT_LATCHES.MAR & 0x0001){
                MDR_Val = CURRENT_LATCHES.MDR & 0xFF00 >> 8;
                if(MDR_Val & 0x0080){
                    MDR_Val |= 0xFF00;
                }
            }
            else{
                MDR_Val = CURRENT_LATCHES.MDR & 0x00FF;
                if(MDR_Val & 0x0080){
                    MDR_Val |= 0xFF00;
                }
            }
        }
        else{
            MDR_Val = CURRENT_LATCHES.MDR;
        }
        BUS = Low16bits(MDR_Val);
    }
    else BUS = 0x0000;
    /*
   * Datapath routine for driving the bus from one of the 5 possible 
   * tristate drivers. 
   */
    // printf("Current BUS: 0x%0.4x\n", BUS);
}


void latch_datapath_values() {
    if(mem_cycle != 0) return; // Don't do anything if we are in a memory cycle

  /* 
   * Datapath routine for computing all functions that need to latch
   * values in the data path at the end of this cycle.  Some values
   * require sourcing the bus; therefore, this routine has to come 
   * after drive_bus.
   */       
    /*
    * LD.PC -
    * LD.REG - 
    * LD.MDR -
    * LD.MAR -
    * LD.CC
    * LD.IR  -
    */
    // LD_CC
    if(GetLD_CC(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER])){
        // printf("Set CC: %i%i%i\n", NEXT_LATCHES.N, NEXT_LATCHES.Z, NEXT_LATCHES.P);
        if(BUS == 0){
            NEXT_LATCHES.Z = 1;
            NEXT_LATCHES.N = 0;
            NEXT_LATCHES.P = 0;
        }
        else if(BUS & 0x8000){
            NEXT_LATCHES.Z = 0;
            NEXT_LATCHES.N = 1;
            NEXT_LATCHES.P = 0;
        }
        else if(BUS > 0){
            NEXT_LATCHES.Z = 0;
            NEXT_LATCHES.N = 0;
            NEXT_LATCHES.P = 1;
        }

    }
    //LD_IR
    if(GetLD_IR(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER])){
            NEXT_LATCHES.IR = Low16bits(BUS);
    }

    //LD_REG
    if(GetLD_REG(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER])){
        
        if(GetDRMUX(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]) == 1){
            NEXT_LATCHES.REGS[7] = Low16bits(BUS);
            // printf("Loading Register R7 with %i\n", BUS);
        }
        else{
            int dr = (CURRENT_LATCHES.IR & 0x0E00) >> 9;
            NEXT_LATCHES.REGS[dr] = Low16bits(BUS);
            // printf("Loading Register R%i with %i\n", dr, BUS);
        }
    }
    //LD_PC
    if(GetLD_PC(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER])){
        if(GetPCMUX(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]) == 0){
            NEXT_LATCHES.PC = BUS;
        }
        else if(GetPCMUX(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]) == 1){
            int16_t val_addr1, val_addr2;
            val_addr2 = GetADDR2MUX(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]);
            if(val_addr2 == 0){
                val_addr2 = (CURRENT_LATCHES.IR & 0x07FF);
                if(val_addr2 & 0x0400){
                    val_addr2 |= 0xF800;
                }
            }
            else if(val_addr2 == 1){
                val_addr2 = (CURRENT_LATCHES.IR & 0x01FF);
                if(val_addr2 & 0x0100){
                    val_addr2 |= 0xFE00;
                }
            }
            else if(val_addr2 == 2){
                val_addr2 = (CURRENT_LATCHES.IR & 0x003F);
                if(val_addr2 & 0x0020){
                    val_addr2 |= 0xFFC0;
                }
            }
            else if(val_addr2 == 3){
                val_addr2 = 0x0000;
            }
            // Check if LSHF1 is enabled
            if(GetLSHF1(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]) == 1){
                val_addr2 = val_addr2 << 1;
            }

            val_addr2 = Low16bits(val_addr2);
            val_addr1 = GetADDR1MUX(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]);
            if(val_addr1 == 0){
                if(GetSR1MUX(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]) == 0){
                    val_addr1 = (CURRENT_LATCHES.IR & 0x0E00) >> 9;
                    val_addr1 = CURRENT_LATCHES.REGS[val_addr1];
                }
                else if(GetSR1MUX(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]) == 1){
                    val_addr1 = (CURRENT_LATCHES.IR & 0x01C0) >> 6;
                    val_addr1 = CURRENT_LATCHES.REGS[val_addr1];
                }
            }
            else if(val_addr1 == 1){
                val_addr1 = CURRENT_LATCHES.PC;
            }
            val_addr1 = Low16bits(val_addr1);
            NEXT_LATCHES.PC = Low16bits(val_addr1 + val_addr2);
        }
        if(GetPCMUX(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]) == 2){
            NEXT_LATCHES.PC = Low16bits(CURRENT_LATCHES.PC + 2);
        }        
    }

    // LD.MAR
    if(GetLD_MAR(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER])){
        NEXT_LATCHES.MAR = Low16bits(BUS);
    }

    // LD.MDR
    if(GetLD_MDR(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER])){
        if(GetMIO_EN(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]) == 0){
            if(GetDATA_SIZE(CONTROL_STORE[CURRENT_LATCHES.STATE_NUMBER]) == 0){
                MDR_Val = 0x00FF & BUS;
                if(MDR_Val & 0x0080){
                    MDR_Val |= 0xFF00;
                }
            }
            else{
                MDR_Val = BUS;
            }
        }
        else{
            MDR_Val = MEM_Val;
        }
        NEXT_LATCHES.MDR = Low16bits(MDR_Val);
    }
    BUS = 0x0000;
    
}
