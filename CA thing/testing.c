#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>


int memory [2048];

struct MEM{
int opcode;
int valuefrommemory;
int memloc;
int r1;
};

struct EXEC{
    int opcode;
    int result;
    int r1;
};

struct DEC{
unsigned opcode;
unsigned r1;
unsigned r2;
unsigned r3;
int valueOfR1;
unsigned shamt;
int imm;
unsigned address;
};

struct FETCH{
int instruction;
};

struct FETCH f;
struct DEC dec;
struct EXEC exec;
struct MEM mem;



//global variables
int pc= 0; //our global pc
int registerFile[32]; 
int memindexInstr=0;
int memindexData=1024;
int totalclkcycles;
int clockCycles=1; 
int branchflag=0;
int decodeFlag=0; //fakedecode 
int executeFlag=-1; //fakeexecute
int j=6;
int memafterbranch=0;

void fetch() {
    printf("Entering fetch\n");
    printf("pc before fetching is: %i\n",pc);
    f.instruction = memory[pc];
    pc++;
    printf("Fetching instruction %d: %u\n",pc, f.instruction);
    printf("PC after fetching is: %i\n", pc);   
}
void decode(){
        unsigned opcode = f.instruction & 0b11110000000000000000000000000000;
        dec.opcode=opcode >> 28;
        //printf("our opcode: %d\n",dec.opcode);
        unsigned r1 = f.instruction & 0b00001111100000000000000000000000;
        dec.r1= r1 >> 23;
        dec.valueOfR1=registerFile[dec.r1];
        //printf("our r1 dest is: %d\n",dec.r1);

        unsigned  r2 = f.instruction & 0b00000000011111000000000000000000;
        dec.r2= registerFile[ (r2 >> 18)];
        //printf("our r2 src is: %d\n",dec.r2);

        unsigned  r3 =f.instruction & 0b00000000000000111110000000000000;
        dec.r3=registerFile[ (r3 >> 13)];
        //printf("our r3 src is: %d\n",dec.r3);

        dec.shamt=f.instruction & 0b00000000000000000001111111111111;
        int temp =f.instruction & 0b00000000000000111111111111111111;
        int temp1 =temp>>17;
        if(temp1==1)
            dec.imm= temp| 0b11111111111111000000000000000000;
        
        else 
            dec.imm=temp;
            dec.address=f.instruction & 0b00001111111111111111111111111111;
}
void execute(){
    exec.opcode=dec.opcode;
    exec.r1=dec.r1;
    //printf("our r1 dest in exec method is: %d\n",exec.r1);

    if(dec.opcode!=-1 || dec.r1!=-1){
        switch (dec.opcode){
            case 0 : exec.result = dec.r2 +dec.r3;break;
            case 1 : exec.result =dec.r2 - dec.r3; break;
            case 2 : exec.result = dec.r2 * dec.r3; break;
            case 3 : exec.result = dec.imm; break;
            case 4 : 
                    if(dec.valueOfR1==dec.r2){
                        branchflag=1;
                        pc=pc+dec.imm-2;
                    }
                    break;
            case 5 : exec.result = dec.r2 & dec.r3; break;
            case 6 : exec.result = dec.r2 ^ dec.imm; break;
            case 7: int msb_pc = pc >> 28;
                    int lsb_address = dec.address & 0xFFFFFFF;
                    pc=(msb_pc << 28) | lsb_address;
                    branchflag=1;
                    break;
            case 8 : exec.result = dec.r2 << dec.shamt; break;
            case 9 :exec.result = dec.r2 >> dec.shamt;break;
            case 10:exec.result= dec.r2+dec.imm;break;
            case 11:exec.result=dec.r2+dec.imm;break;
        }
    }
    else{
        exec.opcode=-1;
        exec.r1=-1;
        exec.result=-1;
    }   
}
void memoryAccess(){
    mem.opcode=exec.opcode;
    mem.memloc=exec.result;
    mem.r1=exec.r1;
    
    if(exec.opcode!=-1 && exec.result != -1 && exec.r1!=-1){
        if(exec.opcode==10){
            printf("reading from memory location %i\n",exec.result);
            mem.valuefrommemory= memory[exec.result];
            printf("%i\n",mem.valuefrommemory);
    }
        else if(exec.opcode==11){
            memory[exec.result]=registerFile[exec.r1];
            printf("value stored at memory location %i is now set to %i\n ",exec.result,exec.r1);
        }
        else{
            printf("entered memory but no memory involvement\n");
    }
}
}
void writeBack(){

    printf("our reg value to be written in currently in the write back is %d\n", mem.r1);
    printf("our opcode is %d\n",mem.opcode);
    if (mem.opcode!=-1&& mem.r1!=-1){
        if(mem.opcode!=11 && mem.opcode!=10 && mem.opcode!=4 && mem.opcode!=7){
            if(mem.r1!=0){
                printf("I am now writing back yay in clkcycle %d\n",clockCycles);
                    registerFile[mem.r1]=mem.memloc;
                    }
        printf("value of Register %i is now set to %i\n ",mem.r1,mem.memloc);
    }
    else if(mem.opcode==10){
        registerFile[mem.r1]= mem.valuefrommemory;
        printf("value of Register %i is now set to %i\n ",mem.r1,mem.valuefrommemory);
    }
    }
    else 
    printf("no write back\n");  

}

