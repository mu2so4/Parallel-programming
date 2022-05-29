#include <pthread.h>
#include <mpi.h>
#include <iostream>
#include <fstream>
#include <string>
#include <signal.h>
#include <unistd.h>
#include "threads.h"

#define CORRECT_ARGC 2

std::ofstream *out, *report;

void interrupt(int signum) {
	if(signum == SIGINT) {
		out->close();
		if(report != NULL) {
			report->close();
			delete report;
		}
		abort();
	}
}

int main(int argc, char **argv) {
	if(argc!= CORRECT_ARGC + 1) {
		std::cerr << "Wrong argc " << argc << '\n';
		return 1;
	}
	signal(SIGINT, interrupt);

	int list_count = atoi(argv[1]);
	int list_size = atoi(argv[2]);

	int real;
	MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &real);
	if(real != MPI_THREAD_MULTIPLE) {
		perror("Failed to get thread access");
		MPI_Finalize();
		return 1;
	}

	pthread_attr_t attrs;
	if(pthread_attr_init(&attrs) != 0) {
		perror("Cannot initialize attributes");
		MPI_Finalize();
                return 1;
    	};
	if(pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_JOINABLE) != 0) {
		perror("Error in setting attributes");
		MPI_Finalize();
		return 1;
	}

	int process_count, rank;
	MPI_Comm_size(MPI_COMM_WORLD, &process_count);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	const int root_rank = process_count / 2;
	int max_task_count = list_size * (rank + 1) / process_count - list_size * rank / process_count;
	task_t *task_buffer = new task_t[max_task_count];

	std::ofstream out("process" + std::to_string(rank) + ".txt");
	std::ofstream *report = NULL;
	if(root_rank == rank)
		report = new std::ofstream("report.txt");
	::report = report;
	::out = &out;

	int current_task_count = 0, done_task_count = 0, current_list_number = 0;
	bool is_filled = 0;

	pthread_mutex_t count_mutex, perform_mutex;
	pthread_cond_t working_cond;
	pthread_mutex_init(&count_mutex, NULL);
	pthread_mutex_init(&perform_mutex, NULL);
	pthread_cond_init(&working_cond, NULL);

	pthread_t threads[5];
	
	ExecutorArgs exec_args;
	ReceiverArgs recv_args;
	SenderArgs send_args;
	ManagerArgs manager_args;
	ReporterArgs reporter_args;

	exec_args.task_buffer = recv_args.task_buffer = send_args.task_buffer = task_buffer;
	exec_args.root_rank = recv_args.root_rank = send_args.root_rank = root_rank;
	exec_args.current_task_count = recv_args.current_task_count =
		send_args.current_task_count = &current_task_count;
	exec_args.done_task_count = recv_args.done_task_count = send_args.done_task_count = &done_task_count;
	exec_args.current_list_number = recv_args.current_list_number = send_args.current_list_number =
		&current_list_number;
	exec_args.count_mutex = recv_args.count_mutex = send_args.count_mutex = &count_mutex;

	exec_args.is_filled = recv_args.is_filled = &is_filled;
	exec_args.perform_mutex = recv_args.perform_mutex = &perform_mutex;
	exec_args.cond = recv_args.cond = &working_cond;
	exec_args.out_file = &out;

	manager_args.list_count = reporter_args.list_count = list_count;
	manager_args.process_count = reporter_args.process_count = process_count;
	manager_args.list_size = list_size;
	reporter_args.main_report = report;

	pthread_create(threads, &attrs, executor_thread, &exec_args);
	pthread_create(threads + 1, &attrs, receiver_thread, &recv_args);
	pthread_create(threads + 2, &attrs, sender_thread, &send_args);

	if(rank == root_rank) {
		pthread_create(threads + 3, &attrs, manager_thread, &manager_args);
		pthread_create(threads + 4, &attrs, reporter_thread, &reporter_args);
	}

	for(int index = 0; index < 3; index++)
		pthread_join(threads[index], NULL);
	for(int index = 3; root_rank == rank && index < 5; index++) {
		pthread_join(threads[index], NULL);
	}

	pthread_attr_destroy(&attrs);
	pthread_mutex_destroy(&count_mutex);
	pthread_mutex_destroy(&perform_mutex);
	pthread_cond_destroy(&working_cond);

	delete[] task_buffer;
	out.close();
	if(report != NULL)
		report->close();
	delete report;

	MPI_Finalize();
}
