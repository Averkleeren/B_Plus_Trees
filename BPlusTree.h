/* ASSUMPTIONS:
1. When a node must be split and has an odd number of keys, the extra key will go to the new right-hand node.
2. There will never be a duplicate key passed to insert.
*/

#include <vector>
#include <algorithm>
#ifndef DS_BPLUSTREE
#define DS_BPLUSTREE

///////////////////////////////////////////////////////////////////////////////
//DO NOT CHANGE THIS CODE
///////////////////////////////////////////////////////////////////////////////

//Do not implement the class here, this is a forward declaration. Implement at
//the bottom of the .h file
template <class T> class BPlusTree; 

template <class T>
class BPlusTreeNode{
public:
	BPlusTreeNode() : parent(NULL) {};
	bool is_leaf();
	bool contains(const T& key);

	//For grading only. This is bad practice to have, because
	//it exposes a private member variable.
	BPlusTreeNode* get_parent() { return parent; } 

	//We need this to let BPlusTree access private members
	friend class BPlusTree<T>; 
private:
	bool contains(const T& key,std::size_t low,std::size_t high);
	std::vector<T> keys;
	std::vector<BPlusTreeNode*> children;
	BPlusTreeNode* parent;
};

template <class T>
bool BPlusTreeNode<T>::is_leaf(){
	for(unsigned int i=0; i<children.size(); i++){
		if(children[i]){
			return false;
		}
	}
	return true;
}

template <class T>
bool BPlusTreeNode<T>::contains(const T& key){
	return contains(key,0,keys.size()-1);
}

//Private method takes advantage of the fact keys are sorted
template <class T>
bool BPlusTreeNode<T>::contains(const T& key,std::size_t low,std::size_t high){
	if(low>high){
		return false;
	}
	if(low==high){
		return key == keys[low];
	}
	std::size_t mid = (low+high)/2;
	if(key<=keys[mid]){
		return contains(key,0,mid);
	}
	else{
		return contains(key,mid+1,high);
	}
}

///////////////////////////////////////////////////////////////////////////////////////
//Your BPlusTree implementation goes below this point.
///////////////////////////////////////////////////////////////////////////////////////




template <class T>
class BPlusTree{
	public:
		//constructors
		BPlusTree(int aDegree) : main(NULL), degree(aDegree){};
		BPlusTree(const BPlusTree& b);
		void operator=(const BPlusTree& b);

		//deconstructors
        ~BPlusTree();

		//modifiers
		void insert(const T& element);
		void copy(BPlusTreeNode<T>* newNode,const BPlusTreeNode<T>* oldNode);
		//accessors
		BPlusTreeNode<T>* find(T element);

		//prints
		void print_sideways(std::ofstream& outfile);
		void print_BFS(std::ofstream& outfile);
		void print_recursive(std::ofstream& outfile, BPlusTreeNode<T>* node, int deep) const;
		void print_reccy2(std::vector<std::vector<std::vector <T> > > &printy, BPlusTreeNode<T>* node, int deep);

		//extra credit
		void print_sideways_pretty(std::ofstream& outfile);
		void print_BFS_pretty(std::ofstream& outfile);

	private:
        void clear(BPlusTreeNode<T>* node);
		void split(BPlusTreeNode<T>* node);
		
		BPlusTreeNode<T>* recursive_find(BPlusTreeNode<T>* node, const T& element);
		void insert_non_full(BPlusTreeNode<T>* node, const T& element);
		BPlusTreeNode<T>* main; 
		int degree;
};

template <class T>
BPlusTreeNode<T>* BPlusTree<T>::recursive_find(BPlusTreeNode<T>* node, const T& element){
	//search the end leafs first
	if(!node->is_leaf()){
		for(int i = 0; i<node->children.size(); i++){
			BPlusTreeNode<T>* result = recursive_find(node->children[i], element);
			if(result->contains(element)){
				return result;
			}
		}
	}
	//check if node contains element
	if(node->contains(element)){
		return node;
	}
	//safety
	return node;
}

template <class T>
BPlusTreeNode<T>* BPlusTree<T>::find(T element){
	if(main == NULL){
		return nullptr;
	}
	else{
		return recursive_find(main, element);
	}
}

