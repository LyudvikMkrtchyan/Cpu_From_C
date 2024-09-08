#include "prototype.h"

//char machine_code[10] = {208, 8, 2 };
//char machine_code[18] = {209,8,2,52,209,12,2,53,209,20,6,42,5,10,17,21,65,20};
unsigned char machin_code_size;
char* machine_code;
unsigned char ram[RAM_SIZE];
unsigned char hdd[HDD_SIZE];
unsigned char mc_table[MC_TABLE_SIZE];
unsigned char tlb[TLB_SIZE];
char reg[REG_NUMBERS];
unsigned char IR;
unsigned short HX;
unsigned char ZA;
unsigned char op1_adr;
unsigned char op2_adr;
unsigned char opcode;
unsigned char functionality;
int pool; // 0 = featch , 1 = operand read, 2 = resulte store, 3 = tlb heat;
int instruction_size;
char op1;
char op2;
int count;
int mem_cont_ret;
char chlp;
char deb = 'n';
int main(int argc, char* argv[]){
    printf("if you whant debuge mode please enter -y- :");
    scanf("%c", &deb);
    LOADER(argv[1]);
    while(HX < 256){
        IR = GH_MMU(HX);
        pool = 0;
        opcode = IR >> 4;
        functionality = IR & 3;
        if(deb == 'y'){
            printf("  FEATCH  ");
            printf("HX = %d func %d opcode %d \n",HX ,functionality, opcode);
            debuge_see();
        }
        if(opcode != 13){
            switch(functionality){
                case 0: // reg - reg
                    IR = GH_MMU(++HX);
                    op1_adr = ((IR >> 4) & 15);
                    op2_adr = (IR & 15);
                    op1 = reg[op1_adr]; // op1 read;
                    op2 = reg[op2_adr]; // op2 read;
                    break;
                case 1: // reg - memory
                    op2_adr = ((IR & 12) << 3);
                    IR = GH_MMU(++HX);
                    op2_adr +=( IR & 31);
                    op1_adr = (IR >> 5);
                    op1 = reg[op1_adr];// op1 read;
                    op2 = MEMORY_READ(op2_adr); // op2 read;
                    break;
                case 2: // memory - reg
                    op1_adr = ((IR & 12) << 3);
                    IR = GH_MMU(++HX);
                    op1_adr += (IR & 31);
                    op2_adr = (IR >> 5);
                   
                    op1 = MEMORY_READ(op1_adr); // op1 read;
                    op2 = reg[op2_adr];         // op2 read;
                    break;
            }
        } 
        else
        { // reg - literal;
            op1_adr = (IR & 7);
            op2 = GH_MMU(++HX);
      //  scanf("%s", &chlp);
            op1 = reg[op1_adr]; // op1 read
            functionality = 0;
        }
        if(deb == 'y'){
            printf("\nOPERAND READ\n");
            printf("operand 1 = %d  operand 2 = %d\n",op1,op2);
            debuge_see();
        }
        pool = 1;
        switch(opcode){
            case 0: // mov
                op1 = op2;
                break;
            case 1: // add
                of_flag();
                op1 += op2;
                sf_zf_flag();
                break;
            case 2: // sub
                of_flag();
                op1 -= op2;
                sf_zf_flag();
                break;
            case 3: // cmp
                functionality = 3;
                of_flag();
                op1 -= op2;
                sf_zf_flag();
                break;
            case 4: // or
                of_flag();
                op1 = op1 | op2;
                sf_zf_flag();
                break;
            case 5: // and
                of_flag();
                op1 = op1 & op2;
                sf_zf_flag();
                break;
            case 6: // not
                op1 = ~op1;
                break;
            case 7: // div 
                of_flag();
                op1 = reg[0] / op2;
                sf_zf_flag();
                break;
            case 8: // mul
                of_flag();
                op1 = reg[0] * op2;
                sf_zf_flag();
                break;
            case 9: // jump
                functionality = 3;
                HX = GH_MMU(++HX) - 2 + PR_MAX_SIZE - instruction_size;
                break;
            case 10: // jg
            functionality = 3;
            bool ZF = ZA & 1;
            bool SF = ((ZA >> 1) & 1);
            bool OF = ((ZA >> 2) & 1);     
                if(!(SF ^ OF) & !ZF){
                    HX = GH_MMU(++HX) - 2 + PR_MAX_SIZE - instruction_size;
                }
                else {
                    HX++;
                }
                break;
            case 11: // jl
            functionality = 3;
             SF = ((ZA >> 1) & 1);
             OF = ((ZA >> 2) & 1);     
                if(SF ^ OF){
                    printf("chlp  \n");
                    HX = GH_MMU(++HX) - 2 + PR_MAX_SIZE - instruction_size;
                }
                else {
                    HX++;
                }
                break;
            case 12: // je
            functionality = 3;
                if(ZA & 1 == 1){
                    HX = GH_MMU(++HX) - 2 + PR_MAX_SIZE - instruction_size;
                }
                else {
                    HX++;
                }
                break;
            case 13:
                op1 = op2;
                break;
        }
         ++HX;
         //registr_see();
      //   printf("op1_addr = %d , op2_addr = %d, op1 = %d, op2 = %d\n", op1_adr,op2_adr,op1,op2 );
        switch(functionality) {
            case 0:   
            case 1:
               pool = 2;
                reg[op1_adr] = op1;
                break;
            case 2:
               pool = 2;
                MEMORY_WRITE(op1, op1_adr);
        }
        if(deb == 'y'){
            printf("\nRESULT STORE\n");
            debuge_see();
        }
    }
    printf("Result = %d\n ZA = %d", reg[0], ZA);
    return 0;
}
unsigned char GH_MMU(unsigned short virtual_address)
{     
    unsigned char ppn = MMU_TLB(virtual_address >> 2);
    ppn &= 15;
    MMU_MC(ppn);
    return ram[(ppn << 2) | (virtual_address & 3)]; // return ram value of ppn + offset address
}

