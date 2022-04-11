#include <mpi.h>
#include <iostream>
#include <climits>

double random_double() {
	return rand() / (double) INT_MAX * 200 - 100;
}

std::ostream &print_matrix(const double *matrix, int rows, int columns, std::ostream &out) {
	for(int x = 0; x < rows; x++) {
		out << matrix[x * columns];
		for(int y = 1; y < columns; y++)
			out << '\t' << matrix[x * columns + y];
		out << '\n';
	}
	return out;
}

int main(int argc, char **argv) {
	if(argc != 6) {
		std::cerr << "Got " << argc << " argument(s) instead of 5\n";
		return 1;
	}

	MPI_Init(&argc, &argv);
	double start_time = MPI_Wtime();
	int n = atoi(argv[1]);
	int k = atoi(argv[2]);
	int m = atoi(argv[3]);
	const int DIM = 2;
	int dims[DIM], periods[DIM], reorder = 0;
	int process_count; MPI_Comm_size(MPI_COMM_WORLD, &process_count);
	int non_grid_rank; MPI_Comm_rank(MPI_COMM_WORLD, &non_grid_rank);

	for(int index = 0; index < DIM; index++) {
		dims[index] = atoi(argv[index + 4]);
		periods[index] = 1;
	}
	if(process_count != dims[0] * dims[1]) {
		if(!non_grid_rank)
			std::cerr << "Expected " << (dims[0] * dims[1]) << " processes, got "
					<< process_count << '\n';
		MPI_Finalize();
		return 1;
	}
	if(n % dims[0] != 0) {
		if(!non_grid_rank)
			std::cerr << n << " N==" << n << " must be divisible by " << dims[0] << '\n';
		MPI_Finalize();
		return 1;
	}
	if(m % dims[1] != 0) {
		if(!non_grid_rank)
			std::cerr << "M==" << m << " must be divisible by " << dims[1] << '\n';
		MPI_Finalize();
		return 1;
	}
	MPI_Comm grid_comm, column_comm, row_comm;
	MPI_Cart_create(MPI_COMM_WORLD, DIM, dims, periods, reorder, &grid_comm);
	int coords[DIM], sub_dims[DIM];
	MPI_Cart_coords(grid_comm, non_grid_rank, DIM, coords);
	sub_dims[0] = 0; sub_dims[1] = 1;
	MPI_Cart_sub(grid_comm, sub_dims, &row_comm);
	sub_dims[0] = 1; sub_dims[1] = 0;
	MPI_Cart_sub(grid_comm, sub_dims, &column_comm);

	double *A = NULL, *B = NULL, *C = NULL;
	double *sub_A = NULL, *sub_B = NULL, *sub_C = NULL;
	int sub_n = n / dims[0];
	int sub_m = m / dims[1];
	sub_A = new double[sub_n * k];
	sub_B = new double[k * sub_m];
	sub_C = new double[sub_n * sub_m];

	if(!coords[0] && !coords[1]) {
		A = new double[n * k];
		B = new double[k * m];
		C = new double[n * m];
		srand(time(0));

		for(int index = 0; index < n * k; index++)
			A[index] = random_double();
		for(int index = 0; index < k * m; index++)
			B[index] = random_double();
	}

	MPI_Datatype SUB_A;
	MPI_Type_contiguous(sub_n * k, MPI_DOUBLE, &SUB_A);
	MPI_Type_commit(&SUB_A);
	if(!coords[1])
		MPI_Scatter(A, 1, SUB_A, sub_A, 1, SUB_A, 0, column_comm);
	MPI_Bcast(sub_A, 1, SUB_A, 0, row_comm);
	MPI_Type_free(&SUB_A);

	MPI_Datatype SUB_B;
	const int MSG_ID = 12;
	MPI_Type_vector(k, sub_m, m, MPI_DOUBLE, &SUB_B);
	MPI_Type_commit(&SUB_B);
	MPI_Datatype SUB_B_CONTIGUOUS;
	MPI_Type_contiguous(k * sub_m, MPI_DOUBLE, &SUB_B_CONTIGUOUS);
	MPI_Type_commit(&SUB_B_CONTIGUOUS);
	if(!coords[0] && !coords[1]) {
		for(int row = 0; row < k; row++) {
			for(int column = 0; column < sub_m; column++)
				sub_B[row * sub_m + column] = B[row * m + column];
		}
		for(int index = 1; index < dims[1]; index++)
			MPI_Send(B + sub_m * index, 1, SUB_B, index, MSG_ID, row_comm);
	}
	if(!coords[0] && coords[1]) {
		MPI_Status status;
		MPI_Recv(sub_B, 1, SUB_B_CONTIGUOUS, 0, MSG_ID, row_comm, &status);
	}
	MPI_Bcast(sub_B, 1, SUB_B_CONTIGUOUS, 0, column_comm);
	MPI_Type_free(&SUB_B);
	MPI_Type_free(&SUB_B_CONTIGUOUS);


	for(int row = 0; row < sub_n; row++) {
		for(int column = 0; column < sub_m; column++) {
			int current_row = row * sub_m;
			sub_C[current_row + column] = 0;
			for(int index = 0; index < k; index++) {
				sub_C[current_row + column] += sub_A[row * k + index] * sub_B[index * sub_m + column];
			}
		}
	}
	delete[] sub_A;
	delete[] sub_B;

	MPI_Datatype SUB_C_MINOR;
	MPI_Type_contiguous(sub_n * sub_m, MPI_DOUBLE, &SUB_C_MINOR);
	MPI_Type_commit(&SUB_C_MINOR);

	if(dims[0] > dims[1]) {
		MPI_Datatype SUB_C_ROWS, SUB_C;
		MPI_Type_contiguous(sub_n * m, MPI_DOUBLE, &SUB_C_ROWS);
		MPI_Type_commit(&SUB_C_ROWS);
		MPI_Type_vector(sub_n, sub_m, m, MPI_DOUBLE, &SUB_C);
		MPI_Type_commit(&SUB_C);
		double *sub_C_rows = NULL;
		if(!coords[1]) {
			sub_C_rows = new double[sub_n * m];
			for(int row = 0; row < sub_n; row++) {
				for(int column = 0; column < sub_m; column++) {
					sub_C_rows[row * m + column] = sub_C[row * sub_m + column];
				}
			}
			MPI_Status status;
			for(int index = 1; index < dims[1]; index++)
				MPI_Recv(sub_C_rows + sub_m * index, 1, SUB_C, index, MSG_ID, row_comm, &status);
		}
		else MPI_Send(sub_C, 1, SUB_C_MINOR, 0, MSG_ID, row_comm);

		if(!coords[1])
			MPI_Gather(sub_C_rows, 1, SUB_C_ROWS, C, 1, SUB_C_ROWS, 0, column_comm);

		MPI_Type_free(&SUB_C_ROWS);
		MPI_Type_free(&SUB_C);
		delete[] sub_C_rows;
	}
	else {
		MPI_Datatype SUB_C_COLUMNS, SUB_C_COLUMNS_CONTIGUOUS;
		MPI_Type_vector(n, sub_m, m, MPI_DOUBLE, &SUB_C_COLUMNS);
		MPI_Type_commit(&SUB_C_COLUMNS);
		MPI_Type_contiguous(n * sub_m, MPI_DOUBLE, &SUB_C_COLUMNS_CONTIGUOUS);
		MPI_Type_commit(&SUB_C_COLUMNS_CONTIGUOUS);
		double *sub_C_columns = NULL;
		if(!coords[0])
			sub_C_columns = new double[n * sub_m];
		MPI_Gather(sub_C, 1, SUB_C_MINOR, sub_C_columns, 1, SUB_C_MINOR, 0, column_comm);

		if(!coords[0]) {
			if(!coords[1]) {
				for(int row = 0; row < n; row++) {
					for(int column = 0; column < sub_m; column++)
						C[row * m + column] = sub_C_columns[row * sub_m + column];
				}
				MPI_Status status;
				for(int index = 1; index < dims[1]; index++)
					MPI_Recv(C + sub_m * index, 1, SUB_C_COLUMNS, index, MSG_ID, row_comm, &status);
			}
			else MPI_Send(sub_C_columns, 1, SUB_C_COLUMNS_CONTIGUOUS, 0, MSG_ID, row_comm);
		}

		delete[] sub_C_columns;
		MPI_Type_free(&SUB_C_COLUMNS);
		MPI_Type_free(&SUB_C_COLUMNS_CONTIGUOUS);
	}
	MPI_Type_free(&SUB_C_MINOR);


	if(!non_grid_rank) {
		print_matrix(C, n, m, std::cout);
		double end_time = MPI_Wtime();
		std::cerr << "It took " << end_time - start_time << " seconds\n";
	}
	delete[] A;
	delete[] B;
	delete[] C;
	delete[] sub_C;

	MPI_Finalize();
	return 0;
}
