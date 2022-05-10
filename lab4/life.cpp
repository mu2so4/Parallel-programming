#include <mpi.h>
#include <iostream>
#include <fstream>
#include <vector>
#include "field.h"

#define correct_argc 2

void inline next_generation_in_row(const cell_t *old_generation, const cell_t *row_below,
		const cell_t *row_above, cell_t *new_generation, int row_size) {
	int neighbour_count = old_generation[1] + old_generation[row_size - 1];
	neighbour_count += row_below[row_size - 1] + row_below[0] + row_below[1];
	neighbour_count += row_above[row_size - 1] + row_above[0] + row_above[1];
	new_generation[0] = neighbour_count == 3 || (neighbour_count == 2 && old_generation[0]);

	neighbour_count = old_generation[0] + old_generation[row_size - 2];
	neighbour_count += row_below[row_size - 2] + row_below[row_size - 1] + row_below[0];
	neighbour_count += row_above[row_size - 2] + row_above[row_size - 1] + row_above[0];
	new_generation[row_size - 1] = neighbour_count == 3 || (neighbour_count == 2 && old_generation[row_size - 1]);

	for(int index = 1; index < row_size - 1; index++) {
		neighbour_count = old_generation[index - 1] + old_generation[index + 1];
		neighbour_count += row_below[index - 1] + row_below[index] + row_below[index + 1];
		neighbour_count += row_above[index - 1] + row_above[index] + row_above[index + 1];
		new_generation[index] = neighbour_count == 3 || (neighbour_count == 2 && old_generation[index]);
	}
}

int main(int argc, char **argv) {
	if(argc != correct_argc + 1) {
		std::cerr << "got " << argc - 1 << " arguments, expected ";
		std::cerr << correct_argc << '\n';
		return 1;
	}

	int height = atoi(argv[1]);
	int width = atoi(argv[2]);
	MPI_Init(&argc, &argv);
	double begin_time = MPI_Wtime();
	int rank, rank_below, rank_above, process_count, row_count;
	MPI_Comm_size(MPI_COMM_WORLD, &process_count);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	rank_below = (rank + 1) % process_count;
	rank_above = (rank + process_count - 1) % process_count;
	row_count = height * (rank + 1) / process_count - height * rank / process_count;
	cell_t *sub_field = new cell_t[row_count * width];

	MPI_Datatype row_t;
	MPI_Type_contiguous(width, MPI_INT8_T, &row_t);
	MPI_Type_commit(&row_t);

	const int root = 0;
	if(rank == root) {
		cell_t *init_field = new cell_t[height * width];
		for(int index = 0; index < height * width; index++)
			init_field[index] = 0;
		add_glider(0, 0, init_field, height, width);
		int *counts = new int[process_count];
		int *offsets = new int[process_count];
		offsets[0] = 0; counts[process_count - 1] = height - height * (process_count - 1) / process_count;
		for(int index = 1; index < process_count; index++) {
			offsets[index] = height * index / process_count;
			counts[index - 1] = offsets[index] - offsets[index - 1];
		}

		MPI_Scatterv(init_field, counts, offsets, row_t, sub_field, row_count, row_t, root, MPI_COMM_WORLD);

		delete[] init_field;
		delete[] counts;
		delete[] offsets;
	}
	else MPI_Scatterv(NULL, NULL, NULL, row_t, sub_field, row_count, row_t, root, MPI_COMM_WORLD);

	cell_t *buff_new_gen = new cell_t[row_count * width];
	cell_t *row_below = new cell_t[width];
	cell_t *row_above = new cell_t[width];

	int iteration;
	
	FieldHistory checker{row_count * width};
	std::vector<int> *flags = new std::vector<int>[process_count];
	int *last_flags = new int[process_count];

	MPI_Request border_requests[4];
	MPI_Status border_statuses[4];
	bool is_added;
	int last_my_flag;

	for(iteration = 0;; iteration++) {
		MPI_Isend(sub_field, 1, row_t, rank_above, rank * 2, MPI_COMM_WORLD, border_requests);
		MPI_Isend(sub_field + width * (row_count - 1), 1, row_t,
				rank_below, rank * 2 + 1, MPI_COMM_WORLD, border_requests + 2);
		MPI_Irecv(row_above, 1, row_t, rank_above, rank_above * 2 + 1, MPI_COMM_WORLD, border_requests + 1);
		MPI_Irecv(row_below, 1, row_t, rank_below, rank_below * 2, MPI_COMM_WORLD, border_requests + 3);

		is_added = checker.add(sub_field);
		last_my_flag = checker.last_flag();

		for(int index = 1; index < row_count - 1; index++)
			next_generation_in_row(sub_field + width * index,
					sub_field + width * (index - 1),
					sub_field + width * (index + 1),
					buff_new_gen + width * index, width);

		MPI_Waitall(2, border_requests + 1, border_statuses + 1);
		next_generation_in_row(sub_field, sub_field + width,
				row_above, buff_new_gen, width);

		MPI_Wait(border_requests, border_statuses);
		MPI_Wait(border_requests + 3, border_statuses + 3);
		next_generation_in_row(sub_field + width * (row_count - 1), row_below,
				sub_field + width * (row_count - 2),
				buff_new_gen + width * (row_count - 1), width);

		if(is_added) {
			sub_field = buff_new_gen;
			buff_new_gen = new cell_t[row_count * width];
		}
		else std::swap(buff_new_gen, sub_field);

		MPI_Allgather(&last_my_flag, 1, MPI_INT, last_flags, 1, MPI_INT, MPI_COMM_WORLD);
		bool is_repeated = false;

		for(size_t index = 0; !is_repeated && index < flags[0].size(); index++) {
			is_repeated = true;
			for(int sub_rank = 0; sub_rank < process_count && is_repeated; sub_rank++) {
				is_repeated = flags[sub_rank][index] == last_flags[sub_rank];
			}
		}
		bool is_repeated_somewhere = false;
		MPI_Allreduce(&is_repeated, &is_repeated_somewhere, 1, MPI_INT8_T, MPI_SUM, MPI_COMM_WORLD);
		if(is_repeated_somewhere)
			break;
		if(iteration % process_count == rank) {
			for(int index = 0; index < process_count; index++)
				flags[index].push_back(last_flags[index]);
		}

	}

	double end_time = MPI_Wtime();

	if(rank == root) {
		std::cerr << end_time - begin_time << '\n';
		std::cout << iteration + 1 << '\n';
	}


	delete[] sub_field;
	delete[] row_below;
	delete[] row_above;
	delete[] buff_new_gen;
	delete[] flags;
	delete[] last_flags;
	MPI_Type_free(&row_t);
	MPI_Finalize();
	return 0;
}
