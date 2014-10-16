#include "CppUnitTest.h"
#include <thread>
#include <string>
#include "cached_summation.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

TEST_CLASS(cached_summation_tests)
{
public:

	TEST_METHOD(cached_summation_node_construction)
	{
		cached_summation_node n;
		Assert::AreEqual(0,n.get_cached_sum());
		Assert::IsTrue(n.is_cache_valid());
	}

	TEST_METHOD(cached_summation_node_validation)
	{
		cached_summation_node n;
		n.validate_cache();
		Assert::IsTrue(n.is_cache_valid());
		n.invalidate_cache();
		Assert::IsFalse(n.is_cache_valid());
		n.validate_cache();
		Assert::IsTrue(n.is_cache_valid());
	}

	TEST_METHOD(cached_summation_node_connect_to_children)
	{
		cached_summation_node child0,child1;
		cached_summation_node parent;
		parent.add_child(&child0);
		parent.add_child(&child1);
		Assert::AreEqual(2,parent.get_num_of_children());
		Assert::IsTrue(&child0==parent.get_child(0));
		Assert::IsTrue(&child1==parent.get_child(1));
	}

	TEST_METHOD(cached_summation_leaf_increment_value)
	{
		cached_summation_leaf n;
		Assert::AreEqual(0,n.get_value());
		n.increment_value();
		Assert::AreEqual(1,n.get_value());
		n.increment_value();
		Assert::AreEqual(2,n.get_value());
	}

	TEST_METHOD(cached_summation_node_connect_to_leaves)
	{
		cached_summation_leaf child0,child1;
		cached_summation_node parent;
		parent.add_child(&child0);
		parent.add_child(&child1);
		child0.increment_value();
		child1.increment_value();
		child1.increment_value();
		Assert::AreEqual(3,parent.get_or_calc_sum());
	}

	TEST_METHOD(cached_summation_invalidate_ancestors)
	{
		cached_summation_leaf grandchild;
		cached_summation_node child;
		cached_summation_node parent;

		child.add_child(&grandchild);
		parent.add_child(&child);

		Assert::AreEqual(0,parent.get_or_calc_sum());
		Assert::IsTrue(parent.is_cache_valid());

		grandchild.increment_value();
		Assert::IsFalse(parent.is_cache_valid());
		Assert::IsFalse(child.is_cache_valid());
	}

	TEST_METHOD(cached_summation_invalidate_cache_in_all_descendants)
	{
		cached_summation_leaf grandchild;
		cached_summation_node child;
		cached_summation_node parent;

		child.add_child(&grandchild);
		parent.add_child(&child);
		Assert::AreEqual(0,parent.get_or_calc_sum());
		Assert::IsTrue(parent.is_cache_valid());
		Assert::IsTrue(child.is_cache_valid());
		parent.invalidate_cache_in_all_descendants();
		Assert::IsFalse(parent.is_cache_valid());
		Assert::IsFalse(child.is_cache_valid());
	}

	TEST_METHOD(cached_summation_2_threads_correctness)
	{
		for(int i=0;i<100;++i)
			validate_summation_correctness(1,20);
	}

	TEST_METHOD(cached_summation_4_threads_correctness)
	{
		for(int i=0;i<100;++i)
			validate_summation_correctness(2,20);
	}

	TEST_METHOD(cached_summation_get_all_leaves)
	{
		cached_summation_leaf child0,child1;
		cached_summation_node parent;
		cached_summation_node grandparent;
		parent.add_child(&child0);
		parent.add_child(&child1);
		grandparent.add_child(&parent);
		auto leaves = grandparent.get_all_leaves();
		Assert::IsTrue(leaves.size()==2);
		Assert::IsTrue(
			leaves[0]==&child0 && leaves[1]==&child1 || 
			leaves[0]==&child1 && leaves[1]==&child0);
	}

private:

	void validate_summation_correctness(int depth, int num_increments)
	{
		auto* root = cached_summation_node::build_full_binary_tree(depth);
		auto leaves = root->get_all_leaves();
		Assert::IsTrue(leaves.size()==1<<depth);

		std::vector<std::thread*> threads;

		// Create summation thread
		int sum = 0;
		bool run_get_or_calc_sum = true;
		std::thread summation_thread([&](){
			while(run_get_or_calc_sum) {
				sum = root->get_or_calc_sum();
			}
		});

		// Create counter threads.
		int correct_sum = 0;
		for(int i=0;i<1<<depth;++i) {
			correct_sum += num_increments;
			threads.push_back(new std::thread([&,i](){
				for(int j=0;j<num_increments;++j)
					leaves[i]->increment_value();
			}));
		}

		for(int i=0;i<(int)threads.size();++i) {
			threads[i]->join();
			delete threads[i];
		}

		run_get_or_calc_sum = false;
		summation_thread.join();
		sum = root->get_or_calc_sum();
		Assert::IsTrue(root->is_cache_valid(),L"Root's cache is invalid");

		root->invalidate_cache_in_all_descendants();
		Assert::AreEqual(correct_sum,root->get_or_calc_sum(),L"Children's sum is not as expected");
		Assert::AreEqual(correct_sum,(int)sum,L"Cached sum in parent is incorrect");
	}

};