template <class T>
void BPlusTree<T>::split(BPlusTreeNode<T>* node){
	//split node into 2 parts
	BPlusTreeNode<T>* left = new BPlusTreeNode<T>();
	BPlusTreeNode<T>* right = new BPlusTreeNode<T>();
	left->parent = node;
	right->parent = node;

	unsigned int middle = (degree/2);
	//move first half of the keys and children to the left node
	for(unsigned int i=0; i<middle; i++){
		if(node->keys.size() > i){
			left->keys.push_back(node->keys[i]);
		}
	}
	for(unsigned int i = 0; i< node->children.size()/2; i++){
		left->children.push_back(node->children[i]);
		node->children[i]->parent = left;
	}

	//move second half of keys and children to right
	for(unsigned int i = middle; i<degree; i++){
		if(node->children.size() != 0 && i == middle){
			continue;
		}
		if(node->keys.size() > i){
			right->keys.push_back(node->keys[i]);
		}
	}
	for(unsigned int i = node->children.size()/2; i<node->children.size(); i++){
		right->children.push_back(node->children[i]);
		node->children[i]->parent = right;
	}

	
	T mid = node->keys[middle];
	
	//create a new root if the node is main
	if(!node->parent){
		node->keys.clear();
		node->keys.push_back(mid);
		node->children.clear();
		node->children.push_back(left);
		node->children.push_back(right);
		left->parent = node;
		right->parent = node;
		main = node;
	}
	
	//move the median key up to the parent node
	else{
		BPlusTreeNode<T>* parent = node->parent;	
		typename std::vector<T>::iterator it = std::lower_bound(parent->keys.begin(), parent->keys.end(), mid);
		parent->keys.insert(it, mid);
		
		//remove node from parent's list of children
		typename std::vector<BPlusTreeNode<T>* >::iterator iter = \
		std::find(parent->children.begin(), parent->children.end(), node);
		
		if (iter != parent->children.end()) {
			iter = parent->children.erase(iter);
			delete node;
		}

		left->parent = parent;
		right->parent = parent;
		
		//find where to insert the left and right in the children list
		typename std::vector<BPlusTreeNode<T>*>::iterator itty = parent->children.begin();
		unsigned int count = 0;
		for(; count < parent->children.size(); itty++, count++){
			if(left->keys[left->keys.size()-1] <= parent->children[count]->keys[0]){
				break;
			}
		}
		parent->children.insert(itty, right);
		parent->children.insert(itty, left);


		if(parent->keys.size() >= degree){
			split(parent);
		}
	}
}

template <class T>
void BPlusTree<T>::insert_non_full(BPlusTreeNode<T>* node, const T& element){
	if(node->is_leaf()){
		//insert the key in sorted order into the leaf node
		typename std::vector<T>::iterator it = std::lower_bound(node->keys.begin(), node->keys.end(), element);
		node->keys.insert(it, element);
		if(node->keys.size() >= degree){
			split(node);
		}
	}
	else{
		//otherwise decide what path to go down
		unsigned int index = 0;
		if(element <= node->keys[0]){
			index = 0;
		}
		else if(element >= node->keys[node->keys.size()-1]){
			index = node->children.size()-1;
		}
		else{
			//if there are multiple nodes, find which gap, the element fits into, and that is the path to take
			if(node->keys.size() >= 2){
				for(int i = 0; i<node->keys.size()-1; i++){
					int j = i+1;
					if(element > node->keys[i] && element < node->keys[j]){
						index = j;
					}
				}
			}
		}
		//follow the path
		BPlusTreeNode<T>* child = node->children[index];
		insert_non_full(child, element);
	}
}

template <class T>
void BPlusTree<T>::insert(const T& element){
	if(main == NULL){
		main = new BPlusTreeNode<T>();
		main->keys.push_back(element);
	}
	else if(main->keys.size() >= (degree)){
		split(main);
	}
	else{
		insert_non_full(main, element);
	}
}

template <class T>
BPlusTree<T>::BPlusTree(const BPlusTree& other) {
    degree = other.degree;
    if (other.main) {
        main = new BPlusTreeNode<T>(*other.main);
        copy(main, other.main);
    }
}

template <class T>
void BPlusTree<T>::copy(BPlusTreeNode<T>* newNode,const BPlusTreeNode<T>* oldNode) {
    for (unsigned int i = 0; i < oldNode->children.size(); i++) {
        if(oldNode->children[i]) {
            BPlusTreeNode<T>* newChild = new BPlusTreeNode<T>(*oldNode->children[i]);
            newChild->parent = newNode;
            newNode->children[i] = newChild;
            copy(newChild, oldNode->children[i]);
        }
    }
}

