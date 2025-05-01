#include <stdio.h>
#include <stdlib.h>

int disassembleOp8080(unsigned char *buffer, int pc);

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Please include a file when running the 8080 disassembler.\n");
        exit(1);
    }
    FILE *f = fopen(argv[1], "rb");
    if (f == NULL)
    {
        printf("error: could not read file %s\n", argv[1]);
        exit(2);
    }

    // Get the file size and read it into a memory buffer
    fseek(f, 0L, SEEK_END);
    int fsize = ftell(f);
    fseek(f, 0L, SEEK_SET);

    unsigned char *romBuffer = malloc(fsize);

    fread(romBuffer, fsize, 1, f);
    fclose(f);

    // Increment through rom and display every instruction
    int pc = 0;
    while (pc < fsize)
    {
        pc += disassembleOp8080(romBuffer, pc);
    }

    return 0;

}

int disassembleOp8080(unsigned char *buffer, int pc) {
    unsigned char *instruction = &buffer[pc];
    int opsize = 1;

    printf("%02X ", *instruction);
    switch(*instruction)
    {
        // NOP: No op
        case 0b00000000: printf("         NOP"); break;
        // HLT: Halt
        case 0b01110110: printf("         HLT"); break;
        // DI: Disable Interrupts
        case 0b11110011: printf("         DI"); break;
        // EI: Enable interrupts
        case 0b11111011: printf("         EI"); break;
        // OUT: Output (takes 3 cycles)
        case 0b11010011: opsize = 2; printf("%02X       OUT    %02X", instruction[1], instruction[1]); break;
        // IN: Input (takes 3 cycles)
        case 0b11011011: opsize = 2; printf("%02X       IN     %02X", instruction[1], instruction[1]); break;
        // SPHL: Move HL to SP
        case 0b11111001: printf("         SPHL   (SP) <- (H)(L)"); break;
        // XTHL: Exchange stack top with H and L (takes 5 cycles)
        case 0b11100011: printf("         XTHL   (L) <-> ((SP)) (H) <-> ((SP)+1)"); break;
        // PCHL: Jump H and L indirect - move H and L to PC
        case 0b11101001: printf("         PCHL   (PCH) <- (H) (PCL) <- (L)"); break;
        // RET: Return
        case 0b11001001: printf("         RET"); break;
        // CALL: Call
        case 0b11001101: opsize = 3; printf("%02X %02X    CALL %02X %02X", instruction[1], instruction[2], instruction[2], instruction[1]); break;
        // JMP: Jump
        case 0b11000011: opsize = 3; printf("%02X %02X    JMP %02X %02X", instruction[1], instruction[2], instruction[2], instruction[1]); break;
        // STC: Set Carry
        case 0b00110111: printf("         STC    (CY) <- 1"); break;
        // CMC: Complement Carry
        case 0b00111111: printf("         CMC    (CY) <- !(CY)"); break;
        // CMA: Complement Accumulator
        case 0b00101111: printf("         CMA    (A) <- !(A)"); break;
        // RAR: Rotate right through carry
        case 0b00011111: printf("         RAR    (An) <- (An+1) (CY) <- (A0) (A7) <- (CY)"); break;
        // RAL: Rotate left through carry
        case 0b00010111: printf("         RAL    (An+1) <- (An) (CY) <- (A7) (A0) <- (CY)"); break;
        // RRC: Rotate right
        case 0b00001111: printf("         RRC    (An) <- (An+1) (A7) <- (A0) (CY) <- (A0)"); break;
        // RLC: Rotate left
        case 0b00000111: printf("         RLC    (An+1) <- (An) (A0) <- (A7) (CY) <- (A7)"); break;
        // CPI: Compare immediate (takes 2 cycles)
        case 0b11111110: opsize = 2; printf("%02X       CPI %02X", instruction[1], instruction[1]); break;
        // CMP M: Compare memory (takes 2 cycles)
        case 0b10111110: printf("         CMP M  (A) - ((H) (L))"); break;
        
        default: opsize = 0; break;
    }

    // Instruction includes variable information
    if (opsize == 0)
    {
        // POP: Pop (takes 3 cycles)
        if ((*instruction & 0b11001111) == 0b11000001)
        {
            opsize = 1;
            if ((*instruction & 0b00110000) == 0b00000000)
            {
                // Pop B
                printf("         POP B  C <- (SP) B <- (SP+1)");
            }
            else if ((*instruction & 0b00110000) == 0b00010000)
            {
                // Pop D
                printf("         POP D  E <- (SP) D <- (SP+1)");
            }
            else if ((*instruction & 0b00110000) == 0b00100000)
            {
                // Pop H
                printf("         POP H  L <- (SP) H <- (SP+1)");
            }
            else if ((*instruction & 0b00110000) == 0b00110000)
            {
                // Pop processor status word
                printf("         POP PSW");
            }
        }
        // PUSH: Push (takes 3 cycles)
        else if ((*instruction & 0b11001111) == 0b11000101)
        {
            opsize = 1;
            if ((*instruction & 0b00110000) == 0b00000000)
            {
                // Push B
                printf("         PUSH B  C -> (SP) B -> (SP+1)");
            }
            else if ((*instruction & 0b00110000) == 0b00010000)
            {
                // Push D
                printf("         PUSH D  E -> (SP) D -> (SP+1)");
            }
            else if ((*instruction & 0b00110000) == 0b00100000)
            {
                // Push H
                printf("         PUSH H  L -> (SP) H -> (SP+1)");
            }
            else if ((*instruction & 0b00110000) == 0b00110000)
            {
                // Push processor status word
                printf("         PUSH PSW");
            }
        }
        // RST: Restart (takes 3 cycles)
        else if ((*instruction & 0b11000111) == 0b11000111)
        {
            opsize = 1;
            printf("         RST    %02X", *instruction & 0b00111000);
        }
        // R(Condition): Conditional return (takes 1 or 3 cycles)
        else if ((*instruction & 0b11000111) == 0b11000000)
        {
            opsize = 1;
            printf("         R%01X", (*instruction & 0b00111000) >> 3);
        }
        // C(Condition): Conditional call (takes 3 or 5 cycles)
        else if ((*instruction & 0b11000111) == 0b11000100)
        {
            opsize = 3;
            printf("%02X %02X    C%01X %02X %02X", instruction[1], instruction[2], (*instruction & 0b00111000) >> 3, instruction[2], instruction[1]);
        }
        // J(Condition): Conditional jump (takes 3 cycles)
        else if ((*instruction & 0b11000111) == 0b11000010)
        {
            opsize = 3;
            printf("%02X %02X    J%01X %02X %02X", instruction[1], instruction[2], (*instruction & 0b00111000) >> 3, instruction[2], instruction[1]);
        }
        // CMP R: Compare Register
        else if ((*instruction & 0b11111000) == 0b10111000)
        {
            opsize = 1;
            printf("         CMP %01X  (A) - (%01X)", (*instruction & 0b00000111), (*instruction & 0b00000111));
        }

    }
    // Debug statement while not all opcodes are implemented
    if (opsize == 0) opsize = 1;

    printf("\n");
    return opsize;
}