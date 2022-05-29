#ifndef H_THREADS
#define H_THREADS

#include <pthread.h>
#include <fstream>
#include <cstdlib>

#define ASSIGN_TAG 10
#define UNASSIGN_TAG 11
#define TASK_COUNT_TAG 12
#define TIME_REPORT_TAG 13
#define TASK_PERFORMED_TAG 14
#define LIST_NUMBER_TAG 15
#define REMAIN_COUNT_RESPONCE_TAG 16
#define GLOBAL_RESULT_TAG 17

#define SENDER_TERMINATE -1
#define GET_REMAIN -2

typedef int task_t;

struct PthreadArgs {
	task_t *task_buffer;
	int root_rank;
	int *current_task_count;
	int *done_task_count;
	int *current_list_number;
	pthread_mutex_t *count_mutex;
};

struct ExecutorArgs: PthreadArgs {
	std::ostream *out_file;
	pthread_cond_t *cond;
	bool *is_filled;
	pthread_mutex_t *perform_mutex;
};

struct ReceiverArgs: PthreadArgs {
	bool *is_filled;
	pthread_cond_t *cond;
	pthread_mutex_t *perform_mutex;
};

struct SenderArgs: PthreadArgs {};

struct ManagerArgs {
	int list_count;
	int list_size;
	int process_count;
};

struct ReporterArgs {
	std::ostream *main_report;
	int list_count;
	int process_count;
};

void generate_task_list(task_t *tasks, int count, int list_index); 

double perform_task(task_t repeat_number);

void *executor_thread(void *args);

void *receiver_thread(void *args);

void *sender_thread(void *args);

void *manager_thread(void *args);

void *reporter_thread(void *args);

#endif //H_TASK
