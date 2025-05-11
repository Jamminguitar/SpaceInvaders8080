#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// Emulate step
uint16_t emulateOp8080(unsigned char *buffer, int pc);

// Helper functions
uint16_t getMemoryAddress();
uint8_t stall_cycles = 0;

// In order, the registers are: B, C, D, E, H, L, N/A, A
uint8_t registers8080[8];

// List of register names (the X register represents memory operations)
char registerNames8080[] = {'B', 'C', 'D', 'E', 'H', 'L', 'X', 'A'};
// List of register pairs
char registerPairs8080[][10] = {"B-C", "D-E", "H-L", "SP"};
// List of condition codes
char conditions8080[][10] = {"NZ", " Z", "NC", " C", "PO", "PE", " P", " M"};

// Memory space (2^16 addresses)
uint8_t memory8080[65536];

// Instruction registers
uint16_t SP = 65535;
uint16_t pc = 0;


int main(int argc, char** argv)
{
    if (argc < 2) {
        printf("Please include a file when running the 8080 emulator.\n");
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
    while (1)
    {
        if (stall_cycles > 0) stall_cycles--;
        else pc = emulateOp8080(romBuffer, pc);
    }

    return 0;

}

uint16_t emulateOp8080(unsigned char *buffer, int pc)
{
    unsigned char *instruction = &buffer[pc];
    int opsize = 1;

    // extra variables for the switch statement
    uint16_t address;


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
        // ORI data: OR immediate (takes 2 cycles)
        case 0b11110110: opsize = 2; printf("%02X       ORI %02X  (A) <- (A) OR %02X", instruction[1], instruction[1], instruction[1]); break;
        // XRI data: Exclusive OR immediate (takes 2 cycles)
        case 0b11101110: opsize = 2; printf("%02X       XRI %02X  (A) <- (A) XOR %02X", instruction[1], instruction[1], instruction[1]); break;
        // ANI data: AND immediate (takes 2 cycles)
        case 0b11100110: opsize = 2; printf("%02X       ANI %02X  (A) <- (A) AND %02X", instruction[1], instruction[1], instruction[1]); break;
        // DAA: Decimal Adjust Accumulator
        case 0b00100111: printf("         DAA"); break;
        // SBI data: Subtract immediate with borrow (takes 2 cycles)
        case 0b11011110: opsize = 2; printf("%02X       SBI %02X  (A) <- (A) - %02X - (CY)", instruction[1], instruction[1], instruction[1]); break;
        // SUI data: Subtract immediate (takes 2 cycles)
        case 0b11010110: opsize = 2; printf("%02X       SUI %02X  (A) <- (A) - %02X", instruction[1], instruction[1], instruction[1]); break;
        // ACI data: Add immediate with carry (takes 2 cycles)
        case 0b11001110: opsize = 2; printf("%02X       ACI %02X  (A) <- (A) + %02X + (CY)", instruction[1], instruction[1], instruction[1]); break;
        // ADI data: Add immediate (takes 2 cycles)
        case 0b11000110: opsize = 2; printf("%02X       ADI %02X  (A) <- (A) + %02X", instruction[1], instruction[1], instruction[1]); break;
        // XCHG: Exchange H and L with D and E
        case 0b11101011: printf("         XCHG");
        // Exchange H and D
        registers8080[4] = registers8080[4] + registers8080[2];
        registers8080[2] = registers8080[4] - registers8080[2];
        registers8080[4] = registers8080[4] - registers8080[2];

        // Exchange L and E
        registers8080[5] = registers8080[5] + registers8080[3];
        registers8080[3] = registers8080[5] - registers8080[3];
        registers8080[5] = registers8080[5] - registers8080[3];
        break;
        // SHLD addr: Store H and L direct (takes 5 cycles)
        case 0b00100010: opsize = 3; printf("%02X %02X    SHLD %02X %02X", instruction[1], instruction[2], instruction[2], instruction[1]);
        stall_cycles = 4;
        address = ((uint16_t) instruction[2] << 8) + (uint16_t) instruction[1];
        memory8080[address] = registers8080[5];         // L
        memory8080[address + 1] = registers8080[4];    // H
        break;
        // LHLD addr: Load H and L direct (takes 5 cycles)
        case 0b00101010: opsize = 3; printf("%02X %02X    LHLD %02X %02X", instruction[1], instruction[2], instruction[2], instruction[1]);
        stall_cycles = 4;
        address = ((uint16_t) instruction[2] << 8) + (uint16_t) instruction[1];
        registers8080[5] = memory8080[address];         // L
        registers8080[4] = memory8080[address + 1];     // H
        break;
        // STA addr: Store Accumulator direct (takes 4 cycles)
        case 0b00110010: opsize = 3; printf("%02X %02X    STA %02X %02X", instruction[1], instruction[2], instruction[2], instruction[1]);
        stall_cycles = 3;
        address = ((uint16_t) instruction[2] << 8) + (uint16_t) instruction[1];
        memory8080[address] = registers8080[7];         // A
        break;
        // LDA addr: Load Accumulator direct (takes 4 cycles)
        case 0b00111010: opsize = 3; printf("%02X %02X    LDA %02X %02X", instruction[1], instruction[2], instruction[2], instruction[1]); 
        stall_cycles = 3;
        address = ((uint16_t) instruction[2] << 8) + (uint16_t) instruction[1];
        registers8080[7] = memory8080[address];         // A
        break;
        // Unused codes
        case 0b00001000: printf("         UNKNOWN"); break;
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
            printf("         R %s", conditions8080[(*instruction & 0b00111000) >> 3]);
        }
        // C(Condition): Conditional call (takes 3 or 5 cycles)
        else if ((*instruction & 0b11000111) == 0b11000100)
        {
            opsize = 3;
            printf("%02X %02X    C %s %02X %02X", instruction[1], instruction[2], conditions8080[(*instruction & 0b00111000) >> 3], instruction[2], instruction[1]);
        }
        // J(Condition): Conditional jump (takes 3 cycles)
        else if ((*instruction & 0b11000111) == 0b11000010)
        {
            opsize = 3;
            printf("%02X %02X    J %s %02X %02X", instruction[1], instruction[2], conditions8080[(*instruction & 0b00111000) >> 3], instruction[2], instruction[1]);
        }
        // CMP
        else if ((*instruction & 0b11111000) == 0b10111000)
        {
            opsize = 1;
            // CMP M: Compare memory (takes 2 cycles)
            if ((*instruction & 0b11111111) == 0b10111110)
            {
                printf("         CMP M  (A) - ((H) (L))");
            }
            // CMP R: Compare Register
            else
            {
                printf("         CMP %c  (A) - (%c)", registers8080[(*instruction & 0b00000111)], registers8080[(*instruction & 0b00000111)]);
            }
        }
        // ORA
        else if ((*instruction & 0b11111000) == 0b10110000)
        {
        	opsize = 1;
            // ORA M: OR Memory (takes 2 cycles)
            if ((*instruction & 0b11111111) == 0b10110110)
            {
                printf("         ORA M  (A) <- (A) OR ((H)(L))");
            }
            // ORA r: OR Register
            else
            {
                printf("         ORA %c  (A) <- (A) OR (%c)", registers8080[(*instruction & 0b00000111)], registers8080[(*instruction & 0b00000111)]);
            }
        }
        // XRA
        else if ((*instruction & 0b11111000) == 0b10101000)
        {
        	opsize = 1;
            // XRA M: XOR Memory (takes 2 cycles)
            if ((*instruction & 0b11111111) == 0b10101110)
            {
                printf("         XRA M  (A) <- (A) XOR ((H)(L))");
            }
            // XRA r: Exclusive OR Register
            else
            {
                printf("         XRA %c  (A) <- (A) XOR (%c)", registers8080[(*instruction & 0b00000111)], registers8080[(*instruction & 0b00000111)]);
            }
        }
        // ANA
        else if ((*instruction & 0b11111000) == 0b10100000)
        {
        	opsize = 1;
            // ANA M: AND Memory (takes 2 cycles)
            if ((*instruction & 0b11111111) == 0b10100110)
            {
                printf("         ANA M  (A) <- (A) AND ((H)(L))");
            }
            // ANA r: AND Register
            else
            {
                printf("         ANA %c  (A) <- (A) AND (%c)", registers8080[(*instruction & 0b00000111)], registers8080[(*instruction & 0b00000111)]);
            }
        }
        // DAD rp: Add register pair to H and L (takes 3 cycles)
        else if ((*instruction & 0b11001111) == 0b00001001)
        {
        	opsize = 1;
            printf("         DAD %s  (H)(L) <- (H)(L) + (%s)", registerPairs8080[(*instruction & 0b00110000) >> 4], registerPairs8080[(*instruction & 0b00110000) >> 4]);
        }
        // DCX rp: Decrement register pair
        else if ((*instruction & 0b11001111) == 0b00001011)
        {
        	opsize = 1;
            printf("         DCX %s  (%s) <- (%s) - 1", registerPairs8080[(*instruction & 0b00110000) >> 4], registerPairs8080[(*instruction & 0b00110000) >> 4], registerPairs8080[(*instruction & 0b00110000) >> 4]);
        }
        // ICX rp: Increment register pair
        else if ((*instruction & 0b11001111) == 0b00000011)
        {
        	opsize = 1;
            printf("         ICX %s  (%s) <- (%s) + 1", registerPairs8080[(*instruction & 0b00110000) >> 4], registerPairs8080[(*instruction & 0b00110000) >> 4], registerPairs8080[(*instruction & 0b00110000) >> 4]);
        }
        // DCR
        else if ((*instruction & 0b11000111) == 0b00000101)
        {
        	opsize = 1;
            // DCR M: Decrement Memory (takes 3 cycles)
            if ((*instruction & 0b11111111) == 0b00110101)
            {
                printf("         DCR M  ((H)(L)) <- ((H)(L)) - 1");
            }
            // DCR r: Decrement Register
            else
            {
                printf("         DCR %c  (%c) <- (%c) - 1", registers8080[(*instruction & 0b00111000) >> 3], registers8080[(*instruction & 0b00111000) >> 3], registers8080[(*instruction & 0b00111000) >> 3]);
            }
        }
        // INR
        else if ((*instruction & 0b11000111) == 0b00000100)
        {
        	opsize = 1;
            // INR M: Increment Memory (takes 3 cycles)
            if ((*instruction & 0b11111111) == 0b00110100)
            {
                printf("         INR M  ((H)(L)) <- ((H)(L)) + 1");
            }
            // INR r: Increment Register
            else
            {
                printf("         INR %c  (%c) <- (%c) + 1", registers8080[(*instruction & 0b00111000) >> 3], registers8080[(*instruction & 0b00111000) >> 3], registers8080[(*instruction & 0b00111000) >> 3]);
            }
        }
        // SBB
        else if ((*instruction & 0b11111000) == 0b10011000)
        {
        	opsize = 1;
            // SBB M: Subtract memory with borrow (takes 2 cycles)
            if ((*instruction & 0b11111111) == 0b10011110)
            {
                printf("         SBB M  (A) <- (A) - ((H)(L)) - (CY)");
            }
            // SBB r: Subtract Register with borrow
            else
            {
                printf("         SBB %c  (A) <- (A) - (%c) - (CY)", registers8080[(*instruction & 0b00000111)], registers8080[(*instruction & 0b00000111)]);
            }
        }
        // SUB
        else if ((*instruction & 0b11111000) == 0b10010000)
        {
        	opsize = 1;
            // SBB M: Subtract memory (takes 2 cycles)
            if ((*instruction & 0b11111111) == 0b10011110)
            {
                printf("         SUB M  (A) <- (A) - ((H)(L))");
            }
            // SUB r: Subtract Register
            else
            {
                printf("         SUB %c  (A) <- (A) - (%c)", registers8080[(*instruction & 0b00000111)], registers8080[(*instruction & 0b00000111)]);
            }
        }
        // ADC
        else if ((*instruction & 0b11111000) == 0b10001000)
        {
        	opsize = 1;
            // ADC M: Add memory with carry (takes 2 cycles)
            if ((*instruction & 0b11111111) == 0b10001110)
            {
                printf("         ADC M  (A) <- (A) + ((H)(L)) + (CY)");
            }
            // ADC r: Add Register with carry
            else
            {
                printf("         ADC %c  (A) <- (A) + (%c) + (CY)", registers8080[(*instruction & 0b00000111)], registers8080[(*instruction & 0b00000111)]);
            }
        }
        // ADD
        else if ((*instruction & 0b11111000) == 0b10000000)
        {
        	opsize = 1;
            // ADD M: Add memory (takes 2 cycles)
            if ((*instruction & 0b11111111) == 0b10000110)
            {
                printf("         ADD M  (A) <- (A) + ((H)(L))");
            }
            // ADD r: Add Register
            else
            {
                printf("         ADD %c  (A) <- (A) + (%c)", registers8080[(*instruction & 0b00000111)], registers8080[(*instruction & 0b00000111)]);
            }
        }
        // STAX rp: Store Accumulator indirect (takes 2 cycles)
        else if ((*instruction & 0b11001111) == 0b00000010)
        {
        	opsize = 1;
            printf("         STAX %s", registerPairs8080[(*instruction & 0b00110000) >> 4]);
            stall_cycles = 1;
            uint16_t address = ((uint16_t) registers8080[(*instruction & 0b00110000) >> 3] << 8) + (uint16_t) registers8080[((*instruction & 0b00110000) >> 3) + 1];
            memory8080[address] = registers8080[7];
        }
        // LDAX rp: Load Accumulator indirect (takes 2 cycles)
        else if ((*instruction & 0b11001111) == 0b00001010)
        {
        	opsize = 1;
            printf("         LDAX %s", registerPairs8080[(*instruction & 0b00110000) >> 4]);
            stall_cycles = 1;
            uint16_t address = ((uint16_t) registers8080[(*instruction & 0b00110000) >> 3] << 8) + (uint16_t) registers8080[((*instruction & 0b00110000) >> 3) + 1];
            registers8080[7] = memory8080[address];
        }
        // LXI rp, data: Load register pair immediate (takes 3 cycles)
        else if ((*instruction & 0b11001111) == 0b00000001)
        {
            opsize = 3;
            printf("%02X %02X    LXI %s", instruction[1], instruction[2], registerPairs8080[(*instruction & 0b00110000) >> 4]);
            stall_cycles = 2;
            // Loading to the stack pointer
            if (((*instruction & 0b00110000) >> 4) == 3)
            {
                uint16_t immediate = ((uint16_t) instruction[2] << 8) + (uint16_t) instruction[1];
                SP = immediate;
            }
            // Loading to another register pair
            else
            {
                registers8080[(*instruction & 0b00110000) >> 3] = instruction[2];
                registers8080[((*instruction & 0b00110000) >> 3) + 1] = instruction[1];
            }
        }
        // MVI
        else if ((*instruction & 0b11000111) == 0b00000110)
        {
            opsize = 2;
            // MVI M, data: Add memory (takes 3 cycles)
            if ((*instruction & 0b11111111) == 0b00110110)
            {
                printf("%02X       MVI M, %02X", instruction[1], instruction[1]);
                stall_cycles = 2;
                uint16_t address = getMemoryAddress();
                memory8080[address] = instruction[1];
            }
            // MVI r, data: Move Immediate (takes 2 cycles)
            else
            {
                printf("%02X       MVI %c, %02X", instruction[1], registerNames8080[(*instruction & 0b00111000) >> 3], instruction[1]);
                stall_cycles = 1;
                registers8080[(*instruction & 0b00111000) >> 3] = instruction[1];
            }
        }
        // MOV
        else if ((*instruction & 0b11000000) == 0b01000000)
        {
        	opsize = 1;
            // MOV M, r: Move to memory (takes 2 cycles)
            if ((*instruction & 0b11111000) == 0b01110000)
            {
                printf("         MOV M, %c", registerNames8080[(*instruction & 0b00000111)]);
                stall_cycles = 1;
                uint16_t address = getMemoryAddress();
                memory8080[address] = registers8080[(*instruction & 0b00111000) >> 3];
            }
            // MOV r, M: Move from memory (takes 2 cycles)
            else if ((*instruction & 0b11000111) == 0b01000110)
            {
                printf("         MOV %c, M", registerNames8080[(*instruction & 0b00111000) >> 3]);
                stall_cycles = 1;
                uint16_t address = getMemoryAddress();
                registers8080[(*instruction & 0b00111000) >> 3] = memory8080[address];
            }
            // MOV r1, r2: Move Register
            else
            {
                printf("         MOV %c, %c", registerNames8080[(*instruction & 0b00111000) >> 3], registerNames8080[(*instruction & 0b00000111)]);
                registers8080[(*instruction & 0b00111000) >> 3] = registers8080[(*instruction & 0b00000111)];
            }
        }
    }

    printf("\n");
    // return the address of the next instruction
    return opsize + pc;
}

uint16_t getMemoryAddress() {
    return ((uint16_t) registers8080[4] << 8) + (uint16_t) registers8080[5];
}