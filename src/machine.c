#include "ijvm.h"
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
// #include "structs.h"
// #include "linkedLists.c"
//  #include "globalVars.c"
typedef struct stack
{
  word_t *stack;
  int sp;
} stack_t;

typedef struct node {
   word_t data;
   int key;
   struct node *next;
}node_t;

typedef struct frame {
   int key;
   int oldpc;
   int oldsp;
   int loadingCount;
   struct frame *nextF;
}frame_t;

int8_t x, y;
int loadingCount;
byte_t *text;
byte_t *temp;
word_t mn;
word_t co;
word_t cs;
word_t *cp;
word_t to;
word_t ts;
int pc=0;
byte_t opCode;
FILE *f;
FILE *out;
FILE *in;
stack_t s;
frame_t frame;
byte_t sig;
signed short cArg;
unsigned short ins;
unsigned int methodIndex;
struct node *head = NULL;
struct node *current = NULL;
struct frame *headF = NULL;
struct frame *currentF = NULL;
struct node *local;
struct frame *localF;
word_t result;
unsigned short nrArg;
unsigned short nrVars;
int c;
int oldpc;
int oldsp;
unsigned short wideArg;
int count=0;
bool wide;
bool invalid;

void printList() {
   struct node *ptr = head;
   printf("\n[ ");

   while(ptr != NULL) {
      printf("(%d,%x) ",ptr->key,ptr->data);
      ptr = ptr->next;
   }
	
   printf(" ]");
}
bool isEmpty() {
   return head == NULL;
}
bool isEmptyFrame() {
   return headF == NULL;
}
struct node* deleteFirst() {
   struct node *tempLink = head;
   head = head->next;
   return tempLink;
}
void insertFirst(int key, word_t data) {
  struct node *link = (struct node*) malloc(sizeof(struct node));
   link->key = key;
   link->data = data;
   link->next = head;
	
   head = link;
}

struct frame* deleteFrame() {

   struct frame *tempLinkF = headF;
   headF = headF->nextF;
   return tempLinkF;
}
void insertFrame(int key, int oldpc) {
  struct frame *linkF = (struct frame*) malloc(sizeof(struct frame));
   linkF->key = key;
   linkF->oldpc = oldpc;
   linkF->nextF = headF;
   headF = linkF;
}
struct frame* get_frame(int key) {
   currentF = headF;

   if(headF == NULL) {
      return NULL;
   }

   while(currentF->key != key) {
	
      if(currentF->nextF == NULL) {
         return NULL;
      } else {
         currentF = currentF->nextF;
      }
   }      
	
   return currentF;
}
struct node* get_node(int key) {

   current = head;

   if(head == NULL) {
      return NULL;
   }

   while(current->key != key) {
	
      if(current->next == NULL) {
         return NULL;
      } else {
         current = current->next;
      }
   }      
	
   return current;
}
word_t get_local_variable(int i){
 
  return get_node(i)->data;
}
word_t pop()
{
  assert(s.sp > 0);
  printf("POPPED :%X\n",s.stack[s.sp--]);
  s.sp++;
  printf("TOS AFTER POP :%X\n",tos());
  return s.stack[s.sp--];
}
word_t tos(){

  assert(s.sp > 0);
  return (int8_t)s.stack[s.sp];
}
void push(word_t n)
{
  printf("sp: %x\n",s.sp);
  printf("sz: %x\n",stack_size());
  assert(s.sp < stack_size() );
  s.stack[++s.sp] =n;
  printf("pushed: %x\n",n);
}
int stack_size()
{
  return sizeof(s.stack);
}
static uint32_t swap_uint32(uint32_t num);

word_t readW(word_t *key, FILE *f)
{
  fread(&key, sizeof(word_t), 1, f);
  key = swap_uint32(key);
  // printf("%X\n",key);
  return key;
}

