#pragma once

#include <functional>
#include <algorithm>
#include <utility>
#include <iostream>

template <typename Key, size_t N =10>
class ADS_set
{
public:
	class ForwardIterator;
	using value_type = Key;
	using key_type = Key;
	using reference = value_type&;
	using const_reference = const value_type&;
	using size_type = size_t;
	using difference_type = std::ptrdiff_t;
	using const_iterator = ForwardIterator;
	using iterator = const_iterator;
	using key_compare = std::less<key_type>;   // B+-Tree
	using key_equal = std::equal_to<key_type>;

private:
	
	struct Node
	{
		value_type* elements[N*2+1];
		Node* pointers[N*2+2];
		bool children;
		int pos;
		Node* nextptr;
		Node* previousptr;
		Node* parent;

		Node()
		{
			children=false;
			pos=0;
			nextptr=nullptr;
			previousptr=nullptr;
			parent=nullptr;
		}
		Node(const Node& other) : Node()
		{
			for (size_type x = 0; x < N * 2 + 1; x++)
			{
				elements[x] = other.elements[x];
				pointers[x]=other.pointers[x];
				
			}
			pointers[N * 2 + 1]=other.pointers[N * 2 + 1];
			children=other.children;
			pos=other.pos;
			nextptr=other.nextptr;
			previousptr=other.previousptr;
			parent=other.parent;
		}
		~Node()
		{
			if(!children)
			for(int x=0;x<pos;++x)
					delete elements[x];
			else
				for(int x=0;x<pos+1;++x)
					delete pointers[x];
		}
	};
public:
	class ForwardIterator
	{
	public:
		using value_type = ADS_set::value_type;
		using reference = ADS_set::const_reference;
		using difference_type = ADS_set::difference_type;
		using pointer = const value_type *;
		using iterator_category = std::forward_iterator_tag;
	private:
		Node* node;
		int index;
	public:
		ForwardIterator() : node(nullptr), index(-1) {}
		ForwardIterator(Node* node, int pos) : node(node), index(pos) {}
		ForwardIterator(const ForwardIterator& other) : node(other.node), index(other.index) {}
		~ForwardIterator(){}
		

		reference operator* () const
		{
			return *node->elements[index];
		}
		pointer operator->() const
		{
			return node->elements[index];
		}

		friend bool operator==(const iterator &lhs, const iterator &rhs)
		{
			return (lhs.node==rhs.node && lhs.index==rhs.index);
		}
		friend bool operator!=(const iterator &lhs, const iterator &rhs)
		{
			return (lhs.node!=rhs.node || lhs.index!=rhs.index);
		}
		iterator& operator++()
		{
			
			if(index>node->pos-2)
			{
				if(node->nextptr!=nullptr){
					node=node->nextptr;
					index=0;
				}
				else{
					index=node->pos;
				}
			}
			else
				++index;
			return *this;
		}
		iterator operator++(int)
		{
			iterator it=*this;
			++*this;
			return it;
		}

	};

private:
	Node *root;
	size_type number_elements;

