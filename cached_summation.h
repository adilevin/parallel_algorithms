#include <vector>

/*

	CACHED SUMMATION:

	Adi Levin, 16/October/2014

	A lock-free mechanism for summation of counters from different threads.
	
	Problem:

		Design a counter that can be incremented by N threads in parallel, without locking.

	Solution:

		1. Each thread increments its own counter (wrapped in cached_summation_leaf).
		2. We build a binary tree whose leaves are the individual counters.
		3. Each node in the tree caches the sum of its descendants.
		4. Whenever a leaf is updated, it invalidates the cache of all of its ancestors.

*/

class cached_summation_leaf;

class cached_summation_node
{
public:
	cached_summation_node();

	int get_cached_sum() const;
	bool is_cache_valid() const;
	void validate_cache();
	void invalidate_cache();
	int get_num_of_children();
	std::vector<cached_summation_leaf*> get_all_leaves();
	void invalidate_cache_in_all_descendants();
	void add_child(cached_summation_node* child);
	cached_summation_node* get_child(int index);
	virtual int get_or_calc_sum();
	void invalidate_cache_of_all_ancestors();
private:
	int calc_sum_from_children();
	void append_leaves(std::vector<cached_summation_leaf*>& leaves);
private:			
	cached_summation_node* parent;
	cached_summation_node* children[2];
	int num_of_children;
	volatile long cache_is_valid;
	long cached_sum;
};

class cached_summation_leaf : public cached_summation_node
{
public:
	cached_summation_leaf() : value(0) {}
	int get_value() const { return value; }
	void increment_value();
	virtual int get_or_calc_sum();
private:
	int value;
};
