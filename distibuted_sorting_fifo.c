#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>


///////////////////
/* Declarations */
/////////////////

// Constants
#define NODE_NUM 4
#define MAX_NUM 1000
#define EACH_NODE_SORTING_NUM 3

// Structures
struct node {
  int id;
  int num[EACH_NODE_SORTING_NUM];
  int fifods[4];
  int flag;
  int buff;
};

// Functions
void generate_new_process (int);
void prepare_fifos (void);
void unlink_fifos (void);
void construct_node (struct node *, int);
void parent_node_process (int);
void child_node_process (int);
void create_sequential_num_array (int *);
void shuffle_array (int *);


//////////////////////
/* Global variables */
////////////////////

int rand_num_array[MAX_NUM];


///////////
/* Main */
/////////

int main (void) {

  // rand_num_array is declared as a global variable
  create_sequential_num_array(rand_num_array);
  shuffle_array(rand_num_array);
  prepare_fifos();
  generate_new_process(NODE_NUM - 1);
  unlink_fifos();

  return 0;
}


////////////////
/* Functions */
//////////////

void generate_new_process (int node_id) {
  pid_t pid;
  pid = fork();
  if (pid == 0){
    child_node_process(node_id);
  } else {
    parent_node_process(node_id);
  }
}


void prepare_fifos (void) {
  char fifo_name[64];
  int i;
  for (i = 0; i < (NODE_NUM - 1) * 2; i++){
    sprintf(fifo_name, "%d.fifo", i);
    mkfifo(fifo_name, 0600);
  }
}


void unlink_fifos (void) {
  char fifo_name[64];
  int i;
  for (i = 0; i < (NODE_NUM - 1) * 2; i++){
    sprintf(fifo_name, "%d.fifo", i);
    unlink(fifo_name);
  }
}


void construct_node (struct node *node, int node_id) {
  node->id = node_id;

  int i;
  int sp = EACH_NODE_SORTING_NUM * node_id;
  for (i = 0; i < EACH_NODE_SORTING_NUM; i++){
    node->num[i] = rand_num_array[sp + i];
  }

  char fifo_name[16];
  if (node_id == 0) {
    // top node
    sprintf(fifo_name, "%d.fifo", 0);
    node->fifods[2] = open(fifo_name, O_RDONLY);
    sprintf(fifo_name, "%d.fifo", 1);
    node->fifods[3] = open(fifo_name, O_WRONLY);
  } else if (node_id == NODE_NUM - 1) {
    // bottom node
    sprintf(fifo_name, "%d.fifo", (NODE_NUM - 1) * 2 - 2);
    node->fifods[0] = open(fifo_name, O_WRONLY);
    sprintf(fifo_name, "%d.fifo", (NODE_NUM - 1) * 2 - 1);
    node->fifods[1] = open(fifo_name, O_RDONLY);
  } else {
    // middle node
    sp = (node_id - 1) * 2;
    sprintf(fifo_name, "%d.fifo", sp);
    node->fifods[sp] = open(fifo_name, O_WRONLY);
    sprintf(fifo_name, "%d.fifo", sp + 1);
    node->fifods[sp + 1] = open(fifo_name, O_RDONLY);
    sprintf(fifo_name, "%d.fifo", sp + 2);
    node->fifods[sp + 2] = open(fifo_name, O_RDONLY);
    sprintf(fifo_name, "%d.fifo", sp + 3);
    node->fifods[sp + 3] = open(fifo_name, O_WRONLY);
    printf("fifodssp %d %d\n",node->fifods[sp], sp);
    printf("fifodssp %d %d\n",node->fifods[sp+1], sp+1);
    printf("fifodssp %d %d\n",node->fifods[sp+2], sp+2);
    printf("fifodssp %d %d\n",node->fifods[sp+3], sp+3);
  }
}


void parent_node_process (int node_id) {
  if (node_id == 0) {
    //printf("All processes have been created\n");
  } else {
    generate_new_process(node_id - 1);
  }

  int status;
  wait(&status);
}


void child_node_process (int node_id) {
  struct node node;
  construct_node(&node, node_id);
  //printf("Node %d pid: %d\n", node_id, getpid());
  int i;
  for (i = 0; i < EACH_NODE_SORTING_NUM; i++) {
    //printf("randnum_%d %d\n", i, node.num[i]);
  }
  if (node_id == 0) {
    // top node
    node.buff = node.num[0];
    write(node.fifods[3], &node.buff, sizeof(node.buff));
    printf("Node %d sending_num: %d\n", node_id, node.buff);
  } else if (node_id == NODE_NUM - 1) {
    // bottom node
    read(node.fifods[1], &node.buff, sizeof(node.buff));
    printf("Received_num_from_top_node: %d\n", node.buff);
  } else {
    // middle node
    read(node.fifods[1], &node.buff, sizeof(node.buff));
    //printf("%d readfifo%d\n", node.buff, node.fifods[1]);
    write(node.fifods[3], &node.buff, sizeof(node.buff));
    //printf("%d writefifo%d\n", node.buff, node.fifods[3]);
  }
}


void create_sequential_num_array(int *array) {
  int i;
  for (i = 0; i < MAX_NUM; i++){
    array[i] = i + 1;
  }
}


void shuffle_array (int *array) {
  int i;
  int tmp;
  srand((unsigned)time(NULL));
  for (i = MAX_NUM - 1; i > 0; i--) {
    int j = rand() % i;
    tmp = array[i];
    array[i] = array[j];
    array[j] = tmp;
  }
}