	Node* find_leaf(Node* current, key_type value) const//Finds the leaf in which the value that we care about is 
	{
		if(current->children==false)
		{
			return current;
		}
		return find_leaf(current->pointers[find_pos(current, value)], value);
	}
	void insert_value(Node* current, int pos, key_type* value, Node* new_node) // Used to insert new value to an inner node after the insert() function is called. Inserts the value in the position, and updates the pointers as well
	{
		if(current->pos==static_cast<int>(N)*2)
		{
			Node* next_ptr= current->nextptr;
			current->nextptr=new Node;
			current->nextptr->previousptr=current;
			current->nextptr->children=true;
			if(next_ptr!=nullptr)
			{
				current->nextptr->nextptr=next_ptr;
				next_ptr->previousptr=current->nextptr;
			}
			next_ptr=current->nextptr;

			if(pos<static_cast<int>(N)+1)
			{
				if(pos==N)
				{
					next_ptr->pointers[0]=new_node;
				}
				else
				{
					next_ptr->pointers[0]=current->pointers[N];
					current->pointers[N]=nullptr;
				}
				next_ptr->pointers[0]->parent=next_ptr;
				for(size_type x=N; x<N*2;++x)
				{
					next_ptr->elements[x-N]=current->elements[x];
					next_ptr->pointers[x-N+1]=current->pointers[x+1];
					next_ptr->pointers[x-N+1]->parent=next_ptr;
					current->elements[x]=nullptr;
					current->pointers[x+1]=nullptr;
				}
				for(int x=static_cast<int>(N); x>pos;--x)
				{
					current->elements[x]=current->elements[x-1];
					current->pointers[x+1]=current->pointers[x];
				}
				current->elements[pos]=value;
				if(pos!=N)
				{
					current->pointers[pos+1] = new_node;
					new_node->parent=current;
				}
				
			}
			else
			{
				for(int x=static_cast<int>(N)+1; x<pos;++x)
				{
					next_ptr->elements[x-(static_cast<int>(N)+1)]=current->elements[x];
					next_ptr->pointers[x-(static_cast<int>(N)+1)]=current->pointers[x];
					next_ptr->pointers[x-(static_cast<int>(N)+1)]->parent=next_ptr;
					current->elements[x]=nullptr;
					current->pointers[x]=nullptr;
				}
				next_ptr->pointers[pos-(static_cast<int>(N)+1)]=current->pointers[pos];
				next_ptr->pointers[pos-(static_cast<int>(N)+1)]->parent=next_ptr;
				current->pointers[pos]=nullptr;
				next_ptr->elements[pos-(static_cast<int>(N)+1)]=value;
				next_ptr->pointers[pos-(static_cast<int>(N))]=new_node;
				next_ptr->pointers[pos-(static_cast<int>(N))]->parent=next_ptr;
				for(size_type x=pos; x<N*2;++x)
				{
					next_ptr->elements[x-N]=current->elements[x];
					next_ptr->pointers[x-N+1]=current->pointers[x+1];
					next_ptr->pointers[x-N+1]->parent=next_ptr;
					current->elements[x]=nullptr;
					current->pointers[x+1]=nullptr;
				}
				
			}
			
			next_ptr->pos=N;
			current->pos=N;
			Node* parent=current->parent;
			if(parent!=nullptr)
			{
				current->nextptr->parent=parent;
				insert_value(parent, find_pos(parent, *current->elements[N]), current->elements[N], current->nextptr);
			}
			else// creates new parent if current is the root (has no parent already)
			{
				current->parent=new Node;
				parent=current->parent;
				parent->elements[0]=current->elements[N];
				parent->pointers[0]=current;
				parent->pointers[1]=next_ptr;
				parent->children=true;
				next_ptr->parent=parent;
				root=parent;
				parent->pos=1;
			}
			current->elements[N]=nullptr;
		}
		else
		{
			for(int x=current->pos; x>pos;--x)
			{
				current->elements[x]=current->elements[x-1];
				current->pointers[x+1]=current->pointers[x];
			}
			current->elements[pos]=value;
			current->pointers[pos+1] = new_node;
			++current->pos;

			
		}

	}

	int find_pos(Node *current, key_type value) const// finds the position of the value that we care about
	{
		for(int x=0;x<current->pos;x++)
		{
			if(key_compare{}(value, *current->elements[x]))
			{
				return x;
			}
		}
		return current->pos;
	}

	value_type* find_first_element(Node* current) const //Returns the smallest element when current node is take as a pseudo-root (important to update the tree when there is a merger of inner nodes)
	{
		if(current->children==false)
			return current->elements[0];
		
		return find_first_element(current->pointers[0]);
	}

