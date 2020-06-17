#ifndef TREE_CONTROLLER_H
#define TREE_CONTROLLER_H

#include <pthread.h>
#include <limits.h>

#include <list>
#include <atomic>

#define MAX_EXPLORERS 50

class work;
class Tree;
class pbab;
class bbthread;

class TreeController
{
private:
    std::atomic<unsigned int> atom_nb_explorers;
    std::atomic<unsigned int> end_counter;
    std::atomic<bool> allEnd;

public:
  int strategy;

  pbab* pbb;

  size_t M;
  // pthread_t* thread;

  bbthread *sbb[MAX_EXPLORERS];
  Tree* trees[MAX_EXPLORERS];

  pthread_mutex_t mutex_cost;
  pthread_barrier_t barrier;

  work*todo, *todo_com;
  bool passed;
  struct timespec boundTotal;

  void initTree(const int id, pbab*pbb);

  int cost;

  //honest
  pthread_mutex_t mutex_list;
  std::list<int> victim_list;
  std::list<int>::iterator list_it;
  //++++++++++++++++++++++++++++++++++==
  TreeController();
  TreeController(pbab*pbb);
  ~TreeController();

  void set_pbab(pbab*_pbb);
  bool next(const int id);

  void next();
  void explore();

  // struct timespec wait_time[EXPLORERS_MAX];
  // struct timespec synchronize_time[EXPLORERS_MAX];
  // struct timespec select_time[EXPLORERS_MAX];//tot time spent in victim selection
  // struct timespec bound_time[EXPLORERS_MAX];//tot time spent in bounding (computeCost only)
  // struct timespec startwait[EXPLORERS_MAX];
  // struct timespec endwait[EXPLORERS_MAX];
  // struct timespec elapsedwait[EXPLORERS_MAX];

  //counters
  // int nbDemandes[EXPLORERS_MAX];
  // int nbReponses[EXPLORERS_MAX];
  // long unsigned int nbBounds[EXPLORERS_MAX];
  // long unsigned int totNodesStolen[EXPLORERS_MAX];
  // int goodSteals[EXPLORERS_MAX];
  // int nullSteals[EXPLORERS_MAX];
  //
  // bool hasWork[EXPLORERS_MAX];

  // int explorer_last;
  void explorer_init();
  int explorer_get_new_id();

 //gestion de la topologie
  int choisir_thread(int id);
  int chooseNeighbour(int id);
  int random(int max);
  int chooseRandom(int id);
  int chooseLargest(int id);
  int chooseOldest(int id);

  //gestion des requetes
  int work_share(int id, int dest);
  bool try_ask(int precedent, int id);
  void demander(int id);
  void try_repondre(int id);

  void set_request(int i, bool req);
  void push_request(int victim, int id);
  int pull_request(int id);

  //gestion de la terminaison
  int compteur;
  pthread_mutex_t compteur_mutex;
  bool counter_increment(int);
  void counter_decrement();

  void unlockWaiting(int id);
  void stop(int id);

  //timing
  timespec substractTime(struct timespec t2,struct timespec t1);
  timespec addTime(struct timespec t1,struct timespec t2);
};
#endif