char MEMORY_READ(unsigned char virtual_address)
{
    
    char ppn = MMU_TLB(virtual_address >> 2);
    ppn &= 15;
    MMU_MC(ppn);
    return ram[(ppn << 2) | (virtual_address & 3)];
}

void MEMORY_WRITE(char op1, unsigned char virtual_address) 
{
    unsigned char ppn = MMU_TLB(virtual_address >> 2);
    if ((ppn >> 6) & 1)
    {
        OS_SEGV_RONLY();
    }
    MMU_MC(ppn);
    unsigned char addr = (ppn << 2) | (virtual_address & 3);
    ram[addr] = op1;
}

// TLB 
unsigned char MMU_TLB(unsigned char Vpn){
    printf ("   MMU addresses TLB TAG = %d\n   ", HX >> 2);
    if(deb == 'y'){
        debuge_see();
    }
    for(int i = 0; i < TLB_SIZE; i += 2){
        if((Vpn == (tlb[i] & 63)) && ((tlb[i] >> 7) == 1)){
            if(deb == 'y'){
                mem_cont_ret = i;
                pool = 3;
                printf("   TLB HIT  \n");
                debuge_see();
            }
            return tlb[i + 1]; // return ppn(phisical page number)
        }
    }
    if(deb == 'y'){
            printf("   TLB MISS  \n");
    }
    unsigned char pte = TLB_MISS(Vpn);
  
    tlb[(tlb[16] % 16) * 2] = (Vpn | 128);
    tlb[(tlb[16] % 16) * 2 + 1] = pte;
    tlb[16]++;
    if(deb == 'y'){
        printf("TLB UPDATE  Vpn = %d, HX = %d\n", Vpn, HX);
        debuge_see();
    }
    return pte; 
}

