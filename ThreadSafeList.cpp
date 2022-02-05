#include <utility>
#include <mutex>
#include <memory>
#include <iostream>

template<typename T>
class ThreadSafeList
{
	private:
		 struct list_node_s
		 {
			 std::mutex m;
			 std::shared_ptr<T> data;
			 std::unique_ptr<list_node_s> next;

//			 default node constructor
			 list_node_s()
			 :next()
		 	 {}

//			 parameter node constructor
		 	 list_node_s(T const& value)
		 	 :data(std::make_shared<T>(value))
		 	 {}
		 };
	 	 list_node_s head;

	public:
//		 Default constructor
		 ThreadSafeList()
		 {}

// 		 Defualt destructor
	 	~ThreadSafeList()
		 {
		 	DeleteItem([](list_node_s const&)
		 	{
		 		return true;
		 	});
		 }

//		Default copy constructor - singleton implementation
	  	 ThreadSafeList(ThreadSafeList const& other)=delete;
//	  	 Default assignment constructor - singleton implementation
	  	 ThreadSafeList& operator=(ThreadSafeList const& other)=delete;


//------------Insert New value into List--------------//
		 void InsertNew(T const& value)
		 {
//			 create node and initialize with value
			 std::unique_ptr<list_node_s> new_node(new list_node_s(value));
//			 Lock head node
			 std::lock_guard<std::mutex> lk(head.m);
//			 safely acquire  pointer to next node
			 new_node->next=std::move(head.next);
//			 add new node
			 head.next=std::move(new_node);
		 }


//-------------------------Membership-----------------//
		template<typename Predicate>
		std::shared_ptr<T> Membership(Predicate predicate)
		{
//		 		get head node
		 		list_node_s* current=&head;

//		 		lock head node
		 		std::unique_lock<std::mutex> lk(head.m);
//				check out all nodes
				while(list_node_s* const next=current->next.get())
				{

//			 		lock next node
			 		std::unique_lock<std::mutex> next_lk(next->m);
//			 		release head node
			 		lk.unlock();
//			 		check if node meets predicate
			 		if(predicate==(*next->data))
			 		{
//			 			return found node
			 			return next->data;
			 		}
//			 		otherwise update and move on
			 		current=next;
			 		lk=std::move(next_lk);
			 	}

//	 		return NULL
	 		return std::shared_ptr<T>();
	 	}


//------------------------DeleteItem-----------------//
	 	template<typename Predicate>
	 	void DeleteItem(Predicate predicate)
	 	{
//	 		get pointer to head node
	 		list_node_s* current=&head;
//	 		lock node
	 		std::unique_lock<std::mutex> lk(head.m);
//	 		check out all nodes
	 		while(list_node_s* const next=current->next.get())
	 		{
//	 			lock node
	 			std::unique_lock<std::mutex> next_lk(next->m);
//	 			check if node meets predicate
	 			if(predicate(*next->data))
	 			{
//
//	 				remove node by updating current->next
//					node is deleted by going out of scope
	 				std::unique_ptr<list_node_s> old_next=std::move(current->next);
	 				current->next=std::move(next->next);
	 				next_lk.unlock();
	 			}
	 			else
	 			{
//	 				Otherwise unlock and move on
	 				lk.unlock();
	 				current=next;
	 				lk=std::move(next_lk);
	 			}
	 		}
	 	}
};



auto main(void)->int
{



    ThreadSafeList<int> List;
    List.InsertNew(4);
    List.InsertNew(3);
    List.InsertNew(2);
    List.InsertNew(1);
    auto found = List.Membership<int>(3);
    std::cout<<*found<<std::endl;

}