void pipelining(){
    while(pc<memindexInstr||j>0){
        if(pc==memindexInstr){
            j--;
        }
        printf("%i\n",mem.r1);
        if(mem.r1!=-1 && clockCycles%2==1)
            writeBack();
        if(exec.opcode!=-1&&clockCycles%2==0&&(memafterbranch==0||memafterbranch==1))
            memoryAccess();
        if(memafterbranch==1&&clockCycles%2==0){
            memafterbranch++;
        }else{
            if(memafterbranch==2&&clockCycles%2==0){
                memafterbranch++;
                mem.r1=-1;
            }
            else if(memafterbranch==3&&clockCycles%2==0){
                mem.r1=-1;
                memafterbranch=0;
            }
        }
        printf("our opcode check before exec in the pipelining is: %d\n",dec.opcode);

        if(executeFlag==0 && dec.opcode!=-1&&clockCycles%2==0){
            printf("here is the real exec time(1st exec cycle)\n");
            execute();
            dec.opcode = -1;
            executeFlag = 1;
        }
        else if(executeFlag==1&&clockCycles%2==1){
            printf("here is the fake exec time (2nd exec cycle)\n");
            printf("im executing %d\n",dec.opcode);
            executeFlag = 0;
        }
        if(decodeFlag==0 && f.instruction!=-1&&clockCycles%2==0){
            printf("here is the real decode time(1st decode cycle)\n");
            decode();
            f.instruction = -1;
            decodeFlag = 1;
        }
        else if(decodeFlag==1 && f.instruction==-1&&clockCycles%2==1){
            printf("here is the fake decode time (2nd decode cycle)\n");
            printf("im decoding\n");
            decodeFlag = 0;
            executeFlag=0;
        }
        if((clockCycles%2==1) && (pc < memindexInstr)){
            fetch();
        }
        if(branchflag==1&& executeFlag==0){
                f.instruction=-1;
                dec.opcode=-1;//not sure
                branchflag=0;
                memafterbranch=1;
            }
        printf("current j is %d\n",j);
        printf("current clock cycle %i\n",clockCycles);
        
        printf("------------printing registers----------------\n");
        for(int i=0;i<32;i++){
            if(registerFile[i]==0)
                printf("Register %i value : %i\n",i,registerFile[i]);
            else
            printf("Register %i value : %i <---\n",i,registerFile[i]);
        }
        printf("------------end of a cycle----------------\n");
        if(j==0)
            break;
        clockCycles++;
    }
    printf("pc: %i\n",pc);
}


