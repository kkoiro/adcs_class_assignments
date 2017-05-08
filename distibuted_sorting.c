#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>


///////////////////
/* Declarations */
/////////////////

// Constants
#define NODE_NUM 3
//#define MAX_NUM 1000
#define MAX_NUM 10
#define EACH_NODE_SORTING_NUM 10

// Structures
struct node {
  int id;
  int num[10];
  int flag;
};

// Functions
int generate_node (int);
void top_node_process (int);
void chain_node_process (int);
void shuffle_array (int *);


//////////////////////
/* Global variables */
////////////////////

int counter = 0;


///////////
/* Main */
/////////

int main (void) {

  int shuffled_array[MAX_NUM];
  shuffle_array(shuffled_array);
  int i;
  for (i = 0; i < sizeof(shuffled_array) / sizeof(int); i++) {
    printf("%d\n", shuffled_array[i]);
  }
  generate_node(NODE_NUM);

  return 0;
}


////////////////
/* Functions */
//////////////

int generate_node (int node_id) {
  if (node_id > 1) {
    pid_t pid;
    int fd[2];

    pipe(fd);
    pid = fork();
    if (pid == 0){
      chain_node_process(node_id);
    } else {
      top_node_process(node_id);
    }
  } else {
    puts("All processes have been created.");
  }
  return 0;
}

void chain_node_process (int node_id) {
  node_id--;
  printf("Node %d pid: %d\n", node_id, getpid());
  generate_node(node_id);
}

void top_node_process (int node_id) {
  if (node_id == NODE_NUM) { // top node
    printf("Node %d pid: %d\n", node_id, getpid());
  }
  int status;
  wait(&status);
}

void shuffle_array (int *array) {
  int i;
  for (i = 0; i < MAX_NUM; i++){
    array[i] = i + 1;
  }

  int j;
  int tmp;
  srand((unsigned)time(NULL));
  for (j = MAX_NUM - 1; i > 0; i--) {
    int k = rand() % j;
    tmp = array[j];
    array[j] = array[k];
    array[k] = tmp;
  }
}
