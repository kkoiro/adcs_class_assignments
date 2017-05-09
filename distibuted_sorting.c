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
#define MAX_NUM 1000
#define EACH_NODE_SORTING_NUM 5

// Structures
struct node {
  int id;
  int num[EACH_NODE_SORTING_NUM];
  int flag;
};

// Functions
int generate_node (int);
void parent_node_process (int);
void child_node_process (int);
void shuffle_array (int *);
void construct_node (struct node *, int);


//////////////////////
/* Global variables */
////////////////////

static int counter = 0;
static int rand_num_array[MAX_NUM];


///////////
/* Main */
/////////

int main (void) {

  shuffle_array(rand_num_array);
  int i;
  generate_node(NODE_NUM);

  return 0;
}


////////////////
/* Functions */
//////////////

int generate_node (int node_id) {
  pid_t pid;
  int fd[2];

  pipe(fd);
  pid = fork();
  if (pid == 0){
    child_node_process(node_id);
  } else {
    parent_node_process(node_id);
  }
  return 0;
}

void construct_node (struct node *node, int node_id) {
  node->id = node_id;
  int i;
  int sp = EACH_NODE_SORTING_NUM * (node_id - 1);
  for (i = 0; i < EACH_NODE_SORTING_NUM; i++){
    node->num[i] = rand_num_array[sp + i];
  }
}

void parent_node_process (int node_id) {
  if (node_id == NODE_NUM) {  // top node
    struct node node;
    construct_node(&node, node_id);
    printf("Node %d pid: %d\n", node.id, getpid());
    int i;
    for (i = 0; i < EACH_NODE_SORTING_NUM; i++) {
      printf("num %d, %d\n", node.num[i], i);
    }
  }
  int status;
  wait(&status);
}

void child_node_process (int node_id) {
  node_id--;
  struct node node;
  construct_node(&node, node_id);
  printf("Node %d pid: %d\n", node.id, getpid());
  int i;
  for (i = 0; i < EACH_NODE_SORTING_NUM; i++) {
    printf("num %d, %d\n", node.num[i], i);
  }
  if (node_id == 1) {  // bottom node
    puts("All processes have been created.");
  } else {
    generate_node(node_id);
  }
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
