#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>

#define MEM_CAP 256
#define REGISTER_COUNT 9

#define PC 0
#define A 1
#define B 2
#define IR 3
#define MAR 4
#define FL 5
#define OP 6
#define CS 7
#define DS 8

void disp(void);
void cpu_run(void);
void err(char *);
void output();

int16_t mem[MEM_CAP];
int16_t reg[REGISTER_COUNT];
bool halt = false;
long cycle;
enum instruc {NOP, LDA, ADD, SUB, STA, LDI, JMP, JC, JZ, LDS, LDC, OUT, HLT};

char reg_str_repr[REGISTER_COUNT][4] = {"PC", "A", "B", "IR", "MAR", "FL", "OP", "CS", "DS"};

int main(void) {
    /* mem[0] = 0b01010100;
    mem[1] = 0b00101111;
    mem[2] = 0b00111110;
    mem[3] = 0b11100000;
    mem[4] = 0b11110000;
    mem[14] = 0b00010000;
    mem[15] = 0b00001000; */

    /* mem[0] = 0b01011111;
    mem[1] = 0b00111111;
    mem[2] = 0b10001010;
    mem[3] = 0b01010001;
    mem[4] = 0b11100000;
    mem[5] = 0b01101010;
    mem[10] = 0b11110000;
    mem[15] = 0b00100000; */

    /* mem[0] = 0b00011010;
    mem[1] = 0b00101011;
    mem[2] = 0b11100000;
    mem[3] = 0b11110000;
    mem[10] = 0b10000000;
    mem[11] = 0b10000000; */

    mem[0] = 0b00011011;
    mem[1] = 0b11100000;
    mem[2] = 0b00011100;
    mem[3] = 0b11100000;
    mem[4] = 0b00011101;
    mem[5] = 0b11100000;
    mem[6] = 0b00011110;
    mem[7] = 0b11100000;
    mem[8] = 0b00011111;
    mem[9] = 0b11100000;
    mem[10] = 0b11110000;
    mem[11] = 'H';
    mem[12] = 'e';
    mem[13] = 'l';
    mem[14] = 'l';
    mem[15] = 'o';

    /* mem[0] = 0b10010010;
    mem[1] = 0b00010000;
    mem[2] = 0b00100001;
    mem[3] = 0b01000010;
    mem[4] = 0b11100000;
    mem[5] = 0b11110000;
    mem[32] = 20;
    mem[33] = 30; */

    cpu_run();
    //disp();
}

void fetch_instruction () {
    reg[MAR] = reg[PC];
    cycle++;
    reg[IR] = mem[reg[MAR]];
    reg[PC]++; 
    cycle++;
}

int decode_instruction () {
    uint8_t instruction = reg[IR] & 0b11110000;
    
    switch (instruction) {
    case 0b00000000: return NOP;
    case 0b00010000: return LDA;
    case 0b00100000: return ADD;
    case 0b00110000: return SUB;
    case 0b01000000: return STA;
    case 0b01010000: return LDI;
    case 0b01100000: return JMP;
    case 0b01110000: return JC;
    case 0b10000000: return JZ;
    case 0b10010000: return LDS;
    case 0b10100000: return LDC;
    case 0b11100000: return OUT;
    case 0b11110000: return HLT;
    default:
        err("No such instruction!");
        break;
    }
}

void run_instruction( int ins ) {
    int content = reg[IR] & 0b00001111;
    switch (ins)
    {
    case NOP:
        cycle++;
        break;
    case LDA:
        reg[MAR] = reg[DS] | content;
        cycle++;
        reg[A] = mem[reg[MAR]];
        cycle++;
        break;
    case ADD:
        reg[MAR] = reg[DS] | content;
        cycle++;
        reg[B] = mem[reg[MAR]];
        reg[A] = reg[A] + reg[B];
        if(reg[A] > 255) {
            reg[FL] = reg[FL] | 0b00000001;
            reg[A] = 0;
        }
        cycle++;
        break;
    case SUB:
        reg[MAR] = reg[DS] | content;
        cycle++;
        reg[B] = mem[reg[MAR]];
        reg[A] = reg[A] - reg[B];
        if(reg[A] == 0) reg[FL] = reg[FL] | 0b00000010;
        cycle++;
        break;
    case STA:
        reg[MAR] = reg[DS] | content;
        cycle++;
        mem[reg[MAR]] = reg[A];
        cycle++;
        break;
    case LDI:
        reg[A] = content;
        cycle++;
        break;
    case JMP:
        reg[PC] = reg[CS] | content;
        cycle++;
        break;
    case JC:
        if(reg[FL] & 0b00000001) {
            reg[FL] = reg[FL] & 0b11111110;
            reg[PC] = reg[CS] | content;
            cycle++;
        }
        break;
    case JZ:
        if(reg[FL] & 0b00000010) {
            reg[FL] = reg[FL] & 0b11111101;
            reg[PC] = reg[CS] | content;
            cycle++;
        }
        break;
    case LDS:
        reg[DS] = content << 4;
        cycle++;
        break;
    case LDC:
        reg[CS] = content << 4;
        cycle++;
        break;
    case OUT:
        reg[OP] = reg[A];
        output();
        cycle++;
        break;
    case HLT:
        halt = true;
        cycle++;
        break;
    default:
        break;
    }
}

void cpu_run (void) {
    cycle = 0;
    int ins;
    while (!halt && (reg[PC]) < MEM_CAP) {
        fetch_instruction();
        ins = decode_instruction();
        run_instruction( ins );
    }
}

void output() {
    if (isalpha(reg[OP])) {
        printf( "%c", reg[OP] );    
    }
    else printf( "\n ---> Output: %d <--- \n", reg[OP] );
}

void disp () {
    for ( int i = 0; i < MEM_CAP; i += 16 ) {
        printf( "%4.4x\t", i );
        for ( int j = 0; j < 16; j++ ) {
            printf( "%2.2x  ", mem[ i + j ] );
        }
        printf( "\n" );
    }
    printf( "\n\n" );
    for ( int i = 0; i < REGISTER_COUNT; i++ ) {
        printf( "%s\t%2.2x\n", reg_str_repr[i], reg[i] );
    }
    printf( "Cycles: %ld", cycle );
}

void err (char *msg) {
    puts(msg);
}