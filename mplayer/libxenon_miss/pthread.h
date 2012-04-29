typedef pthread_cond_t;
typedef pthread_condattr_t;
typedef pthread_t;
typedef pthread_attr_t;
typedef pthread_mutex_t;
typedef pthread_mutexattr_t;


int   pthread_cond_broadcast(pthread_cond_t *);
int   pthread_cond_destroy(pthread_cond_t *);
int   pthread_cond_init(pthread_cond_t *, const pthread_condattr_t *);
int   pthread_cond_signal(pthread_cond_t *);
int   pthread_cond_wait(pthread_cond_t *, pthread_mutex_t *);

int   pthread_create(pthread_t *, const pthread_attr_t *,
          void *(*)(void *), void *);
void  pthread_exit(void *);
int   pthread_join(pthread_t, void **);

int   pthread_mutex_destroy(pthread_mutex_t *);
int   pthread_mutex_init(pthread_mutex_t *, const pthread_mutexattr_t *);
int   pthread_mutex_lock(pthread_mutex_t *);
int   pthread_mutex_trylock(pthread_mutex_t *);
int   pthread_mutex_unlock(pthread_mutex_t *);