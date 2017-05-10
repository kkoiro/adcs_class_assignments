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
  int buff;
};

// Functions
int generate_process (int);
void construct_nodes (struct node *);
void parent_node_process (int, int [2], int [2]);
void child_node_process (int, int [2], int [2]);
void shuffle_array (int *);


//////////////////////
/* Global variables */
////////////////////

struct node node[NODE_NUM];


///////////
/* Main */
/////////

int main (void) {

  construct_nodes(node);
  generate_process(NODE_NUM -1);

  return 0;
}


////////////////
/* Functions */
//////////////

int generate_process (int node_id) {
  pid_t pid;
  int pipe_c2p[2];
  int pipe_p2c[2];

  pipe(pipe_c2p);
  pipe(pipe_p2c);
  pid = fork();
  if (pid == 0){
    close(pipe_c2p[0]);
    close(pipe_p2c[1]);
    child_node_process(node_id, pipe_c2p, pipe_p2c);
  } else {
    close(pipe_c2p[1]);
    close(pipe_p2c[0]);
    parent_node_process(node_id, pipe_c2p, pipe_p2c);
  }
  return 0;
}


void construct_nodes (struct node *node) {
  int rand_num_array[MAX_NUM];
  shuffle_array(rand_num_array);

  int i;
  for (i = 0; i < NODE_NUM; i++){
    node[i].id = i;
    node[i].buff = 0;
    int sp = EACH_NODE_SORTING_NUM * i;
    int j;
    for (j = 0; j < EACH_NODE_SORTING_NUM; j++){
      node[i].num[j] = rand_num_array[sp + j];
    }
  }
}


void parent_node_process (int node_id, int pipe_c2p[2], int pipe_p2c[2]) {
  if (node_id == NODE_NUM - 1) {  // top node
    //printf("Node %d pid: %d\n", node_id, getpid());
    int i;
    for (i = 0; i < EACH_NODE_SORTING_NUM; i++) {
      printf("num %d, %d\n", node[node_id].num[i], i);
    }
    node[node_id].buff = node[node_id].num[0];
  }

  write(pipe_p2c[1], &node[node_id].buff, sizeof(node[node_id].buff));

  read(pipe_c2p[0], &node[node_id].buff, sizeof(node[node_id].buff));

  int status;
  wait(&status);
}


void child_node_process (int node_id, int pipe_c2p[2], int pipe_p2c[2]) {
  node_id--;
  printf("Node %d pid: %d\n", node_id, getpid());
  int i;
  for (i = 0; i < EACH_NODE_SORTING_NUM; i++) {
    printf("num %d, %d\n", node[node_id].num[i], i);
  }

  read(pipe_p2c[0], &node[node_id].buff, sizeof(node[node_id].buff));
  node[node_id].buff++;

  if (node_id == 0) {  // bottom node
    puts("All processes have been created.");
    printf("received_num %d\n", node[node_id].buff);
    node[node_id].buff = node[node_id].num[0];
    write(pipe_c2p[1], &node[node_id].buff, sizeof(node[node_id].buff));
  } else {
    generate_process(node_id);
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