int init_ijvm(char *binary_file)
{
 
  FILE *f = fopen(binary_file, "r");
  in=stdin;
  out=stdout;
  pc=0;
  wide=false;
  if (f == NULL)
  {
    perror("Failed");
    return -1;
  }

  readW(mn, f);
  readW(co, f);

  fread(&cs, sizeof(word_t), 1, f);
  cs = swap_uint32(cs);
  // printf("%x\n",cs);
  cp = malloc(cs * sizeof(byte_t));
  fread(cp, sizeof(word_t), cs / 4, f);

  for (int i = 0; i < cs / 4; i++)
  {
    //  printf("%x\n",cp[i]);
  }

  readW(to, f);

  fread(&ts, sizeof(word_t), 1, f);
  ts = swap_uint32(ts);
  // printf("%x\n",ts);
  text = malloc(ts * sizeof(byte_t));
  fread(text, sizeof(byte_t), ts, f);
  for (int i = 0; i <= ts; i++)
  {
    //  printf("%x\n",text[i]);
  }
  s.stack = malloc(ts * sizeof(word_t));
  insertFrame(count,pc);
  
  return 0;
}

void destroy_ijvm(void)
{
  pc=0;
  s.sp=0;
  count=0;
  in=stdin;
  out=stdout;
  struct node *head=NULL;
  struct node *curent=NULL;
  free(s.stack); 
  free(text);
  free(cp);
    while(!isEmpty()) {            
        struct node *temp = deleteFirst();
    } 
    while(!isEmptyFrame()){
      struct frame *tempf = deleteFrame();
    }
  printf("Delete done");
}

byte_t *get_text()
{
  return text;
}
static uint32_t swap_uint32(uint32_t num)
{
  return ((num >> 24) & 0xff) | ((num << 8) & 0xff0000) | ((num >> 8) & 0xff00) | ((num << 24) & 0xff000000);
}

int get_program_counter(void)
{
  return pc;
}

int text_size(void)
{
  return ts;
}

