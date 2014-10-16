#include "cached_summation.h"
#include <windows.h>

cached_summation_node::cached_summation_node() : cache_is_valid(1), num_of_children(0), cached_sum(0), parent(0)
{

}

int cached_summation_node::get_cached_sum() const
{
	return cached_sum;
}

bool cached_summation_node::is_cache_valid() const
{
	return cache_is_valid!=0;
}

int cached_summation_node::get_num_of_children()
{
	return num_of_children;
}

void cached_summation_node::validate_cache()
{
	InterlockedExchange(&cache_is_valid,1);
}

void cached_summation_node::invalidate_cache()
{
	InterlockedExchange(&cache_is_valid,0);
}

std::vector<cached_summation_leaf*> cached_summation_node::get_all_leaves()
{
	std::vector<cached_summation_leaf*> v;
	append_leaves(v);
	return v;
}

void cached_summation_node::invalidate_cache_in_all_descendants()
{
	invalidate_cache();
	for(int i=0;i<num_of_children;++i)
		children[i]->invalidate_cache_in_all_descendants();
}

void cached_summation_node::add_child( cached_summation_node* child )
{
	invalidate_cache(); 
	children[num_of_children++]=child; 
	child->parent = this;
}

cached_summation_node* cached_summation_node::get_child( int index )
{
	return children[index];
}

int cached_summation_node::get_or_calc_sum()
{
	if (!is_cache_valid()) {
		validate_cache();
		InterlockedExchange(&cached_sum,calc_sum_from_children());
	}
	return cached_sum;
}

void cached_summation_node::invalidate_cache_of_all_ancestors()
{
	cached_summation_node* ancestor = parent;
	while (ancestor)
	{
		ancestor->invalidate_cache();
		ancestor = ancestor->parent;
	}
}

int cached_summation_node::calc_sum_from_children()
{
	int sum = 0;
	for(int i=0;i<num_of_children;++i)
		sum += children[i]->get_or_calc_sum();
	return sum;
}

void cached_summation_node::append_leaves( std::vector<cached_summation_leaf*>& leaves )
{
	if (auto* leaf = dynamic_cast<cached_summation_leaf*>(this))
		leaves.push_back(leaf);
	for(int i=0;i<num_of_children;++i)
		children[i]->append_leaves(leaves);
}


void cached_summation_leaf::increment_value()
{
	++value;
	invalidate_cache_of_all_ancestors();
}

int cached_summation_leaf::get_or_calc_sum()
{
	return value;
}

cached_summation_node* cached_summation_node::build_full_binary_tree(int depth)
{
	if (depth==0)
		return new cached_summation_leaf;
	else {
		auto* node = new cached_summation_node;
		node->add_child(build_full_binary_tree(depth-1));
		node->add_child(build_full_binary_tree(depth-1));
		return node;
	}
}