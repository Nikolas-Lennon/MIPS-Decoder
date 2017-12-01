// Nikolas Lennon
// Copyright 2017

#include <stdio.h>
#include <string.h>
#include <iostream>
#include <map>
#include <vector>
#include <deque>


//------------GLOBAL VARIABLES------------------//
#define MAXLINE 100
#define MAXREG 5
#define IMMEDIATE 16
unsigned int dCount = 0; //data directive counts
unsigned int tCount = 0; //text directive counts
bool isTextField = false;
bool isDataField = false;
std::map <std::string, unsigned int>label; // MAP STRUCTURE TO STORE LABELS TO
                                           // ACCESS WITH
// BRANCHING AND J INSTRUCTIONS
std::deque<std::string> pass1;
//-----------END VARIABLES---------------------//


//------------HELPER FUNCTIONS-----------------//
int toReg(char* registerName);
int toOper(char* operation);
void Count();
int Immed(char* str);
int aLabel(char* str);
//-------------END FUNCTIONS--------------------//



//------------UNION STRUCTURES-----------------//
//R-type Union structure(rd, rs, rt) (Little endian)
union {
  unsigned int x;
  struct {
    unsigned int funct : 6;
    unsigned int shamt : 5;
    unsigned int rd : 5;
    unsigned int rt : 5;
    unsigned int rs : 5;
    unsigned int oper : 6;
  } rstyle;
}rMap;


//R-Type Union Structure (syscall) (Little endian)
union {
  unsigned int a;
  struct {
    unsigned int oper4 : 32;
  }rstyle4;
}sysMap;


//I-type Union structure(rs, rt) (Little endian)
union {
  unsigned int y;
  struct {
    unsigned int imm : 16;
    unsigned int rt2 : 5;
    unsigned int rs2 : 5;
    unsigned int oper2 : 6;
  } istyle;
}iMap;

union {
  unsigned int b;
  struct {
    unsigned int imm3 : 16;
    unsigned int rt3 : 5;
    unsigned int rs3 : 5;
    unsigned int oper4 : 6;
  }bstyle;
}iMap2;

//J-type Union structure(oper, imm) (Little endian)
union {
  unsigned int z;
  struct {
    unsigned int imm2 : 26;
    unsigned int oper3 : 6;
  } jstyle;
}jMap;
//----------------------END STRUCTURES---------------//