	void update_inner(Node* current, const key_type& initial_value, key_type* new_value) // When the value erased was the first element of a leaf, an element in an inner node needs to be updated with the new first element
	{
		int pos=find_pos(current, initial_value)-1;
		if(pos==-1)
		{
			if(current!=root)
				update_inner(current->parent, initial_value, new_value);
		}
		else
		{
			current->elements[pos]=new_value;
		}

	}
	void erase_inner(Node* current, const key_type& value) // used to erase value of an inner node when the child node has been destroyed during deletion
	{
		int pos=find_pos(current, value)-1;
		Node* nextptr=nullptr;
		if(pos==(current->pos-1))
			nextptr=current->pointers[current->pos]->nextptr;
		if(current->pointers[pos+1]->pos<static_cast<int>(N))
		{
			for(int x=pos;x<current->pos;++x)
			{
				current->elements[x]=current->elements[x+1];
				current->pointers[x+1]=current->pointers[x+2];
			}
			current->pointers[pos]->nextptr=nullptr;
		}
		else
		{
			for(int x=pos;x<current->pos;++x)
			{
				current->elements[x]=current->elements[x+1];
				current->pointers[x]=current->pointers[x+1];
			}
		}
		--current->pos;
		current->elements[current->pos]=nullptr;
		current->pointers[current->pos+1]=nullptr;
		if(pos==current->pos && nextptr!=nullptr)
			current->pointers[current->pos]->nextptr=nextptr;
		
		if(current->pos<static_cast<int>(N))
		{
			if(current!=root)
				merge_inner(current);
			else
			{
				current->parent=nullptr;
				if(current->pointers[1]==nullptr)
				{
					Node* previous_root=root;
					root=current->pointers[0];
					previous_root->pointers[0]=nullptr;
					delete previous_root;
					root->parent=nullptr;
				}
			}
		}
	}
	void merge_inner(Node* current) // When current->pos >N+1, then the nodes will need to be split again, so there is no need to do any actual splitting. 
									//When it's N or N+1, current needs to be deleted and all the operations that come with it.
	{
		value_type* first_element;
		if(current->previousptr!=nullptr)
		{
			first_element=find_first_element(current);
			Node* previous=current->previousptr;
			if(previous->pos>static_cast<int>(N))
			{
				for(int x=current->pos;x>0;--x)
				{
					current->elements[x]=current->elements[x-1];
					current->pointers[x+1]=current->pointers[x];
				}
				current->pointers[1]=current->pointers[0];
				current->elements[0]=first_element;
				previous->pointers[previous->pos]->parent=current;
				current->pointers[0]=previous->pointers[previous->pos];
				previous->pointers[previous->pos]=nullptr;
				--previous->pos;
				previous->elements[previous->pos]=nullptr;
				++current->pos;
				update_inner(current->parent, *first_element, find_first_element(current));
			}
			else
			{
				previous->elements[previous->pos]=first_element;
				current->pointers[0]->parent=previous;
				previous->pointers[previous->pos+1]=current->pointers[0];
				current->pointers[0]=nullptr;
				for(int x=previous->pos+1;x<previous->pos+static_cast<int>(N);++x)
				{
					previous->elements[x]=current->elements[x-previous->pos-1];
					current->elements[x-previous->pos-1]=nullptr;
					current->pointers[x-previous->pos]->parent=previous;
					previous->pointers[x+1]=current->pointers[x-previous->pos];
					current->pointers[x-previous->pos]=nullptr;
				}
				previous->pos+=N;
				if(current->nextptr!=nullptr)
				{
					update_inner(current->parent, *first_element, find_first_element(current->nextptr));
					current->previousptr->nextptr=current->nextptr;
					current->nextptr->previousptr=previous;
					erase_inner(current->parent, *current->nextptr->elements[0]);
				}

				else
				{
					erase_inner(current->parent, *first_element);
				}
				delete current;
			}
		}
		else
		{
				first_element=find_first_element(current->nextptr);
				Node* next=current->nextptr;
				if(next->pos>static_cast<int>(N))
				{
					current->elements[current->pos]=first_element;
					next->pointers[0]->parent=current;
					current->pointers[current->pos+1]=next->pointers[0];
					for(int x=0;x<next->pos;++x)
					{
						next->elements[x]=next->elements[x+1];
						next->pointers[x]=next->pointers[x+1];
					}
					next->pointers[next->pos]=next->pointers[next->pos+1];
					++current->pos;
					--next->pos;
					update_inner(current->parent, *first_element, find_first_element(next));
				}
				else
				{
					next->pointers[N*2]=next->pointers[N];
					for(int x=next->pos+N-1; x>static_cast<int>(N)-1;--x)
					{
						next->elements[x]=next->elements[x-next->pos];
						next->pointers[x]=next->pointers[x-next->pos];
					}
					next->elements[next->pos-1]=first_element;
					
					for(int x=0;x<current->pos;++x)
					{
						next->elements[x]=current->elements[x];
						current->elements[x]=nullptr;
						current->pointers[x]->parent=next;
						next->pointers[x]=current->pointers[x];
						current->pointers[x]=nullptr;
					}
					current->pointers[current->pos]->parent=next;
					next->pointers[current->pos]=current->pointers[current->pos];
					current->pointers[current->pos]=nullptr;
					next->pos+=N;
					update_inner(current->parent, *first_element, next->elements[0]);
					next->previousptr=nullptr;
					erase_inner(current->parent, *first_element);
					delete current;
				}
		}
	}

public:

