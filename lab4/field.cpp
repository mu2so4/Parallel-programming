#include "field.h"
#include <cstdlib>

void add_glider(int row, int column, cell_t *field, int height, int width) {
	field[row * width + column + 1] =
		field[(row + 1) * width + column + 2] = 1;
	for(int index = 0; index < 3; index++)
		field[(row + 2) * width + column + index] = 1;
}

int FieldHistory::first_alive(const cell_t *field) const {
	for(int index = 0; index < buffer_size; index++)
		if(field[index] == 1)
			return index;
	return buffer_size;
}

FieldHistory::FieldHistory(int data_size):
	buffer_size(data_size) {}

FieldHistory::~FieldHistory() {
	for(size_t index = 0; index < snapshots.size(); index++)
		delete[] snapshots[index];
}

bool FieldHistory::compare(const cell_t *a, int first_alive_a, const cell_t *b, int first_alive_b) const {
	if(a == nullptr || b == nullptr)
		return false;
	if(first_alive_a != first_alive_b)
		return false;
	for(int index = first_alive_a + 1; index < buffer_size; index++)
		if(a[index] != b[index])
			return false;
	return true;
}



bool FieldHistory::add(cell_t *snapshot) {
	int first = first_alive(snapshot);
	for(size_t index = 0; index < snapshots.size(); index++) {
		if(compare(snapshots[index], first_alive_cells[index], snapshot, first)) {
			factor_set.push_back(index);
			return false;
		}
	}
	factor_set.push_back(snapshots.size());
	snapshots.push_back(snapshot);
	first_alive_cells.push_back(first);
	return true;
}

int FieldHistory::last_flag() const {
	return factor_set.back();
}