//------------------------PROGRAM-------------------//
int main() {

  //--------------LOCAL VARIABLES----------------//

  char line[MAXLINE] = { 0 };
  char oper[MAXLINE] = { 0 };
  char rd[MAXREG];
  char rs[MAXREG];
  char rt[MAXREG];
  unsigned int immed[IMMEDIATE];

  char directive[MAXLINE] = { 0 };
  char instructiontypes[MAXLINE] = { 0 };
  char info[MAXLINE] = { 0 };
  char* r1delimiters = "%s $%[^,],$%[^,],$%s";
  char* r2delimiters = "%s $%[^,],$%s";
  char* r3delimiters = "%s";
  char* idelimiters = "%s $%[^,],$%[^,],%u";
  char offset[MAXLINE];
  char labels[MAXLINE];
  unsigned int Value = 0;

  //--------------END VARIABLES-----------------//


  // first pass through
  while (fgets(line, MAXLINE, stdin)) {

    //Gather instructions for the first pass to parse in the second pass
    if (sscanf(line, "#%[^\n]s", instructiontypes) == 1) {
      std::cout << instructiontypes;
      continue; //continue looking through the program ignoring comments
    }
    //gather lines that are directives (.word,.text.space,.
    else if (sscanf(line, "[^\t].%s %u", directive, &Value) == 2)
    {
      if (strcmp(directive, "word") == 0) {
        std::cout << directive << " " << &Value; //gather the instruction
        ++dCount;//count the data segment
        pass1.push_back(line);//push the line to the stack
        continue; //keep doing this
      }
      else if (strcmp(directive, "space") == 0)
      {
        std::cout << directive; 
        for (size_t i = 0; i < Value; i++) {
          ++dCount; //keep counting the space directives
        }
        pass1.push_back(line); //push the line to the stack
        continue; //keep doing this
      }

    }
    //push  all data and text directives to the stack
    else if (sscanf(line, " .%s", directive) == 1) {
      if (strcmp(directive, "text") == 0) {
        isTextField = true;
        isDataField = false;
        pass1.push_back(line);
        continue;
      }
      //push any information in the data section
      else if (strcmp(directive, "data") == 0) {
        isDataField = true;
        isTextField = false;
        pass1.push_back(line);
        continue;
      }
    }
    //push back all information on the stack that is a label
    else if (sscanf(line, "%[^:]: %[^\n]s", labels, info) == 2) {
      //push back information that is "label:"
      pass1.push_back(labels);

      //check the address of the label
      if (isTextField && !isDataField) {
        label[labels] = (tCount - 1);
      }

      //pass back all the rest of the information after the "colon"
      pass1.push_back(info);
      Count();
    }
    else {
      //Count any other text lines that are not in the above format
      Count();
      pass1.push_back(line);
    }
  }

  //print out the way Dr. Uh wants to count all text 
  //and data amounts of instructions
  printf("%u %u\n", tCount, dCount);



  //second pass through
  //PRINT OUT ALL THE INSTRUCTIONS IN HEXADECIMAL FORMATS
  while (!pass1.empty()) {

    if (sscanf(pass1.front().c_str(), " .%s\n", directive) == 1)
    {
      if (strncmp(directive, "text", 4) == 0)
      {
        isTextField = true; //THIS IS A TEXT FIELD
        pass1.pop_front();
        continue; //CONTINUE ON THE POPPING OFF OF INSTRUCTIONS
      }
      else if (strncmp(directive, "data", 4) == 0)
      {
        isDataField = true; //THIS IS A DATA FIELD
        pass1.pop_front();
        continue; //CONTINUE ON THE POPPING OFF OF INSTRUCTIONS
      }
    }
    else if (sscanf(pass1.front().c_str(), " .%s %u\n", directive, &Value) == 2) {
      if (strcmp(directive, "word") == 0) {
        printf("%08x", Value);
        continue;
      }
      else if (strcmp(directive, "space") == 0) {
        for (int i = 0; i < Value; i++) {
          printf("%08x", Value);
        }
        continue;
      }
    }

    // R-Type primary instructions(rd, rs, rt)
    if (sscanf(pass1.front().c_str(), r1delimiters, oper, rd, rs, rt) == 4) {

      int shamt = 0;
      int funct = 0;

      rMap.rstyle.rd = toReg(rd);
      rMap.rstyle.rs = toReg(rs);
      rMap.rstyle.rt = toReg(rt);
      rMap.rstyle.funct = toOper(oper);
      rMap.rstyle.oper = funct;
      rMap.rstyle.shamt = shamt;

      //printf("rformat\n");
      printf("%08x\n", rMap.x);

    }
    // I-Type Instructions (rs, rt, immed)
    else if (sscanf(pass1.front().c_str(), idelimiters, oper, rt, rs, &immed) == 4) {

      iMap.istyle.imm = *immed;
      iMap.istyle.rs2 = toReg(rs);
      iMap.istyle.rt2 = toReg(rt);
      iMap.istyle.oper2 = toOper(oper);

      //printf("iformat\n");
      printf("%08x\n", iMap.y);

    }
    else if (sscanf(pass1.front().c_str(), "%s $%[^,],$%[^,],%s", oper, rs, rt, offset) == 4) {

      iMap2.bstyle.imm3 = Immed(offset);
      iMap2.bstyle.rs3 = toReg(rs);
      iMap2.bstyle.rt3 = toReg(rt);
      iMap2.bstyle.oper4 = toOper(oper);

      printf("%08x\n", iMap2.b);
    }
    // R-Type secondary instructions (rs, rt)
    else if (sscanf(pass1.front().c_str(), r2delimiters, oper, rs, rt) == 3) {

      int funct = 0;
      int shamt = 0;
      char* rd = "zero";

      rMap.rstyle.rs = toReg(rs);
      rMap.rstyle.rt = toReg(rt);
      rMap.rstyle.funct = toOper(oper);
      rMap.rstyle.oper = funct;
      rMap.rstyle.shamt = shamt;
      rMap.rstyle.rd = toReg(rd); // returns 0

      //printf("secondaryrformat\n");
      printf("%08x\n", rMap.x);

    }
    // R-type Tertiary instruction (syscall)
    else if (sscanf(pass1.front().c_str(), r3delimiters, oper) == 1) {

      sysMap.rstyle4.oper4 = toOper(oper);

      printf("%08x\n", sysMap.a);
    }
    // J-Type Instructions
    else if (sscanf(pass1.front().c_str(), "%s %s", oper, offset) == 2) {
      jMap.jstyle.oper3 = toOper(oper);
      jMap.jstyle.imm2 = Immed(offset);

    }

    // Incorrect instruction format
    else {
      break;
    }
    pass1.pop_front();
    pass1.empty();
  }
  return 0;
}

//----------------FUNCTIONS---------------------//