unsigned char TLB_MISS(unsigned char VPN) 
{
       
    verev:
        if ((ram[VPN>>4] >> 7) & 1) {
            if ((ram[PAGE_SIZE + ((VPN >> 2) & 3)] >> 7) & 1) {
                if (((ram[8 + (VPN & 3)]) >> 7) & 1) {
                    return ram[8 + (VPN & 3)];
                }
                else { 
                    if(deb == 'y'){
                        printf("PAGE FAULT\n");
                        debuge_see();
                    }
                    PAGE_FAULT(ram[8 + (VPN & 3)], VPN);
                    ram[8 + (VPN & 3)] |= 128;
                    if(deb == 'y'){
                        printf("PAGE FAULTE IS HENDLES \n RAM UPDATES\n");
                        debuge_see();
                    }
                    goto verev;
                }
            } else {
                if(deb == 'y'){
                    printf("PTD MISS\n");
                    debuge_see();
                }
                PTD_FAULT((short)ram[4 +((VPN >> 2) & 3)]);
                INVALID(ram + PAGE_SIZE, (VPN >> 2) & 3);
                if(deb == 'y'){
                    printf("PTD UPDATE\n");
                    debuge_see();
                }
                goto verev;
            }
        } else {
            if(deb == 'y'){
                printf("PTG MISS\n");
                debuge_see();
            }
            PTG_FAULT((short)ram[VPN >> 4] & 3);
            INVALID(ram, ((VPN >> 4) & 3));
            if(deb == 'y'){
                printf("PTG UPDATE\n");
                debuge_see();
            }
            goto verev;
        }
}

//HANDLERS
void PTG_FAULT(short disk_address)
{
    disk_address = PR_MAX_SIZE + PAGE_SIZE + (PAGE_SIZE * disk_address);
    int mem_id = PAGE_SIZE;
    for (int i = 0; i < PAGE_SIZE; ++i, ++mem_id, ++disk_address) {
        ram[mem_id] = hdd[disk_address];
    }
}

void PTD_FAULT(short disk_address)
{
    disk_address = disk_address * PAGE_SIZE + PTBR;
    int mem_id = PAGE_SIZE * 2;
    for (int i = 0; i < PAGE_SIZE; ++i, ++mem_id, ++disk_address) {
        ram[mem_id] = hdd[disk_address];
    }
}

void PAGE_FAULT(unsigned char disk_address, unsigned char VPN)
{  
    if(deb == 'y'){
        printf("OC addresses HDD\n");
        getchar();
    }
    unsigned char PPN = PAGE_MISS(VPN);
    ram[8 + (VPN & 3)] = 128 | PPN;
    unsigned char PH_ADDRES = PPN * PAGE_SIZE;
    disk_address = (disk_address & 63) * PAGE_SIZE;
    for (int i = 0; i < PAGE_SIZE; ++i, ++PH_ADDRES, ++disk_address) {
        ram[PH_ADDRES] = hdd[disk_address];
    }
    mc_table[PPN + 1] = VPN; 
    mc_table[PPN] |= 128;
   
}

void MMU_MC(unsigned char ppn)
{  
   
    for (int i = 6; i < MC_TABLE_SIZE; i += 2) {
        if (i == (ppn * 2)) {
            mc_table[i] &= 128; 
        }
         mc_table[i]++;
    }
}

unsigned char PAGE_MISS()
{
    unsigned char index = 0;
    int count = -1;
    for (int i = 6; i < MC_TABLE_SIZE; i+=2) {
        if (((mc_table[i] & 127) > count) && ((mc_table[i] >> 7) == 0)) {
            count = mc_table[i] & 127;
            index = i/2;
        }
    }
    if (index == 0) {
        for (int i = 3; i < MC_TABLE_SIZE; i+=2) {
            if ((mc_table[i] & 127) > count) {
                count = mc_table[i] & 127;
                index = i/2;
            }
        }
       
    }
    mem_cont_ret = index;
    return index;
}
//OS
void INVALID(char* address, char valid)
{
    for (int i = 0; i < 4; ++i, ++address) {
        *address &= 127;
    }
    *(address - 4 + valid) |= 128;
}

void PT_UPDATE(unsigned char old_address, unsigned char new_address)
{
    //hdd[PTBR + old_address] = SWP_ADDRESS;
}

void OS_SEGV_NULL()
{
    printf("SEGMENTATION FAULT YOU DONT HAVE ANY DATA IN THIS ADDRESS\n");
    exit(0);
}