void saveInMemory(char instruction[]){
    int i=0;
    char operator[12]="";
    int32_t instmaking=0 ;
    char forreg[12]="";
    char *ptr = instruction;

    while (*ptr != ' ') {
        strncat(operator, ptr, 1);
        ptr++;
    }

    if(strcmp(operator,"ADD") == 0){
        instmaking=0b00000000000000000000000000000000;
        //printf("%i\n",instmaking>>28);
        ptr=ptr+2;
        while (*ptr !=' ') {
            strncat(forreg, ptr, 1);
            ptr++;
        }
        int num = atoi(forreg);
        //printf("%i\n",num);
        instmaking |= (num & 0b11111) << 23;
        //printf("%i\n",instmaking);
        strcpy(forreg,"");
        ptr=ptr+2;
        while (*ptr !=' ') {
            strncat(forreg, ptr, 1);
            ptr++;
        }
        num = atoi(forreg);
        //printf("%i\n",num);
        instmaking |= (num & 0b11111) << 18;
        strcpy(forreg,"");
        ptr=ptr+2;
        while (*ptr != '\0' && *ptr != ' ') {
            strncat(forreg, ptr, 1);
            ptr++;
        }
        num = atoi(forreg);
        //printf("%i\n",num);
        instmaking |= (num & 0b11111) << 13;
    }
    if(strcmp(operator,"SUB") == 0 ){
        instmaking=0b00010000000000000000000000000000;
        //printf("%i\n",instmaking>>28);
        ptr=ptr+2;
        while (*ptr !=' ') {
            strncat(forreg, ptr, 1);
            ptr++;
        }
        int num = atoi(forreg);
        //printf("%i\n",num);
        instmaking |= (num & 0b11111) << 23;
        strcpy(forreg,"");
        ptr=ptr+2;
        while (*ptr !=' ') {
            strncat(forreg, ptr, 1);
            ptr++;
        }
        num = atoi(forreg);
        //printf("%i\n",num);
        instmaking |= (num & 0b11111) << 18;
        strcpy(forreg,"");
        ptr=ptr+2;
        while (*ptr != '\0' && *ptr != ' ') {
            strncat(forreg, ptr, 1);
            ptr++;
        }
        num = atoi(forreg);
        //printf("%i\n",num);
        instmaking |= (num & 0b11111) << 13;
    }
    if(strcmp(operator,"MUL") == 0){
        instmaking=0b00100000000000000000000000000000;
        //printf("%i\n",instmaking>>28);
        ptr=ptr+2;
        while (*ptr !=' ') {
            strncat(forreg, ptr, 1);
            ptr++;
        }
        int num = atoi(forreg);
        //printf("%i\n",num);
        instmaking |= (num & 0b11111) << 23;
        //printf("%i\n",instmaking);
        strcpy(forreg,"");
        ptr=ptr+2;
        while (*ptr !=' ') {
            strncat(forreg, ptr, 1);
            ptr++;
        }
        num = atoi(forreg);
        //printf("%i\n",num);
        instmaking |= (num & 0b11111) << 18;
        strcpy(forreg,"");
        ptr=ptr+2;
        while (*ptr != '\0' && *ptr != ' ') {
            strncat(forreg, ptr, 1);
            ptr++;
        }
        num = atoi(forreg);
        //printf("%i\n",num);
        instmaking |= (num & 0b11111) << 13;
    }
    if(strcmp(operator,"MOVI") == 0){
        instmaking=0b00110000000000000000000000000000;
        //printf("%i\n",instmaking>>28);
        ptr=ptr+2;
        while (*ptr !=' ') {
            strncat(forreg, ptr, 1);
            ptr++;
        }
        int num = atoi(forreg);
        //printf("%i\n",num);
        instmaking |= (num & 0b11111) << 23;
        //printf("%i\n",instmaking);
        strcpy(forreg,"");
        ptr=ptr+1;
        while (*ptr != '\0' && *ptr != ' ') {
            strncat(forreg, ptr, 1);
            ptr++;
        }
        num = atoi(forreg);
        //printf("%i\n",num);
        instmaking |= (num & 0b111111111111111111);
    }
    if(strcmp(operator,"JEQ") == 0){
        instmaking=0b01000000000000000000000000000000;
        //printf("%i\n",instmaking>>28);
        ptr=ptr+2;
        while (*ptr !=' ') {
            strncat(forreg, ptr, 1);
            ptr++;
        }
        int num = atoi(forreg);
        //printf("%i\n",num);
        instmaking |= (num & 0b11111) << 23;
        //printf("%i\n",instmaking);
        strcpy(forreg,"");
        ptr=ptr+2;
        while (*ptr !=' ') {
            strncat(forreg, ptr, 1);
            ptr++;
        }
        num = atoi(forreg);
        //printf("%i\n",num);
        instmaking |= (num & 0b11111) << 18;
        strcpy(forreg,"");
        ptr=ptr+1;
        while (*ptr != '\0' && *ptr != ' ') {
            strncat(forreg, ptr, 1);
            ptr++;
        }
        num = atoi(forreg);
        //printf("%i\n",num);
        instmaking |= (num & 0b111111111111111111);
    }
    if(strcmp(operator,"AND") == 0){
        instmaking=0b01010000000000000000000000000000;
        //printf("%i\n",instmaking>>28);
        ptr=ptr+2;
        while (*ptr !=' ') {
            strncat(forreg, ptr, 1);
            ptr++;
        }
        int num = atoi(forreg);
        //printf("%i\n",num);
        instmaking |= (num & 0b11111) << 23;
        //printf("%i\n",instmaking);
        strcpy(forreg,"");
        ptr=ptr+2;
        while (*ptr !=' ') {
            strncat(forreg, ptr, 1);
            ptr++;
        }
        num = atoi(forreg);
        //printf("%i\n",num);
        instmaking |= (num & 0b11111) << 18;
        strcpy(forreg,"");
        ptr=ptr+2;
        while (*ptr != '\0' && *ptr != ' ') {
            strncat(forreg, ptr, 1);
            ptr++;
        }
        num = atoi(forreg);
        //printf("%i\n",num);
        instmaking |= (num & 0b11111) << 13;
    }
    if(strcmp(operator,"XORI") == 0){
        instmaking=0b01100000000000000000000000000000;
        //printf("%i\n",instmaking>>28);
        ptr=ptr+2;
        while (*ptr !=' ') {
            strncat(forreg, ptr, 1);
            ptr++;
        }
        int num = atoi(forreg);
        //printf("%i\n",num);
        instmaking |= (num & 0b11111) << 23;
        //printf("%i\n",instmaking);
        strcpy(forreg,"");
        ptr=ptr+2;
        while (*ptr !=' ') {
            strncat(forreg, ptr, 1);
            ptr++;
        }
        num = atoi(forreg);
        //printf("%i\n",num);
        instmaking |= (num & 0b11111) << 18;
        strcpy(forreg,"");
        ptr=ptr+1;
        while (*ptr != '\0' && *ptr != ' ') {
            strncat(forreg, ptr, 1);
            ptr++;
        }
        num = atoi(forreg);
        //printf("%i\n",num);
        instmaking |= (num & 0b111111111111111111);
    }
    if(strcmp(operator,"JMP") == 0){
        instmaking=0b01110000000000000000000000000000;
        //printf("%i\n",instmaking>>28);
        ptr=ptr+1;
        while (*ptr != '\0'&&*ptr !=' ') {
            strncat(forreg, ptr, 1);
            ptr++;
        }
        int num = atoi(forreg);
        num--;
        //printf("%i\n",num);
        instmaking |= (num & 0b1111111111111111111111111111) ;
        //printf("%i\n",instmaking);
        
    }
    if(strcmp(operator,"LSL") == 0){
        instmaking=0b10000000000000000000000000000000;
        ptr=ptr+2;
        while (*ptr !=' ') {
            strncat(forreg, ptr, 1);
            ptr++;
        }
        int num = atoi(forreg);
        instmaking |= (num & 0b11111) << 23;
        //printf("%i\n",instmaking);
        strcpy(forreg,"");
        ptr=ptr+2;
        while (*ptr !=' ') {
            strncat(forreg, ptr, 1);
            ptr++;
        }
        num = atoi(forreg);
        instmaking |= (num & 0b11111) << 18;
        strcpy(forreg,"");
        ptr=ptr+1;
        while (*ptr != '\0' && *ptr != ' ') {
            strncat(forreg, ptr, 1);
            ptr++;
        }
        num = atoi(forreg);
        instmaking |= (num & 0b1111111111111);
    }
    if(strcmp(operator,"LSR") == 0){
        instmaking=0b10010000000000000000000000000000;
        ptr=ptr+2;
        while (*ptr !=' ') {
            strncat(forreg, ptr, 1);
            ptr++;
        }
        int num = atoi(forreg);
        instmaking |= (num & 0b11111) << 23;
        strcpy(forreg,"");
        ptr=ptr+2;
        while (*ptr !=' ') {
            strncat(forreg, ptr, 1);
            ptr++;
        }
        num = atoi(forreg);
        instmaking |= (num & 0b11111) << 18;
        strcpy(forreg,"");
        ptr=ptr+1;
        while (*ptr != '\0' && *ptr != ' ') {
            strncat(forreg, ptr, 1);
            ptr++;
        }
        num = atoi(forreg);
        instmaking |= (num & 0b1111111111111);
    }
    if(strcmp(operator,"MOVR") == 0){
        instmaking=0b10100000000000000000000000000000;
        //printf("%i\n",instmaking>>28);
        ptr=ptr+2;
        while (*ptr !=' ') {
            strncat(forreg, ptr, 1);
            ptr++;
        }
        int num = atoi(forreg);
        //printf("%i\n",num);
        instmaking |= (num & 0b11111) << 23;
        //printf("%i\n",instmaking);
        strcpy(forreg,"");
        ptr=ptr+2;
        while (*ptr !=' ') {
            strncat(forreg, ptr, 1);
            ptr++;
        }
        num = atoi(forreg);
        //printf("%i\n",num);
        instmaking |= (num & 0b11111) << 18;
        strcpy(forreg,"");
        ptr=ptr+1;
        while (*ptr != '\0' && *ptr != ' ') {
            strncat(forreg, ptr, 1);
            ptr++;
        }
        num = atoi(forreg);
        //printf("%i\n",num);
        instmaking |= (num & 0b111111111111111111);
    }
    if(strcmp(operator,"MOVM") == 0){
        instmaking=0b10110000000000000000000000000000;
        //printf("%i\n",instmaking>>28);
        ptr=ptr+2;
        while (*ptr !=' ') {
            strncat(forreg, ptr, 1);
            ptr++;
        }
        int num = atoi(forreg);
        //printf("%i\n",num);
        instmaking |= (num & 0b11111) << 23;
        //printf("%i\n",instmaking);
        strcpy(forreg,"");
        ptr=ptr+2;
        while (*ptr !=' ') {
            strncat(forreg, ptr, 1);
            ptr++;
        }
        num = atoi(forreg);
        //printf("%i\n",num);
        instmaking |= (num & 0b11111) << 18;
        strcpy(forreg,"");
        ptr=ptr+1;
        while (*ptr != '\0' && *ptr != ' ') {
            strncat(forreg, ptr, 1);
            ptr++;
        }
        num = atoi(forreg);
        //printf("%i\n",num);
        instmaking |= (num & 0b111111111111111111);
    }
    instmaking &= 0xFFFFFFFF;
    memory[memindexInstr]=instmaking;
    memindexInstr++;          
}
void readFromFile(){
    FILE *file = fopen("test.txt", "r");
    if (file == NULL) {
        perror("an error happened while opening the file");
    }
    char line[200];
    while (fgets(line, sizeof(line), file) != NULL) {
        // Remove trailing newline character, if it exists
        char *newline = strchr(line, '\n');
        if (newline != NULL) {
            *newline = '\0';
        }
        // Check if the line is empty
        if (strcmp(line, "") == 0) {
            continue; // Skip empty lines
        }        
        //printf("%s\n", line);
        saveInMemory(line);
    }
    fclose(file);
}
void init() {
    f.instruction=-1;
    dec.address=-1;
    dec.opcode=-1;
    dec.r1=-1;
    dec.r2=-1;
    dec.r3=-1;
    mem.memloc=-1;
    mem.opcode=-1;
    mem.r1=-1;
    mem.valuefrommemory=-1;
    exec.opcode=-1;
    exec.r1=-1;
    exec.result=-1;
    for(int i=0;i<32;i++){
        registerFile[i]=0;
    }
    readFromFile(); // Read instructions from file and save them into memory
    pipelining();
    printf("%d\n",memindexInstr);
    totalclkcycles=7+((memindexInstr-1)*2);
    
}
int main() {
    init(); // Initialize memory and read instructions from file
    printf("our cycles are: %d\n",clockCycles);
    printf("total cycles are: %d\n",totalclkcycles);   
    // for(int i=0;i<2048;i++){
    //     printf("memory content at indec %i is %i \n",i,memory[i]);
    // }
    return 0;
}