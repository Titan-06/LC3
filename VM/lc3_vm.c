#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/time.h>
#include <signal.h>
#include <sys/stat.h>

#define MEMORY_MAX ( 1 << 16 )
#define START_ADDRESS 0x3000 // Lower and upper bound given to programs
#define END_ADDRESS 0xFDFF

__uint16_t memory[MEMORY_MAX];
__uint16_t registers[8]; // The 8 general purpose registers

__uint16_t PC; // Program Counter Register ( Contains Which Memory address to fetch next )
__uint16_t PSR; // Processor status register

struct termios oldt,newt; // To handle terminal settings

enum GENERAL_REGISTERS{
    R0 = 0,
    R1,
    R2,
    R3,
    R4,
    R5,
    R6,
    R7,
};

enum OPCODES{ // 15 OPCODES in LC 3 and One reserved code
    OP_BR = 0,
    OP_ADD,
    OP_LD,
    OP_ST,
    OP_JSR,
    OP_AND,
    OP_LDR,
    OP_STR,
    OP_RTI,
    OP_NOT,
    OP_LDI,
    OP_STI,
    OP_RET,
    OP_RESERVED,
    OP_LEA,
    OP_TRAP
};

enum TRAPS{
    TRAP_GETC = 0x20,
    TRAP_OUT,
    TRAP_PUTS,
    TRAP_IN,
    TRAP_PUTSP,
    TRAP_HALT
};

enum SPECIAL_MEMORY_REGISTERS{ //These registers are klocated in the memory and are not supposed to be accesible 
    KBSR = 0xFE00, //To see status if key pressed
    KBDR = 0xFE02 // To access value if key pressed
};

// SYS CALL and file handling fucntion
__uint8_t check_key()
{
    // Creates a file descriptor set and adds keyboard(input) in it , clearing everything else
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO,&readfds);

    // Creating a timer and set it value to zero
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    //Select returns immendiately after checking if there is any unread data in STDIN file
    return select(1,&readfds,NULL,NULL,&timeout);
}
int get_file_size(char * file)
{
    struct stat file_stat;
    if(stat(file,&file_stat) == 0) return file_stat.st_size;
    return 0;
}


/* Accessing memory */
__uint16_t mem_write(__uint16_t address, __uint16_t value)
{
    if(address == KBSR || address == KBDR) // Can not be written to
    {
        return 0;
    }
    memory[address] = value;
    return 1;
}
__uint16_t mem_read(__uint16_t address)
{
    if(address == KBSR)
    {
        if(check_key()) // If a key was pressed
        {
            memory[KBSR] = (1 << 15);
            memory[KBDR] = getchar();
        }else{
            memory[KBSR] = 0;
        }
    }
    return memory[address];
}

// Op code helper functions
__uint16_t swap16(__uint16_t num)
{
    __uint16_t first = num << 8;
    num = num >> 8;
    return first | num;
}
__uint16_t sext(__uint16_t num,__uint8_t sig_dig)
{
    __uint16_t mask = (0xFFFF << sig_dig);
    if( !((1 << (sig_dig -1)) & num) ) mask = 0;
    return num | mask;
}
__uint16_t setcc(int16_t number)
{
    if(number > 0){
        PSR = 0b1;
    }else if(number < 0){
        PSR = 0b100;
    }else{
        PSR = 0b10; 
    }
}

// Op code instruction 
void op_add(__uint16_t instr)
{
    __uint8_t dr = (instr & 0b111000000000) >> 9; // Get the DR out
    __uint8_t sr1 = (instr & 0b111000000) >> 6; // Get the first source register
    if(instr & 0b100000)
    {
        u_int8_t imm5 = instr & 0b11111;
        registers[dr] = registers[sr1] + sext(imm5,5);
    }else{
        __uint8_t sr2 = instr & 0b111;
        registers[dr] = registers[sr1] + registers[sr2];
    }
    setcc(registers[dr]);
}

void op_and(__uint16_t instr)
{
    __uint8_t dr = (instr & 0b111000000000) >> 9; // Get the DR out
    __uint8_t sr1 = (instr & 0b111000000) >> 6; // Get the first source register
    if(instr & 0b100000)
    {
        __uint8_t imm5 = instr & 0b11111;
        registers[dr] = registers[sr1] & sext(imm5,5);
    }else{
        __uint8_t sr2 = instr & 0b111;
        registers[dr] = registers[sr1] & registers[sr2];
    }
    setcc(registers[dr]);
}