int toReg(char * registerName)
{
  //Registers registerName;

  // ZERO REGISTER
  if (strcmp(registerName, "zero") == 0) {
    return 0;
  } // ASSEMBLER TEMP REGISTER
  else if (strcmp(registerName, "at") == 0) {
    return 1;
  }
  // SYSTEM REGISTERS
  else if (strcmp(registerName, "v0") == 0) {
    return 2;
  }
  else if (strcmp(registerName, "v1") == 0) {
    return 3;
  }
  // ARGUMENT REGISTERS
  else if (strcmp(registerName, "a0") == 0) {
    return 4;
  }
  else if (strcmp(registerName, "a1") == 0) {
    return 5;
  }
  else if (strcmp(registerName, "a2") == 0) {
    return 6;
  }
  else if (strcmp(registerName, "a3") == 0) {
    return 7;
  }
  // SAVE REGISTERS
  else if (strcmp(registerName, "s0") == 0) {
    return 16;
  }
  else if (strcmp(registerName, "s1") == 0) {
    return 17;
  }
  else if (strcmp(registerName, "s2") == 0) {
    return 18;
  }
  else if (strcmp(registerName, "s3") == 0) {
    return 19;
  }
  else if (strcmp(registerName, "s4") == 0) {
    return 20;
  }
  else if (strcmp(registerName, "s5") == 0) {
    return 21;
  }
  else if (strcmp(registerName, "s6") == 0) {
    return 22;
  }
  else if (strcmp(registerName, "s7") == 0) {
    return 23;
  }
  // TEMPORARY REGISTERS
  else if (strcmp(registerName, "t0") == 0) {
    return 8;
  }
  else if (strcmp(registerName, "t1") == 0) {
    return 9;
  }
  else if (strcmp(registerName, "t2") == 0) {
    return 10;
  }
  else if (strcmp(registerName, "t3") == 0) {
    return 11;
  }
  else if (strcmp(registerName, "t4") == 0) {
    return 12;
  }
  else if (strcmp(registerName, "t5") == 0) {
    return 13;
  }
  else if (strcmp(registerName, "t6") == 0) {
    return 14;
  }
  else if (strcmp(registerName, "t7") == 0) {
    return 15;
  }
  else if (strcmp(registerName, "t8") == 0) {
    return 24;
  }
  else if (strcmp(registerName, "t9") == 0) {
    return 25;
  }
  // GLOBAL POINTER
  else if (strcmp(registerName, "gp") == 0) {
    return 28;
  }
  // NOTHING FOUND
  else {
    return NULL;
  }
}

int toOper(char * operation)
{
  // R-TYPE (RD, RS, RT) INSTRUCTIONS
  if (strcmp(operation, "add") == 0) {
    return 32;
  }
  else if (strcmp(operation, "addu") == 0) {
    return 33;
  }
  else if (strcmp(operation, "subu") == 0) {
    return 35;
  }
  else if (strcmp(operation, "and") == 0) {
    return 36;
  }
  else if (strcmp(operation, "slt") == 0) {
    return 42;
  }
  else if (strcmp(operation, "or") == 0) {
    return 37;
  }
  // R-TYPE (RS, RT) INSTRUCTIONS
  else if (strcmp(operation, "mult") == 0) {
    return 24;
  }
  else if (strcmp(operation, "div") == 0) {
    return 26;
  }
  // R-TYPE (RD) INSTRUCTIONS
  else if (strcmp(operation, "mfhi") == 0) {
    return 16;
  }
  else if (strcmp(operation, "mflo") == 0) {
    return 18;
  }
  // I-TYPE (RS, RT, IMMED) INSTRUCTIONS
  else if (strcmp(operation, "addiu") == 0) {
    return 9;
  }
  else if (strcmp(operation, "beq") == 0) {
    return 4;
  }
  else if (strcmp(operation, "bne") == 0) {
    return 5;
  }
  else if (strcmp(operation, "lw") == 0) {
    return 35;
  }
  else if (strcmp(operation, "sw") == 0) {
    return 43;
  }
  else if (strcmp(operation, "syscall") == 0) {
    return 12;
  }
  // NOTHING FOUND
  else {
    return NULL;
  }
}

//FUNCTION TO COUNT ALL THE INSTRUCTIONS THAT ARE EITHER TEXT OR DATA

void Count() {

  if (isDataField && !isTextField) {
    ++dCount;
  }
  else if (isTextField && !isDataField) {
    ++tCount;
  }
}

//CHECK TO SEE IF A LINE IS A LABEL

int aLabel(char* str) {

  char t = *str;
  int i = 0;


  while (t != '\0') {
    if (t == ':') {
      return 1;
    }
    i++;
    t = *(str + i);
  }

  return 0;
}

//GRAB THE IMMEDIATE SECTION ADDRESS 
int Immed(char* str) {

  if (aLabel(str)) {
    return aLabel(str);
  }
  else
  {
    int immediate = atoi(str);
    return immediate;
  }

  return 0;
}