void add (){
    x = tos();
    pop();
    y = tos();
    pop();
    push((x + y));
}
void swap(){
    x = tos();
    pop();
    y = tos();
    pop();
    push(x);
    push(y);
}
void ifeq(){
  x=tos();
  pop();
  if(x==0){
    
    goTo();
  }
  else{
    pc+=2;
  }

}
void icmpeq(){
  x=tos();
   pop();
   y=tos();
   pop();

  if(y==x){
    goTo();
  }
  else{
     pc+=2;   
  }
}
void iflt(){
  x=tos();
  pop();
  if(x<0){
    goTo();
  }
  else{
    pc+=2;
  }

}
void goTo(){
   cArg=text[pc]<<8|text[pc+1] ;
   pc=pc+cArg-1;
}
void iOr(){
   x = tos();
    pop();
    y = tos();
    pop();
    push((x | y));
}
void sub(){
  x = tos();
    pop();
    y = tos();
    pop();
    int z=y-x;
    push((int8_t)z);
    
}
void and(){
    x = tos();
    pop();
    y = tos();
    pop();
    push((x & y));
}
void outF(){
  x = tos();
  if(out!=NULL){
    fputc(x,out);
    // fwrite(&x,1,sizeof(byte_t),out);
  }
  else{
    fputc(x,stdout);
    // fwrite(&x,1,sizeof(byte_t),stdout);
  }
  pop();
 
}
void ldc(){
    ins=text[pc]<<8|text[pc+1] ;
    push(get_constant(ins));
    pc=pc+2;
}
word_t get_constant(int i){
  return swap_uint32(cp[i]);
}
void istore(int i){
    insertFirst(i,tos());
    printList();
    pop();
    pc++;
}
void ireturn(){
  
   for(int i=nrArg-1;i>0;i--){ 
               
        struct node *temp = deleteFirst();
        printf("\nDeleted value:");
        printf("(%d,%d) ",temp->key,temp->data);
    } 
    struct frame *ftemp =deleteFrame();
    for(int i=0;i<loadingCount+1;i++){
      pop();
    }
    // printf("(%d,%d) ",ftemp->key,ftemp->oldpc);
    pc=ftemp->oldpc;
    // s.sp=ftemp->oldsp;
     printf("%d is sp\n",s.sp);
    
}
void invokevirtual(){

    count=count+1;
    methodIndex=text[pc+1];
    printf("METHODINDEX %x",methodIndex);
    oldsp=s.sp;
    printf("%d is OLDSP\n",oldsp);
    oldpc=pc+2;
    insertFrame(count,oldpc);
    get_frame(count)->oldsp=oldsp;
    get_frame(count)->loadingCount=loadingCount;
    loadingCount=0;
    result=get_constant(methodIndex);
    pc=result;

    nrArg=text[pc+1];
    // nrArg=swap_uint32(nrArg);
    pc=pc+1;
    nrVars=text[pc+2];
    // nrVars=swap_uint32(nrVars);
    pc=pc+3;

    if(nrArg>1){ 
      for(int i=nrArg-1;i>0;i--){ 
        insertFirst(i,tos());
        pop();
      }
    }
}
void iinc(){
  local=get_node(text[pc]);
    result=get_local_variable(text[pc])+(int8_t)text[pc+1];
    local->data=result;
    pc+=2;
}
byte_t get_instruction(void){
  return text[pc];
}
bool step()
{
  opCode = text[pc];
  pc++;
  
  switch (opCode)
  {
  case OP_IRETURN:
    printf("IRETURN\n");
    x=tos();
    ireturn();
      push(x);
      
     printf("%d is sp\n",s.sp);
    break;

  case OP_INVOKEVIRTUAL:
    printf("INVOKEVIRTUAL\n");
    invokevirtual();
    printf("nr args %d nr vars %d\n",nrArg,nrVars);
    break;

  case OP_POP:
    printf("POP\n");
    pop();
    break;

  case OP_IINC:
    printf("IINC\n");
    iinc();
    break;

  case OP_SWAP:
    printf("SWAP\n");
    swap();
    break;

  case OP_ILOAD:
    printf("ILOAD \n");
    loadingCount++;
    if(wide){
      // cArg=swap_uint32(cArg);
      wideArg=(int16_t)((text[pc]<<8)|text[pc+1]);
      push(get_local_variable(wideArg));
       pc=pc+2;
    }
    else{
      push(get_local_variable(text[pc]));
     pc++;
    }
    
    break;

  case OP_ISTORE:
    printf("ISTORE \n");
    if(wide){
      // cArg=swap_uint32(cArg);
      wideArg=(int16_t)((text[pc]<<8)|text[pc+1]);

      istore(wideArg);
       pc=pc+1;
    }
    else{
    istore(text[pc]);
    }
    break;

  case OP_LDC_W:
    printf("LDC_W \n");
    ldc();
    break;

  case OP_NOP:
    printf("NOP \n");
    break;
  
  case OP_BIPUSH:

    printf("BIPUSH \n");
    push(((int8_t)text[pc]));
     pc++;
    break;

  case OP_WIDE:
    printf("WIDE\n");
    wide=true;
    step();
    wide=false;
    break;

  case OP_IADD:
    printf("IADD \n");
    add();
    break;

  case OP_IFLT:
    printf("IFLT \n");
    iflt();
    break;

  case OP_GOTO:
    printf("GOTO \n");
    goTo();
    break;

  case OP_DUP:
    printf("DUP\n");
    x = tos();
    push(((int8_t)x));
    break;

  case OP_ERR:
    printf("Error, halting the simulator...");
    exit(0);
    return false;

  case OP_HALT:
    printf("HALT\n");
    return false;

  case OP_IOR:
    printf("OR\n");
    iOr();
    break;

  case OP_ISUB:
    printf("SUB\n");
    sub();
    break;

  case OP_IAND:
    printf("AND\n");
    and();
    break;

  case OP_IFEQ:
    printf("IFEQ\n");
    ifeq();
    break;
  case OP_ICMPEQ:
    printf("ICMEQ\n");
    icmpeq();
    break;
  case OP_OUT:
    printf("OUT\n");
    outF();
    break;
  case OP_IN:
    printf("IN\n");
    c=getc(in);
    if(c==EOF){
      push(0);
    }
    else{
    push((int8_t)c);
    }
    break;
  default:
    invalid=true;
    break;
  }
  printf("----------STACK AFTER STEP: \n");
  for(int i=0;i<stack_size();i++){
    printf("%x ,",s.stack[i]);
  }
  printf("\n----------\n");
  invalid=false;
  return true;
}
void run()
{
  while(step());
}

word_t *get_stack(void){
  return s.stack;
}

void set_input(FILE *fp)
{
  in=fp;
}

void set_output(FILE *fp)
{
  out=fp;
}

bool finished(void){

  if(pc==ts){
    return true;
  }
  else if(opCode==OP_HALT||opCode==OP_ERR){
    return true;
  }
  else if(invalid){
    return true;
  }
  return false;
}