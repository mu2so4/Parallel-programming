#include "threads.h"

#include <cmath>
#include <mpi.h>
#include <iostream>
#include <unistd.h>

#define multiplier 500

void generate_task_list(task_t *tasks, int task_count, int list_index) {
	for(int index = 0; index < task_count; index++)
		tasks[index] = multiplier + (int) (multiplier * pow((index + 1) + multiplier, 1. / 3) * sqrt(list_index + 1));
}

double perform_task(task_t repeat_count) {
	double sum = .0;
	for(int index = 0; index < repeat_count; index++)
		sum += sin(sqrt(index));
	return sum;
}

void *executor_thread(void *args) {
	ExecutorArgs *exec_args = (ExecutorArgs *) args;
	task_t *tasks = exec_args->task_buffer;
	int root_rank = exec_args->root_rank;
	int *current_task_count = exec_args->current_task_count;
	int *done_task_count = exec_args->done_task_count;
	bool *is_filled = exec_args->is_filled;
	int *current_list_number = exec_args->current_list_number;
	pthread_mutex_t *perform_mutex = exec_args->perform_mutex;
	pthread_mutex_t *count_mutex = exec_args->count_mutex;
	pthread_cond_t *cond = exec_args->cond;
	std::ostream *out = exec_args->out_file;

	double total_result = .0;
	double total_exec_time = .0;
	int total_performed_task_count = 0;

	double sub_result = .0;
	double sub_exec_time = .0;
	int performed_task_count = 0;
	int last_saved_list_number = 0;

	*out << "List\ttask\tResult\tTime\n";
	*out << "number\tcount\n";

	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	while(true) {
		pthread_mutex_lock(perform_mutex);
		if(!(*is_filled))
			pthread_cond_wait(cond, perform_mutex);
		if(last_saved_list_number != *current_list_number) {
			MPI_Send(&performed_task_count, 1, MPI_INT, root_rank,
				TASK_COUNT_TAG, MPI_COMM_WORLD);
			MPI_Send(&sub_result, 1, MPI_DOUBLE, root_rank,
				GLOBAL_RESULT_TAG, MPI_COMM_WORLD);
			MPI_Send(&sub_exec_time, 1, MPI_DOUBLE, root_rank,
				TIME_REPORT_TAG, MPI_COMM_WORLD);
			*out << last_saved_list_number << "\t" <<
				performed_task_count << "\t" << sub_result <<
				"\t" << sub_exec_time << "\n";
			last_saved_list_number = *current_list_number;
			total_result += sub_result;
			total_exec_time += sub_exec_time;
			total_performed_task_count += performed_task_count;
			sub_result = .0;
			performed_task_count = 0;
			sub_exec_time = .0;
			MPI_Barrier(MPI_COMM_WORLD);
		}
		if(*current_task_count == 0) {
			pthread_mutex_unlock(perform_mutex);
			break;
		}
		for(*done_task_count = 0; ; (*done_task_count)++) {
			pthread_mutex_lock(count_mutex);
			bool loop_is_done = *done_task_count >= *current_task_count;
			pthread_mutex_unlock(count_mutex);
			if(loop_is_done)
				break;

			sub_exec_time -= MPI_Wtime();
			sub_result += perform_task(tasks[*done_task_count]);
			sub_exec_time += MPI_Wtime();
			performed_task_count++;
		}
		MPI_Send(NULL, 0, MPI_INT, root_rank,
			TASK_PERFORMED_TAG, MPI_COMM_WORLD);
		MPI_Send(NULL, 0, MPI_INT, root_rank,
			TASK_PERFORMED_TAG, MPI_COMM_WORLD);
		*is_filled = 0;
		pthread_cond_signal(cond);
		pthread_mutex_unlock(perform_mutex);
	}
	*out << "-------------------------------------\n";
	*out << "TOTAL:\t" << total_performed_task_count << "\t" <<
		total_result << "\t" << total_exec_time << "\n";

	pthread_exit(NULL);
}

