#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>


///////////////////
/* Declarations */
/////////////////

// Constants
#define NODE_NUM 10
#define MAX_NUM 1000
#define MIN_NUM 1
#define EACH_NODE_SORTING_NUM 10

// Structures
struct node {
  int id;
  int num[EACH_NODE_SORTING_NUM];
  int fifods_r_from_upper;
  int fifods_w_to_upper;
  int fifods_r_from_lower;
  int fifods_w_to_lower;
  int buff;
};

// Functions
void generate_new_process (int);
void prepare_fifos (void);
void unlink_fifos (void);
void construct_node (struct node *, int);
void parent_node_process (int);
void child_node_process (int);
void top_node_process (struct node *);
void bottom_node_process (struct node *);
void middle_node_process (struct node *);
void create_sequential_num_array (int *);
void shuffle_array (int *);
int compare_int (const void *a, const void *b);
void swap_minnum_with_buff (int *, int *);
void swap_maxnum_with_buff (int *, int *);
void swap_int (int *, int *);
void display_int_array (int *, int);


//////////////////////
/* Global variables */
////////////////////

int rand_num_array[MAX_NUM];


///////////
/* Main */
/////////

int main (void) {

  create_sequential_num_array(rand_num_array);  // rand_num_array is declared as a global variable
  shuffle_array(rand_num_array);
  prepare_fifos();
  generate_new_process(0);
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
  int offset = EACH_NODE_SORTING_NUM * node_id;
  for (i = 0; i < EACH_NODE_SORTING_NUM; i++){
    node->num[i] = rand_num_array[offset + i];
  }

  char fifo_name[16];
  if (node_id == 0) {
    // top node
    sprintf(fifo_name, "%d.fifo", 0);
    node->fifods_r_from_lower = open(fifo_name, O_RDONLY);
    sprintf(fifo_name, "%d.fifo", 1);
    node->fifods_w_to_lower = open(fifo_name, O_WRONLY);
  } else if (node_id == NODE_NUM - 1) {
    // bottom node
    sprintf(fifo_name, "%d.fifo", (NODE_NUM - 1) * 2 - 2);
    node->fifods_w_to_upper = open(fifo_name, O_WRONLY);
    sprintf(fifo_name, "%d.fifo", (NODE_NUM - 1) * 2 - 1);
    node->fifods_r_from_upper = open(fifo_name, O_RDONLY);
  } else {
    // middle node
    offset = (node_id - 1) * 2;
    sprintf(fifo_name, "%d.fifo", offset);
    node->fifods_w_to_upper = open(fifo_name, O_WRONLY);
    sprintf(fifo_name, "%d.fifo", offset + 1);
    node->fifods_r_from_upper = open(fifo_name, O_RDONLY);
    sprintf(fifo_name, "%d.fifo", offset + 2);
    node->fifods_r_from_lower = open(fifo_name, O_RDONLY);
    sprintf(fifo_name, "%d.fifo", offset + 3);
    node->fifods_w_to_lower = open(fifo_name, O_WRONLY);
  }
}


void parent_node_process (int node_id) {
  if (node_id != NODE_NUM - 1) {
    generate_new_process(node_id + 1);
  }

  int status;
  wait(&status);
}


void child_node_process (int node_id) {
  struct node node;
  construct_node(&node, node_id);
  qsort((void *)node.num, EACH_NODE_SORTING_NUM, sizeof(int), compare_int);  // sort separately first

  // display
  int i;
  for (i = 0; i < EACH_NODE_SORTING_NUM; i++) {
    if (i == 0) {
      printf("Before sorting Node %d: %d, ", node.id, node.num[i]);
    } else if (i == EACH_NODE_SORTING_NUM - 1) {
      printf("%d\n", node.num[i]);
    } else {
      printf("%d, ", node.num[i]);
    }
  }

  if (node_id == 0) {
    // top node
    top_node_process(&node);
  } else if (node_id == NODE_NUM - 1) {
    // bottom node
    // passing num starts from bottom node
    node.buff = MAX_NUM + 1;  // dummy for having the same number of nums at each node
    swap_minnum_with_buff(node.num, &node.buff);  // insert dummy into num[max] and num[0] into buff
    bottom_node_process(&node);
  } else {
    // middle node
    middle_node_process(&node);
  }
}