void op_br(__uint16_t instr)
{
    __uint8_t conditions = instr >> 9;
    __uint16_t offset = instr & 0b111111111;
    if(!conditions){
        PC = PC + sext(offset,9);
        return;
    }
    if(conditions & PSR){
        PC = PC + sext(offset,9);
    }
}

void op_jmp_ret(__uint16_t instr)
{
    __uint8_t baser = (instr & 0b111000000) >> 6;
    PC = registers[baser];
}

void op_jsr(__uint16_t instr)
{
    registers[R7] = PC;
    if(instr & 0b100000000000){
        __uint16_t offset = instr & 0b11111111111;
        PC = PC + sext(offset,11);
    }else{
        __uint8_t baser = (instr & 0b111000000) >> 6;
        PC = registers[baser];
    }
}

void op_ld(__uint16_t instr)
{
    __uint8_t dr = (instr & 0b111000000000) >> 9;
    __uint16_t offset = instr & 0b111111111;

    registers[dr] = mem_read(PC + sext(offset,9));
    setcc(registers[dr]);
}

void op_ldi(__uint16_t instr)
{
    __uint8_t dr = (instr & 0b111000000000) >> 9;
    __uint16_t offset = instr & 0b111111111;

    __uint16_t target = mem_read(PC + sext(offset,9));
    registers[dr] = mem_read(target);

    setcc(registers[dr]);
}

void op_ldr(__uint16_t instr)
{
    __uint8_t dr = (instr & 0b111000000000) >> 9;
    __uint8_t sr = (instr & 0b111000000) >> 6;
    __uint8_t offset = (instr & 0b111111);

    registers[dr] =mem_read(registers[sr] + sext(offset,6));
    setcc(registers[dr]);
}

void op_lea(__uint16_t instr)
{
    __uint8_t dr = (instr & 0b111000000000) >> 9;
    __uint16_t offset = instr & 0b111111111;

    registers[dr] = PC + sext(offset,9);
}

void op_not(__uint16_t instr)
{
    __uint8_t dr = (instr & 0b111000000000) >> 9;
    __uint8_t sr = (instr & 0b111000000) >> 6;

    registers[dr] = ~registers[sr];
    setcc(registers[dr]);
}

void op_st(__uint16_t instr)
{
    __uint8_t sr = (instr & 0b111000000000) >> 9;
    __uint16_t offset = instr & 0b111111111;

    mem_write(PC + sext(offset,9),registers[sr]);
}

void op_sti(__uint16_t instr)
{
    __uint8_t sr = (instr & 0b111000000000) >> 9;
    __uint16_t offset = instr & 0b111111111;

    __uint16_t target = mem_read(PC + sext(offset,9));

    mem_write(target,registers[sr]);
}

void op_str(__uint16_t instr)
{
    __uint8_t sr = (instr & 0b111000000000) >> 9;
    __uint8_t baser = (instr & 0b111000000) >> 6;
    __uint8_t offset = (instr & 0b111111);

    mem_write(registers[baser] + sext(offset,6),registers[sr]);
}

// Handling Trap routines

void trap_getc()
{
    __uint8_t character = getc(stdin);
    registers[R0] = 0 | character;
    setcc(registers[R0]);
}

void trap_out()
{
    __uint16_t character = registers[R0];

    putc(character,stdout);

    fflush(stdout);
}

void trap_puts()
{
    __uint16_t c = registers[R0];

    while(mem_read(c))
    {
        putc((char)memory[c],stdout);
        c++;
    }
    fflush(stdout);
}

void trap_in()
{
    printf("Enter a character: ");
    __uint8_t character = getc(stdin);

    registers[R0] = 0 | character;
    putc(character,stdout);

    setcc(registers[R0]);
    fflush(stdout);
}

void trap_putsp()
{
    __uint16_t c = registers[R0];

    __uint8_t till_8c; // The 0-7 bit of the memory location
    __uint8_t till_16c; // The 8-15 bit of the memory location
    while(1)
    {
        till_8c = mem_read(c) & 0b11111111;
        if(!till_8c) break;
        putc(till_8c,stdout);

        till_16c = mem_read(c) >> 8;
        if(!till_16c) break;
        putc(till_16c,stdout);

        c++;
    }
    fflush(stdout);
}

