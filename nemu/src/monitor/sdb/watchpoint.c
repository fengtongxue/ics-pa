#include "sdb.h"

#define NR_WP 32
#define EXPR_LEN 100


static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
    wp_pool[i].expr = malloc(EXPR_LEN);
    wp_pool[i].value = 0;
  }

  // 监视中的是 null  head 应该也是指向 第一个 wp (head) 的指针， 
  // -1 指代表链表， 其他代表 node
  WP *head_head = malloc(sizeof(WP));
  head_head->NO = -1;
  head_head->next = NULL;
  head = head_head ;
        
  // 空闲的是整个 wp pool    数组的指针也是指向第一个值地址的指针， 指向 head
  WP *free_head = malloc(sizeof(WP));
  free_head->NO = -1;
  free_head->next = wp_pool;
  free_ = free_head;
}

// log pretty linkList
void
log_linkList(WP* linkList) {
  WP * c = linkList;
  while (c != NULL)
  {
    if (c->NO != -1) {
      printf("node NO: %d\n", c->NO);
    } 
    c = c->next;
  }
}

// pop first 
// if NULL  panic
WP*
linkList_pop1(WP* linkList) {
  WP* head = linkList->next;  
  if (head == NULL) {
    panic("linkList_pop1: no nore node");
  } else {
    linkList->next = head->next;
    head->next = NULL;
  }
  return head;
}

// list .next = node
// node. next = head
void
workinglinkList_add(WP* node) {
  WP* headnode = head->next;
  head->next = node;
  node->next = headnode;
}

void
freelinkList_add(WP* node) {
  WP* headnode = free_->next;
  head->next = node;
  node->next = headnode;
}


void 
LinkTest() {
  log_linkList(free_);
  WP* wp = linkList_pop1(free_);
  printf("NO: %d\n", wp->NO);

  wp = linkList_pop1(free_);
  printf("NO: %d\n", wp->NO);

  wp = linkList_pop1(free_);
  printf("NO: %d\n", wp->NO);

  log_linkList(free_);
  // linkList_add(free_, wp);
  log_linkList(free_);

}

/*
  其中new_wp()从free_链表中返回一个空闲的监视点结构
*/
WP* 
new_wp() {
  WP* wp = linkList_pop1(free_);
  return wp;
}



// free_wp()将wp归还到free_链表中, 这两个函数会作为监视点池的接口被其它函数调用. 需要注意的是, 
// 调用new_wp()时可能会出现没有空闲监视点结构的情况,
// 为了简单起见, 此时可以通过assert(0)马上终止程序
void 
free_wp(WP *wp) {
  freelinkList_add(wp);
}

/*
 todo 
 watch point $t0 没有暂停
 
*/ 
void check_watchpoints() {
  WP *wp;
  bool success;

  wp = head->next;
  while (wp != NULL)
  { 
    word_t value = expr(wp->expr, &success);
    if (!success) {
      panic("error");
    }
    if (value != wp->value) {
      printf("watchpoints: %s\n", wp->expr);
      printf("Old value = %d", wp->value);
      printf("New value = %d", value);
      wp->value = value;
    }
    wp = wp->next;
  }
}

void
watchpoints_dispaly() {
  printf("Num     Type           Disp Enb Address            What\n");
  WP *wp = head->next;
  while (wp != NULL) {
    printf("%d      hw watchpoint  Disp Enb %s\n", wp->NO, wp->expr);
    wp = wp->next;
  }
}