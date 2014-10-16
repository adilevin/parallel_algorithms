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
	static cached_summation_node* build_full_binary_tree(int depth); // depth=0 is just a leaf. depth=1 has two leaves, etc...
	void add_child(cached_summation_node* child);
	int get_num_of_children();

	// Summation
	virtual int get_or_calc_sum();

protected:
	cached_summation_node();
	virtual ~cached_summation_node() {}
	void invalidate_cache_of_all_ancestors(); // For tests only
private:
	int calc_sum_from_children();
	void append_leaves(std::vector<cached_summation_leaf*>& leaves);
	int get_cached_sum() const;
	bool is_cache_valid() const;
	void validate_cache();
	void invalidate_cache();
	void invalidate_cache_in_all_descendants();
	std::vector<cached_summation_leaf*> get_all_leaves();
	cached_summation_node* get_child(int index);
	friend class cached_summation_tests;
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
	int get_value() const { return value; }
	void increment_value();
	virtual int get_or_calc_sum();
private:
	cached_summation_leaf() : value(0) {}
	virtual ~cached_summation_leaf() {}
	friend static cached_summation_node* cached_summation_node::build_full_binary_tree(int depth);
	friend class cached_summation_tests;
private:
	int value;
};