__uint8_t op_trap(__uint16_t instr){
    
    registers[R7] = PC;
    __uint16_t trap_code = (instr & 0b11111111);
    // Handle every trap code
    switch (trap_code)
    {
    case TRAP_GETC:
        trap_getc();
        break;
    case TRAP_OUT:
        trap_out();
        break;
    case TRAP_PUTS:
        trap_puts();
        break;
    case TRAP_IN:
        trap_in();
        break;
    case TRAP_PUTSP:
        trap_putsp();
        break;
    case TRAP_HALT:
        puts("HALT");
        return 1;
    }
    return 0;
}

/* SETUP and LOAD Functions */
int load_program(int argc, char * argv[]) // Loads the program returns the start address
{
    if(argc != 2)
    {
        printf("Usage: ./main [PATH OF PROGRAM]\n");
        return 0;
    }

    FILE *fptr;
    fptr = fopen(argv[1],"rb");
    // Check if file exits
    if(fptr == NULL){
        printf("No such file found.\n");
        return 0;
    }
    // Check if its empty
    long size = get_file_size(argv[1]);
    if(size == 0)
    {
        printf("FILE is empty.");
        return 0 ;
    }

    //Read first instr which is the location to store thr file
    __uint16_t temp;
    fread(&temp,sizeof(__uint16_t),1,fptr);
    __uint16_t start_address = swap16(temp);
    __uint16_t address = start_address;
    if(address < START_ADDRESS || address > END_ADDRESS)
    {
        printf("Invalid start address.\n");
        return 0;
    }
    if(END_ADDRESS - address < size){
        printf("Program too big. (Size:%d ,Size Allowed: %d)\n",size,END_ADDRESS - START_ADDRESS);
        return 0;
    }

    // Shifting to acheive big endian 
    while(fread(&temp,sizeof(__uint16_t),1,fptr))
    {
        memory[address++] = swap16(temp);
    }

    fclose(fptr);

    return start_address;
}
void setup_noncanon_mode()
{
    // SETUP -- to get input directly and not make it echo in terminal
    
    tcgetattr(STDIN_FILENO,&oldt);
    newt = oldt;

    newt.c_lflag &= ~(ICANON | ECHO);

    tcsetattr(STDIN_FILENO,TCSANOW,&newt);

}
void disable_noncanon_mode()
{
    tcsetattr(STDIN_FILENO,TCSANOW,&oldt);
}
void handle_termination(int signal)
{
    disable_noncanon_mode();
    printf("\n");
    exit(signal);
}


// MAIN
int main(int argc, char * argv[])
{

    signal(SIGINT,handle_termination); // If ctrl c is pressed , restore old terminal

    setup_noncanon_mode();

    __uint16_t running = 1;

    // Program is LOADED
    u_int16_t start = load_program(argc,argv);
    if(start == 0) running = 0;

    PSR = 1 << 1; // Initially set to zero condition code and supervisor mode 
    PC = start; // Starting position of memory for the user programs

    while(running)
    {
        __uint16_t instr = memory[PC];
        __uint8_t opcode = instr >> 12;

        PC++; // Move on to the next PC

        switch (opcode)
        {
            case OP_BR:
                op_br(instr);
                break;
            case OP_ADD:
                op_add(instr);
                break;
            case OP_LD:
                op_ld(instr);
                break;
            case OP_ST:
                op_st(instr);
                break;
            case OP_JSR:
                op_jsr(instr);
                break;
            case OP_AND:
                op_and(instr);
                break;
            case OP_LDR:
                op_ldr(instr);
                break;
            case OP_STR:
                op_str(instr);
                break;
            case OP_RTI:
                running = 0;
                printf("BAD OPCODE\n");
                break;
            case OP_NOT:
                op_not(instr);
                break;
            case OP_LDI:
                op_ldi(instr);
                break;
            case OP_STI:
                op_sti(instr);
                break;
            case OP_RET:
                op_jmp_ret(instr);
                break;
            case OP_LEA:
                op_lea(instr);
                break;
            case OP_TRAP:
                __uint8_t result = op_trap(instr);
                if(result) running = 0;
                break;

            default:
                running = 0;
                printf("BAD OPCODE\n");
                break;
        }
    }

    // Restore the older settings
    disable_noncanon_mode();

    return  0;
}