template <class T>
void BPlusTree<T>::operator=(const BPlusTree& other) {
	degree = other.degree;
    if (other.main) {
        main = new BPlusTreeNode<T>(*other.main);
        copy(main, other.main);
    }
}

template <class T>
BPlusTree<T>::~BPlusTree(){
	clear(main);
}

template <class T>
void BPlusTree<T>::clear(BPlusTreeNode<T>* node){
	if(node){
		for (unsigned int i = 0; i < node->children.size(); i++) {
			if(node->children[i]) {
				clear(node->children[i]);
			}
		}
	}
	delete node;
}


template <class T>
void BPlusTree<T>::print_recursive(std::ofstream& outfile, BPlusTreeNode<T>* node, int deep) const{
	//if its a leaf node, just print
	if(node->is_leaf()){
		//proper indent level
		for(int i = 0; i < deep; i++){
			outfile << "\t";
		}

		//values
		for(int i = 0; i<node->keys.size(); i++){
			if(i > 0){
				outfile << ',';
			}
			outfile << node->keys[i];
		}
		outfile << std::endl;
	}
	else{
		//search the left paths
		for(int i = 0; i<node->children.size()/2; i++){
			print_recursive(outfile, node->children[i], deep+1);
		}
		//print indent levels
		for(int i = 0; i < deep; i++){
			outfile << "\t";
		}
		//print current values
		for(int i = 0; i<node->keys.size(); i++){
			if(i > 0){
				outfile << ',';
			}
			outfile << node->keys[i];
		}
		outfile << std::endl;
		//search right paths
		for(int i = node->children.size()/2; i<node->children.size(); i++){
			print_recursive(outfile, node->children[i], deep+1);
		}
	}
}

template <class T>
void BPlusTree<T>::print_sideways(std::ofstream& outfile){
	if(main == NULL){
		outfile << "Tree is empty." << std::endl;
	}
	else{
		print_recursive(outfile, main, 0);
	}
}

template <class T>
void BPlusTree<T>::print_reccy2(std::vector<std::vector<std::vector<T> > > &printy,\
 BPlusTreeNode<T>* node, int deep){
	//if there isnt a vector at that level yet, make one and add it
	if(printy.size() <= deep){
		std::vector<std::vector<T>> filler;
		printy.push_back(filler);
	}
	//if it is a leaf node, just add it to the level vector
	if(node->is_leaf()){
		std::vector<T> base_level;
		for(int i = 0; i<node->keys.size(); i++){
			base_level.push_back(node->keys[i]);
		}
		printy[deep].push_back(base_level);
	}

	//if it is not a leaf node, search the children and then add the current value to the level vector
	else{
		for(int i = 0; i<node->children.size(); i++){
			print_reccy2(printy, node->children[i], deep+1);
		}

		std::vector<T> base_level;
		for(int i = 0; i<node->keys.size(); i++){
			base_level.push_back(node->keys[i]);
		}
		printy[deep].push_back(base_level);
	}
}

template <class T>
void BPlusTree<T>::print_BFS(std::ofstream& outfile){
	if(main == NULL){
		outfile << "Tree is empty." << std::endl;
	}
	else{
		//I used a triple vector for this portion as the largest contains all the levels of a tree
		//the second contains all the values in a level, and the smallest contains all the
		//keys in a given node
		
		//this allowed me to not have to worry about formatting too much as all of it is handled
		//during the printing 

		std::vector<std::vector<std::vector <T> > > printy;
		print_reccy2(printy, main, 0);
		for(int i = 0; i<printy.size(); i++){
			for(int j = 0; j<printy[i].size(); j++){
				for(int k = 0; k<printy[i][j].size(); k++){
					if(k > 0){
						outfile << ',';
					}
					outfile << printy[i][j][k];
				}
				if(j+1 != printy[i].size())
				outfile << "\t";
			}
			outfile << std::endl;
		}
	}
}

template <class T>
void BPlusTree<T>::print_sideways_pretty(std::ofstream& outfile){
	// :)
	print_sideways(outfile);
}

template <class T>
void BPlusTree<T>::print_BFS_pretty(std::ofstream& outfile){
	// :p
	print_BFS(outfile);
}




#endif