	ADS_set()
	{
		this->root=new Node;
		number_elements=0;
	}
	
	ADS_set(const ADS_set& other): ADS_set()
	{
		for(const auto it: other)
		{
			this->insert(it);
		}
	}
	
	~ADS_set()
	{
		delete root;
	}

	template<typename InputIt> ADS_set(InputIt first, InputIt last) : ADS_set()
	{
		while(first!=last)
		{
			this->insert(*first);
			++first;
		}
	}
	
	ADS_set(std::initializer_list<key_type> ilist) : ADS_set(std::begin(ilist), std::end(ilist)) {}
	
	std::pair<iterator,bool> insert(const key_type& key)
	{
		Node* current= find_leaf(root, key);
		int pos=find_pos(current, key);
		if(pos!=0  && key_equal{}(*current->elements[pos-1],key))
		{
			iterator it(current,pos-1);
			std::pair<iterator, bool> temp(it, false);
			return temp;
		}
		int ptr=pos;
		Node* it_node=current;
		if(current->pos==static_cast<int>(N)*2)
		{
			Node* next_ptr= current->nextptr;

			current->nextptr=new Node;
			current->nextptr->previousptr=current;
			if(next_ptr!=nullptr)
			{
				current->nextptr->nextptr=next_ptr;
				next_ptr->previousptr=current->nextptr;
			}
			next_ptr=current->nextptr;

			if(pos<static_cast<int>(N)+1)
			{
				for(size_type x=N; x<N*2;++x)
				{
					next_ptr->elements[x-N]=current->elements[x];
					current->elements[x]=nullptr;
				}
				for(int x=static_cast<int>(N); x>pos;--x)
				{
					current->elements[x]=current->elements[x-1];
				}
				current->elements[pos]=new value_type(key);
				
			}
			else
			{
				for(int x=static_cast<int>(N)+1; x<pos;++x)
				{
					next_ptr->elements[x-(static_cast<int>(N)+1)]=current->elements[x];
					current->elements[x]=nullptr;
				}
				next_ptr->elements[pos-(static_cast<int>(N)+1)]=new value_type(key);
				for(size_type x=pos; x<N*2;++x)
				{
					next_ptr->elements[x-N]=current->elements[x];
					current->elements[x]=nullptr;
				}
				ptr=ptr-static_cast<int>(N)-1;
				it_node=next_ptr;
				
			}
			current->pos=N+1;
			next_ptr->pos=N;
			Node* parent=current->parent;
			if(parent!=nullptr)
			{
				parent=current->parent;
				next_ptr->parent=parent;
				insert_value(parent, find_pos(parent, *next_ptr->elements[0]), next_ptr->elements[0], next_ptr);
			}
			else
			{
				
				current->parent=new Node;
				parent=current->parent;
				parent->elements[0]=next_ptr->elements[0];
				parent->pointers[0]=current;
				parent->pointers[1]=next_ptr;				
				parent->children=true;
				next_ptr->parent=parent;
				root=parent;
				parent->pos=1;
				
			}			
		}
		else
		{
			for(int x=current->pos; x>pos;--x)
			{
				current->elements[x]=current->elements[x-1];
			}
			current->elements[pos]=new value_type(key);
			++current->pos;
		}

		++number_elements;
		iterator it(it_node, ptr);
		std::pair<iterator,bool> pair(it,true);
		
		return pair;
	}
	