void *receiver_thread(void *args) {
	ReceiverArgs *recv_args = (ReceiverArgs *) args;
	int root_rank = recv_args->root_rank;
	task_t *task_buffer = recv_args->task_buffer;
	int *current_task_count = recv_args->current_task_count;
	int *list_number = recv_args->current_list_number;
	int *done_task_count = recv_args->done_task_count;
	bool *is_filled = recv_args->is_filled;
	pthread_cond_t *cond = recv_args->cond;
	pthread_mutex_t *perform_mutex = recv_args->perform_mutex;
	pthread_mutex_t *count_mutex = recv_args->count_mutex;

	while(true) {
		pthread_mutex_lock(perform_mutex);
		if(*is_filled)
			pthread_cond_wait(cond, perform_mutex);

		MPI_Status recv_status;
		MPI_Probe(MPI_ANY_SOURCE, ASSIGN_TAG, MPI_COMM_WORLD, &recv_status);
		MPI_Get_count(&recv_status, MPI_INT, current_task_count);
		MPI_Request list_request;
		MPI_Irecv(list_number, 1, MPI_INT, root_rank, LIST_NUMBER_TAG,
			MPI_COMM_WORLD, &list_request);
		*done_task_count = 0;
		MPI_Recv(task_buffer, *current_task_count, MPI_INT, MPI_ANY_SOURCE,
			ASSIGN_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		if(*current_task_count == 0) {
			*list_number = 0;
			*is_filled = 1;
			MPI_Request_free(&list_request);
			pthread_cond_signal(cond);
			pthread_mutex_unlock(perform_mutex);
			break;	
		}
		*is_filled = 1;
		int is_got;
		MPI_Test(&list_request, &is_got, MPI_STATUS_IGNORE);
		if(is_got) {
			MPI_Wait(&list_request, MPI_STATUS_IGNORE);
		}
		else MPI_Request_free(&list_request);

		pthread_cond_signal(cond);
		pthread_mutex_unlock(perform_mutex);
	}
	
	pthread_exit(NULL);
}


void *sender_thread(void *args) {
	SenderArgs *send_args = (SenderArgs *) args;
	int root_rank = send_args->root_rank;
	task_t *task_buffer = send_args->task_buffer;
	int *current_task_count = send_args->current_task_count;
	int *done_task_count = send_args->done_task_count;
	pthread_mutex_t *mutex = send_args->count_mutex;

	while(true) {
		int destination;
		MPI_Recv(&destination, 1, MPI_INT, root_rank, UNASSIGN_TAG,
			MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		if(destination == SENDER_TERMINATE)
			break;
		if(destination == GET_REMAIN) {
			pthread_mutex_lock(mutex);
			int remain = *current_task_count - *done_task_count;
			pthread_mutex_unlock(mutex);
			MPI_Send(&remain, 1, MPI_INT, root_rank,
				REMAIN_COUNT_RESPONCE_TAG, MPI_COMM_WORLD);
			continue;
		}
		pthread_mutex_lock(mutex);
		int unassigned_task_count = (*current_task_count - *done_task_count - 1) / 2;
		*current_task_count = *current_task_count - unassigned_task_count;
		pthread_mutex_unlock(mutex);
		if(unassigned_task_count < 1) {
			continue;
		}
		MPI_Send(task_buffer + *current_task_count, unassigned_task_count,
			MPI_INT, destination, ASSIGN_TAG, MPI_COMM_WORLD);
	}
	pthread_exit(NULL);
}

void drop_acks() {
	while(true) {
		MPI_Request ack_request;
		MPI_Irecv(NULL, 0, MPI_INT, MPI_ANY_SOURCE,
			TASK_PERFORMED_TAG, MPI_COMM_WORLD, &ack_request);
		int is_caught;
		MPI_Test(&ack_request, &is_caught, MPI_STATUS_IGNORE);
		if(is_caught) {
			MPI_Wait(&ack_request, MPI_STATUS_IGNORE);
		}
		else {
			MPI_Request_free(&ack_request);
			break;
		}
	}
}

void *manager_thread(void *args) {
	ManagerArgs *manager_args = (ManagerArgs *) args;
	int list_count = manager_args->list_count;
	int list_size = manager_args->list_size;
	int process_count = manager_args->process_count;

	int *task_list = new int[list_size];
	int *process_statuses = new int[process_count];
	int *counts = new int[process_count];
	int *offsets = new int[process_count];

	offsets[0] = 0; counts[process_count - 1] = list_size - list_size *
			(process_count - 1) / process_count;
	for(int index = 1; index < process_count; index++) {
		offsets[index] = list_size * index / process_count;
		counts[index - 1] = offsets[index] - offsets[index - 1];
	}

	for(int list_num = 0; list_num < list_count; list_num++) {
		drop_acks();
		std::cerr << "List " << list_num << '\n';
		generate_task_list(task_list, list_size, list_num);
		for(int index = 0; index < process_count; index++) {
			process_statuses[index] = counts[index];
			MPI_Send(task_list + offsets[index], counts[index],
                                MPI_INT, index, ASSIGN_TAG, MPI_COMM_WORLD);
			MPI_Send(&list_num, 1, MPI_INT, index, LIST_NUMBER_TAG,
				MPI_COMM_WORLD);
		}

		int remaining_task_count = list_size;
		while(remaining_task_count != 0) {
			int destination;
			MPI_Status ack_status;
			MPI_Recv(&destination, 0, MPI_INT, MPI_ANY_SOURCE,
				TASK_PERFORMED_TAG, MPI_COMM_WORLD, &ack_status);
			destination = ack_status.MPI_SOURCE;
			int msg = GET_REMAIN;
			for(int index = 0; index < process_count; index++) {
				MPI_Sendrecv(&msg, 1, MPI_INT, index,
					UNASSIGN_TAG, process_statuses + index, 1,
					MPI_INT, index, REMAIN_COUNT_RESPONCE_TAG, MPI_COMM_WORLD,
					MPI_STATUS_IGNORE);
			}

			int source = 0; int max_rem = process_statuses[0];
			for(int rank = 1; rank < process_count; rank++) {
				if(max_rem < process_statuses[rank]) {
					max_rem = process_statuses[rank];
					source = rank;
				}
			}
			MPI_Send(&destination, 1, MPI_INT, source, UNASSIGN_TAG,
				MPI_COMM_WORLD);

			remaining_task_count = 0;
			for(int index = 0; index < process_count; index++) {
				remaining_task_count += process_statuses[index];
			}
			std::cerr << destination << ' ' << remaining_task_count << '\n';
		}

	}
	drop_acks();

	int terminate_senders = SENDER_TERMINATE;
	for(int index = 0; index < process_count; index++) {
		MPI_Send(&terminate_senders, 1, MPI_INT, index, UNASSIGN_TAG, MPI_COMM_WORLD);
		MPI_Send(NULL, 0, MPI_INT, index, ASSIGN_TAG, MPI_COMM_WORLD);
	}

	delete[] task_list;
	delete[] process_statuses;
	delete[] counts;
	delete[] offsets;
	pthread_exit(NULL);
}

void *reporter_thread(void *args) {
	ReporterArgs *report_args = (ReporterArgs*) args;
	std::ostream *report = report_args->main_report;
	int process_count = report_args->process_count;
	int list_count = report_args->list_count;

	double *time_reports = new double[process_count];
	double *sub_results = new double[process_count];
	int *task_counts = new int[process_count];


	*report << "List\tTotal\tTotal\tMin\tMax\tDisbalance,\n";
	*report << "number\ttasks\tresult\ttime\ttime\t%\n";

	for(int list_num = 0; list_num < list_count; list_num++) {
		for(int index = 0; index < process_count; index++) {
			MPI_Recv(task_counts + index, 1, MPI_INT, index,
				TASK_COUNT_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(sub_results + index, 1, MPI_DOUBLE, index,
				GLOBAL_RESULT_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			MPI_Recv(time_reports + index, 1, MPI_DOUBLE, index,
				TIME_REPORT_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}
		double result = sub_results[0];
		int total_task_count = task_counts[0];
		double min_time = time_reports[0], max_time = min_time;
		for(int index = 1; index < process_count; index++) {
			min_time = std::min(min_time, time_reports[index]);
			max_time = std::max(max_time, time_reports[index]);
			result += sub_results[index];
			total_task_count += task_counts[index];
		}
		*report << list_num << "\t" << total_task_count << '\t' << result << '\t' <<
			min_time << "\t" << max_time << "\t" <<
			100. * (1 - min_time / max_time) << "\n";
	}

	delete[] time_reports;
	delete[] sub_results;
	delete[] task_counts;
	pthread_exit(NULL);
}