void OS_SEGV_RONLY()
{
    printf("YOU CAN NOT WRITE IN PAGE READ ONLY \n");
    exit(0);
}
void SWAP(char* first, char* second)
{
    for (int i = 0; i < PAGE_SIZE; ++i, ++first, ++second) {
        *first = *second;
    }
}
// FLAGS ASSIGN
void of_flag(){
    ZA = 0;
    if((op1 < 0 == op2 < 0) && (((op1 + op2) < 0) != (op1 < 0))){
        ZA |= 4;
    }
}
void sf_zf_flag(){
    if(op1 == 0)
        ZA |= 1;
    if((op1 >> 7) & 1)
        ZA |= 2;
}
//LOADER
void LOADER(char* arg)
{   FILE* chlp = fopen(arg, "r");
    int data_size = 0;
    unsigned char* data;
    char key[6];
    for(int i = 0; i < 5; ++i){
        key[i] = fgetc(chlp);
    }
    instruction_size = (int) fgetc(chlp);
    printf("inst size =  %d\n", instruction_size);
    machine_code = (unsigned char*)malloc((instruction_size));
    key[5] = '\0';
    printf("key = %s\n", key);
    if(strcmp(key,"*ATL*") != 0){
        printf("INVALID FORMAT");
    }
    for(int i = 0; i < instruction_size; ++i){
       machine_code[i] = (unsigned char)fgetc(chlp);
    }
    HX = PR_MAX_SIZE - instruction_size;
    for(int i = 0; i < instruction_size; ++i){
        hdd[HX + i] = machine_code[i];
    }
    data_size = (int) fgetc(chlp);
    printf("data size = %d\n", data_size);
    for(int i = 0; i < data_size; --data_size){
        hdd[128-data_size] = (char)fgetc(chlp);
    }

    PT_CREATE(instruction_size);
    SWAP(ram, hdd + PR_MAX_SIZE);
}

