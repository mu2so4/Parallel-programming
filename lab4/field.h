#ifndef H_FIELD
#define H_FIELD

#include <vector>
typedef bool cell_t;

class FieldHistory {
	std::vector<cell_t*> snapshots;
	std::vector<int> factor_set;
	std::vector<int> first_alive_cells;
	int buffer_size;

	bool compare(const cell_t *a, int first_alive_a, const cell_t *b, int first_alive_b) const;
	int first_alive(const cell_t *field) const;

public:
	FieldHistory(int);
	~FieldHistory();

	bool add(cell_t *snapshot);
	int last_flag() const;
};


void add_glider(int row, int column, cell_t *field, int height, int width);

#endif //H_FIELD