	void insert(std::initializer_list<key_type> ilist)
	{
		this->insert(std::begin(ilist), std::end(ilist));
	}

	template<typename InputIt> void insert(InputIt first, InputIt last)
	{
		while(first!=last)
		{
			this->insert(*first);
			++first;
		}
	}
	
	size_type erase(const key_type& key)
	{
		Node* current=find_leaf(root, key);
		int pos=-1;

		for(int x=0;x<current->pos;++x)
			if(key_equal{}(*current->elements[x],key))
					pos=x;

		if(pos==-1)
			return 0;
		
		key_type* deleted_element=current->elements[pos];
		for(int x=pos;x<current->pos;++x)
		{
			current->elements[x]=current->elements[x+1];
		}
		--current->pos;
		current->elements[current->pos]=nullptr;
		if(current->pos <static_cast<int>(N) && current!=root)
		{
				value_type* first_element;
				if(current->previousptr!=nullptr)
				{
					if(current->previousptr->pos>static_cast<int>(N))
					{
						for(size_type x=N-1;x>0;--x)
						{
							current->elements[x]=current->elements[x-1];
						}
						current->elements[0]=current->previousptr->elements[current->previousptr->pos-1];
						++current->pos;
						--current->previousptr->pos;
						current->previousptr->elements[current->previousptr->pos]=nullptr;
						update_inner(current->parent, key, current->elements[0]);
					}
					else
					{
						Node* previous=current->previousptr;
						if(current->elements[0]!=nullptr)
							first_element=current->elements[0];
						else
							first_element=deleted_element;
						for(int x=previous->pos;x<previous->pos+static_cast<int>(N)-1;++x)
						{
							previous->elements[x]=current->elements[x-previous->pos];
							current->elements[x-previous->pos]=nullptr;
						}
						previous->pos+=N-1;
						if(current->nextptr!=nullptr) 
						{
							update_inner(current->parent,*first_element, current->nextptr->elements[0]);
							current->nextptr->previousptr=previous;
							current->previousptr->nextptr=current->nextptr;
							erase_inner(current->parent, *current->nextptr->elements[0]);
						}
						else
						{
							erase_inner(current->parent, *first_element);
						}
						delete current;
					}
				}
				else
				{
					first_element=current->nextptr->elements[0];
					Node* next=current->nextptr;
					if(next->pos>static_cast<int>(N))
					{
						current->elements[current->pos]=first_element;
						for(int x=0;x<next->pos-1;++x)
						{
							next->elements[x]=next->elements[x+1];
						}
						++current->pos;
						--next->pos;
						next->elements[next->pos]=nullptr;
						update_inner(current->parent, *first_element, next->elements[0]);
					}
					else
					{
						for(int x=next->pos+N-2; x>static_cast<int>(N)-2;--x)
						{
							next->elements[x]=next->elements[x-N+1];
						}
						for(int x=0;x<current->pos;++x)
						{
							next->elements[x]=current->elements[x];
							current->elements[x]=nullptr;
						}
						next->pos+=N-1;
						next->previousptr=nullptr;
						erase_inner(current->parent, *first_element);
						delete current;
					}
				}
		}
		else
		{
			if(pos==0)
			{
				if(current->previousptr!=nullptr)
					update_inner(current->parent, key, current->elements[0]);
			}
		}
		delete deleted_element;
		number_elements--;
		return 1;
		
	}