void top_node_process (struct node *node) {  // finally top node is suppose to have "min, min+1, min+2..."
  read(node->fifods_r_from_lower, &node->buff, sizeof(node->buff));
  if (node->buff < node->num[EACH_NODE_SORTING_NUM - 1]) {  // received_num < having_max_num
    swap_maxnum_with_buff(node->num, &node->buff);
    write(node->fifods_w_to_lower, &node->buff, sizeof(node->buff));
    top_node_process(node);
  } else {  // done sorting
    // display
    printf("[top]After sorting Node %d: ", node->id);
    display_int_array(node->num, EACH_NODE_SORTING_NUM);

    write(node->fifods_w_to_lower, &node->buff, sizeof(node->buff));  // somehow this line is skipped when this is last node
  }
}


void bottom_node_process (struct node *node) {  // finally bottom node is suppose to have "...min-2, max-1, max"
  write(node->fifods_w_to_upper, &node->buff, sizeof(node->buff));
  read(node->fifods_r_from_upper, &node->buff, sizeof(node->buff));
  if (node->buff > node->num[0]) {  // received_num > having_min_num
    swap_minnum_with_buff(node->num, &node->buff);
    bottom_node_process(node);
  } else {  // done sorting
    swap_maxnum_with_buff(node->num, &node->buff);  // remove dummy

    //display
    printf("[bottom]After sorting Node %d: ", node->id);
    display_int_array(node->num, EACH_NODE_SORTING_NUM);

    write(node->fifods_w_to_upper, &node->buff, sizeof(node->buff));  // somehow this line is skipped when this is last node
  }
}


void middle_node_process (struct node *node) {
  read(node->fifods_r_from_lower, &node->buff, sizeof(node->buff));
  if (node->buff > node->num[0]) {  // received_num > having_min_num
    if (node->buff == MAX_NUM + 1) { // if done lower node sorting
      swap_minnum_with_buff(node->num, &node->buff);
      bottom_node_process(node);
      exit(0);
    }
    swap_minnum_with_buff(node->num, &node->buff);
  }
  write(node->fifods_w_to_upper, &node->buff, sizeof(node->buff));
  int prev_sent_num_to_upper;
  prev_sent_num_to_upper = node->buff;
  read(node->fifods_r_from_upper, &node->buff, sizeof(node->buff));
  if (node->buff < node->num[EACH_NODE_SORTING_NUM - 1]) {  // received_num < having_max_num
    if (node->buff == prev_sent_num_to_upper) {  // if done upper node sorting
      swap_maxnum_with_buff(node->num, &node->buff);
      write(node->fifods_w_to_lower, &node->buff, sizeof(node->buff));
      top_node_process(node);
      exit(0);
    }
    swap_maxnum_with_buff(node->num, &node->buff);
  }
  write(node->fifods_w_to_lower, &node->buff, sizeof(node->buff));
  middle_node_process(node);
}


void create_sequential_num_array (int *array) {
  int i;
  for (i = 0; i < MAX_NUM; i++){
    array[i] = i + MIN_NUM;
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


int compare_int (const void *a, const void *b) {
  if (*(int *)a > *(int *)b) {
    return 1;
  } else if (*(int *)a == *(int *)b) {
    return 0;
  } else {
    return -1;
  }
}


void swap_minnum_with_buff (int *num, int *buff) {
  swap_int(&num[0], buff);
  int i;
  for (i = 0; i < EACH_NODE_SORTING_NUM - 1; i++) {
    if (num[i] > num[i + 1]) {
      swap_int(&num[i], &num[i + 1]);
    } else {
      break;
    }
  }
}


void swap_maxnum_with_buff (int *num, int *buff) {
  swap_int(&num[EACH_NODE_SORTING_NUM - 1], buff);
  int i;
  for (i = EACH_NODE_SORTING_NUM - 1; i > 0; i--) {
    if (num[i] < num[i - 1]) {
      swap_int(&num[i], &num[i - 1]);
    } else {
      break;
    }
  }
}


void swap_int (int *a, int *b) {
  int tmp;
  tmp = *a;
  *a = *b;
  *b = tmp;
}


void display_int_array (int *array, int length) {
  int i;
  for (i = 0; i < length; i++) {
    if (i == length - 1) {
      printf("%d\n", array[i]);
    } else {
      printf("%d, ", array[i]);
    }
  }
}