void PT_CREATE(int instruction_size)
{
    for (int i = PR_MAX_SIZE, j = 0; i < PR_MAX_SIZE + PAGE_SIZE; ++i, ++j) {
        hdd[i] = j;
    }

    for (int i = PR_MAX_SIZE + 4, j = 0; i < PR_MAX_SIZE + 20 ; ++i , ++j ) {
        hdd[i] = j;
    }

    for (int i = 0; i < PT_SIZE; ++i) {
        int read = PT_SIZE - (instruction_size / 4) - 1;
        hdd[PTBR + i] = i;
        if (i >= read) {
            hdd[PTBR + i] |= 64;
        }
    }
}
void ram_see() // printf("\033[1;31mWELCOME TO OPENGENUS\033[0m\n");
{
      printf("     RAM    \n");
        printf("=======================\n");
        printf("|  index  |   value   |\n");
        printf("=======================\n");
    for(int i = 0; i < RAM_SIZE; i++){
        if(functionality == 2 && pool == 2 && i == (op1_adr + (mem_cont_ret * PAGE_SIZE))){
            if(i < 10){
                printf("\033[1;31m|   %d    | \033[0m", i);
            }
            else {
                printf("\033[1;31m|   %d   | \033[0m", i);
            }
            for(int j = 7; j >= 0; --j){
                printf("\033[1;31m%d\033[0m", ((ram[i] >> j) & 1));
            }
            printf("\033[1;31m   |\033[0m\n");
            
        }
        else{
            if(i < 10){
                printf("|   %d    | ", i);
            }
            else {
                printf("|   %d   | ", i);
            }
            for(int j = 7; j >= 0; --j){
                printf("%d", ((ram[i] >> j) & 1));
            }
            printf("   |\n");
            
        }
        if((i + 1) % 4 == 0){
            printf("----------------------\n");
        }  
    }
    scanf("%s", &chlp);
}
void registr_see(){
        printf("     REGISTRS \n");
    printf(" ===================\n");
    printf("|  name |   value   |\n");
    printf(" ===================\n");
    for(int i = 0; i < REG_NUMBERS; i++){
        char reg_name[4];
        switch(i){
            case 0: strcpy(reg_name, "AYB"); break;
            case 1: strcpy(reg_name, "BEN"); break;
            case 2: strcpy(reg_name, "GIM"); break;
            case 3: strcpy(reg_name, "DAH"); break;
            case 4: strcpy(reg_name, "TMP"); break;
            case 5: strcpy(reg_name, "TMP"); break;
            case 6: strcpy(reg_name, "TMP"); break;
            case 7: strcpy(reg_name, "TMP"); break;
        }
        if((functionality == 1 || functionality == 0) && (i == op1_adr) && (pool == 2)){ // print update line 
            printf("\033[1;31m|  %s  | \033[0m", reg_name);   
            for(int j = 7; j >= 0; --j){
                printf("\033[1;31m%d\033[0m", ((reg[i] >> j) & 1));
            }
            printf("\033[1;31m  |\033[0m\n");
            printf("====================\n");
        }
        else{                                       // print others lines
            printf("|  %s  | ", reg_name);   
            for(int j = 7; j >= 0; --j){
                printf("%d", ((reg[i] >> j) & 1));
            }
            printf("  |\n");
            printf("====================\n");
        }
    }
    getchar();
}
void tlb_see(){ // printf("\033[1;31mWELCOME TO OPENGENUS\033[0m\n");
    printf(" ==========================\n");
    printf("|valid|  TAG   |   PTE     |\n");
    printf(" ==========================\n");
    for(int i = 0; i < TLB_SIZE - 1; i += 2){
        if(pool == 3 && mem_cont_ret == i){
            for(int j = 5; j >= 0; --j){
                if(j == 5 ){
                    printf("\033[1;32m| %d   | \033[0m",((tlb[i] >> 7) & 1));
                }
                else{
                    printf("\033[1;32m%d\033[0m", ((tlb[i] >> j) & 1));
                }
            }
            printf("\033[1;32m  |  \033[0m");
            for(int j = 7; j >= 0; --j){
                printf("\033[1;32m%d\033[0m", ((tlb[i + 1] >> j) & 1));
            }
        printf("\033[1;32m |\033[0m\n");
        }
        else{
                for(int j = 5; j >= 0; --j){
                    if(j == 5 ){
                        printf("| %d   | ",((tlb[i] >> 7) & 1));
                    }
                    else{
                        printf("%d", ((tlb[i] >> j) & 1));
                    }
                }
                printf("  |  ");
                for(int j = 7; j >= 0; --j){
                    printf("%d", ((tlb[i + 1] >> j) & 1));
                }
                printf(" |\n");
        }
    printf("===========================\n");

    }
    pool = 0;
 getchar();
}
void hdd_see(){
    for(int i = 0; i < 256; i++){
        if(i < 10){
            printf("Hdd add  = %d ram val = ", i);
        }
        else {
            printf("Hdd add = %d ram val = ", i);
        }
        for(int j = 7; j >= 0; --j){
            printf("%d", ((hdd[i] >> j) & 1));
        }
        printf("\n");
    }
  getchar();
   
}
void ptr_see(){
    for(int i = 0; i < 256; i++){
        if(i < 10){
            printf(" add  = %d ram val = ", i);
        }
        else {
            printf("Hdd add = %d hdd val = ", i);
        }
        for(int j = 7; j >= 0; --j){
            printf("%d", ((hdd[i] >> j) & 1));
        }
        printf("\n");
    }
    getchar();
}
void vmem_see(){
      printf("     HDD    \n");
        printf("=======================\n");
        printf("|  index  |   value   |\n");
        printf("=======================\n");
    for(int i = 0; i < PR_MAX_SIZE; i++){
      
       
        if(i < 10){
            printf("|   %d    | ", i);
        }
        else if(i < 100){
            printf("|   %d   | ", i);
             }
            else{
                printf("|   %d  | ", i);
            }
        for(int j = 7; j >= 0; --j){
            printf("%d", ((hdd[i] >> j) & 1));
        }
        printf("   |\n");
         if((i + 1) % 4 == 0){
            printf("----------------------\n");
        }  
    }
    
    getchar();
}

void debuge_see()
{
    int see  = 0;
    char kays[7];
    printf("memory - m | tlb - t | registor - r | hdd - h | memcnt - c \n");
    scanf("%s", kays);
    printf("%s", kays);
    for(int i = 0; kays[i] != '\0'; ++i){
        switch(kays[i]){
            case 'm': ram_see();    break;
            case 't': tlb_see();     break;
            case 'r': registr_see(); break;
            case 'h': vmem_see();    break;
            }
    }
}