	void clear()
	{
		
		delete root;
		root=new Node;
		number_elements=0;
	}

	ADS_set& operator=(const ADS_set& other)
	{
		this->clear();
		for(const auto& it: other)
			this->insert(it);
		return *this;
	}
	ADS_set& operator=(std::initializer_list<key_type> ilist)
	{
		this->clear();
		for(const auto& it: ilist)
			this->insert(it);
		return *this;
	}

	size_type size() const
	{
		return number_elements;
	}

	bool empty() const
	{
		if(number_elements==0)
			return true;
		return false;
	}
	
	size_type count(const key_type& key) const
	{
		if(this->number_elements!=0)
		{
			Node* current= this->find_leaf(root, key);
			for(int x=0;x<current->pos;++x)
				if(key_equal{}(*current->elements[x],key))
						return 1;
		}
		return 0;
	}
	
	void swap (ADS_set & other)
	{
		Node* other_root=other.root;
		int number=other.size();
		other.root=this->root;
		other.number_elements=this->number_elements;
		this->root=other_root;
		this->number_elements=number;
	}

	iterator find(const key_type& key) const
	{
		Node* current= this->find_leaf(root, key);
		int pos;
		for(int x=0;x<current->pos;++x)
		{
			if(key_equal{}(*current->elements[x],key))
			{
				pos=x;
				iterator it(current, pos);
				return it;
			}
		}
		return this->end();
	}

	iterator begin() const
	{
		Node* current = root;
		
		while(current->children==true)
		{
			current = current->pointers[0];
		}
		int ptr=0;
		iterator it(current, ptr);
		return it;
	}

	iterator end() const
	{
		Node* current = root;
		while (current->children == true)
		{
			current = current->pointers[current->pos];
		}
		int ptr=current->pos;
		
		iterator it(current, ptr);
		return it;
	}

	void dump(std::ostream& o = std::cerr) const
	{
		o<<"\n**************************************\n";
		o<<"size: "<<this->size()<<"\n";
		o<<"oder: "<<N<<"\n---------------root------------------\n";
		Node* current;
		Node* current_init = root;
		do{
			current=current_init;
			if(current->children==false)
				o<<"\n---------------leaves----------------\n";
			int a;
			int x=0;
			do
			{	if(current->parent==nullptr)
					a= static_cast<int>(N*2+1);
				else
					a=current->parent->pos;
				o<<"pos:"<<current->pos<<" ->";
				for(int x=0;x<current->pos;x++)
				{
					o<<*current->elements[x]<<", ";
				}
				o<<"\n--------------------------------------\n";
				++x;
				if(x==a+1){
					o<<"^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n";
					x=0;
				}
				current=current->nextptr;
			}while(current!=nullptr);
			o<<"~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
			if(current_init->children)
			current_init=current_init->pointers[0];
			else
				break;
		}while(current_init!=nullptr);
		o<<"**************************************\n";
	}

};

template <typename Key, size_t N>
bool operator==(const ADS_set<Key,N>& lhs, const ADS_set<Key,N>& rhs)
{	
	if(lhs.size()!=rhs.size())
		return false;
	for(const auto& it: lhs)
	{
		if(rhs.count(it)==0)
		{
			return false;
		}
	}
	return true;
}

template <typename Key, size_t N>
bool operator!=(const ADS_set<Key,N>& lhs, const ADS_set<Key,N>& rhs)
{
	if(lhs.size()!=rhs.size())
		return true;
	for(const auto& it: lhs)
	{
		if(rhs.count(it)==0)
		{
			return true;
		}
	}
	return false;
}


template <typename Key, size_t N>
void swap(ADS_set<Key,N>& lhs, ADS_set<Key,N>& rhs)
{
	lhs.swap(rhs);
}