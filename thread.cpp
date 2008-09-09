
#include <thread.h>
#include <utils.h>

static void *kickoff(void *arg) {
  ((Thread*)arg)->run();
  return NULL;
}

Thread::Thread() {
  if(pthread_mutex_init(&mutex,NULL) == -1)
    E("error initializing thread mutex");
  if(pthread_attr_init(&attr) == -1)
    E("error initializing thread attribute");

  pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE);
  
  running = false;
  quit = false;
}

Thread::~Thread() {
  if(pthread_mutex_destroy(&mutex) == -1)
    E("error destroying thread mutex");
  if(pthread_attr_destroy(&attr) == -1)
    E("error destroying thread attribute");
}

bool Thread::launch() {
  lock(); // the runner will unlock when ready
  return pthread_create(&thread,&attr,&kickoff, this);
  lock(); // wait until the thread is ready
  running = true;
  unlock();
}

void Thread::lock() {
  pthread_mutex_lock(&mutex);
}
  
void Thread::unlock() {
  pthread_mutex_unlock(&mutex);
}

void Thread::join() {
  pthread_join(thread,NULL);
}
