#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>

#define LOCKED 0
#define LRU 1

// declare structure to hold block information (id, status, pointers, etc.)
struct Block {
	int block_num;
	int status; // 0=locked, 1=LRU
	struct Block* prev;
	struct Block* next;
	struct Block* nextInRow;
};

// declare hash table as dynamic array of block pointers
struct HashTable {
	int block_num;
	struct Block* block_ptr;
	struct HashTable* next;
};

int num_indices;
struct HashTable* hash_table;
int num_blocks;

// declare variables as linked list of blocks
struct Block* lru_list=NULL;
struct Block* locked_list=NULL;
struct Block* lru_end = NULL;
struct Block* locked_end = NULL;

//to display menu and get selection
int displayMenu()
{
	printf("Disk Block Cache\n-----------------\n");

	printf("1) Enter parameters \n2) Insert/access disk block in cache \n3) Delete disk block from cache \n4) Quit program");

	printf("\n\nEnter Selection: ");

	int sel;
	scanf("%d", &sel);

	return sel;
}

//To print the blocks
void print_blocks() {
	// declare local variables

	printf("\nIndex\tID\tStatus\tPrev\tNext\n");
	printf("------------------------------------\n");

	// for each row of table, print hash index and information (id, status, previous block in list, next block in list)
	for (int i = 0; i < num_indices; i++) {

		struct Block* entry = hash_table[i].block_ptr;

		while (entry != NULL) 
		{
			printf("%d\t", entry->block_num / num_indices);

			struct Block* block = entry;
			
			printf("%d\t%s\t", block->block_num, (block->status == 0)? "Locked":"LRU");

			if (block->prev != NULL) {
				printf("%d\t", block->prev->block_num);
			}
			else
				printf("\t");
			
			if (block->next != NULL) {
				printf("%d", block->next->block_num);
			}
			else
				printf("\t");
			
			printf("\n");
			entry = entry->nextInRow;
		}
	}

	printf("\n");
}

//function to create a block
struct Block* create_block(int block_id, int status) {
	struct Block* new_block = (struct Block*)malloc(sizeof(struct Block));
	new_block->block_num = block_id;
	new_block->status = status;
	new_block->next = NULL;
	new_block->prev = NULL;

	return new_block;
}

//OPTION 1
void enter_parameters()
{
	printf("Enter number of indices for hash table: ");
	scanf("%d", &num_indices);

	//allocate memory and initialize to null
	hash_table = (struct HashTable*) malloc(num_indices * sizeof(struct HashTable));
	for (int i = 0; i < num_indices; i++) {
		hash_table[i].block_ptr = NULL;
	}
	printf("\n");
}

//OPTION 2
void insert_block()
{
	// declare local variables
	int block_id, status, hash_idx;
	struct Block* block = NULL;

	// prompt for block id & status
	printf("Enter block ID: ");
	scanf("%d", &block_id);
	printf("Enter block status (0=LOCKED, 1=LRU): ");
	scanf("%d", &status);

	//get the index of row
	hash_idx = block_id / num_indices;

	struct Block *current_block = hash_table[hash_idx].block_ptr;
	block = create_block(block_id, status);

	// if hash table row is empty, add new block	
	if (current_block == NULL) {
		hash_table[hash_idx].block_ptr = block;
		hash_table[hash_idx].block_ptr->nextInRow = NULL;
	}

	// else search hash table row for entry
	else
	{
		// else traverse hash table row until either block is found or reached last block of row
		// check last block in hash table row for matching id
		// if block not found, add to end of hash table row
		while (current_block->nextInRow != NULL && current_block->block_num != block_id) {
			current_block = current_block->nextInRow;
		}
		
		if (current_block->block_num == block_id)
		{
			printf("Block exists.\n");
			return;
		}
		
		else
		{
			current_block->nextInRow = block;
			block->nextInRow = NULL;
		}
	}
	

	// if LOCKED status
		// if block already in Locked list, return
		// if locked list is empty, add to head of locked list, return
		// else traverse to end of Locked list & add block
	if (status == 0)
	{
		if (locked_list == NULL)
		{
			locked_list = block;
			locked_end = locked_list;
			printf("Block %d added to head of locked list\n", block_id);
		}
		else
		{
			struct Block* temp = locked_list;

			while (temp->next != NULL)
			{
				//if block exists, return
				if (temp->block_num == block_id)
					return;
				temp = temp->next;
			}

			locked_end->next = block;
			locked_end = locked_end->next;
			printf("Block %d added to locked list\n", block_id);
		}

	}
	// else LRU status
		// if LRU list is empty, add to head of LRU list, return			
		// else traverse to last block of LRU list
		// if block was not found in hash table row, add block to end of LRU list
		// else adjust LRU list, moving block to end of LRU list and adjust previous, next blocks
	else
	{
		if (lru_list == NULL)
		{
			lru_list = block;
			lru_end = lru_list;
			printf("Block %d added to head of LRU list\n", block_id);
		}
		else
		{
			struct Block* temp = lru_list;

			while (temp->next != NULL)
				temp = temp->next;

			//end of list
			if (temp->next == NULL)
			{
				lru_end->next = block;
				block->prev = lru_end;
				lru_end = lru_end->next;
				printf("Block %d added to LRU list\n", block_id);
				return;
			}
			else
			{
				struct Block* t = temp->prev->next;
				temp->prev->next = temp->next;
				temp->next->prev = t;
				temp->prev = lru_end;
				temp->next = NULL;
				lru_end = temp;
				return;
			}
			
		}

	}
}

//OPTION 3
void remove_block()
{
	// declare local variables
	// prompt for block id, set row to 1st digit of id
	int block_id, row;
	printf("Enter block id: ");
	scanf("%d", &block_id);
	row = block_id / num_indices;

	// if hash row is empty, return
	if (hash_table[row].block_ptr == NULL) {
		printf("\nEmpty Hash Table Row\n");
		return;
	}

	// if block is found at head of hash table, check status 
	if (hash_table[row].block_ptr->block_num == block_id) 
	{
		// if locked, return
		if (hash_table[row].block_ptr->status == LOCKED) {
			printf("Cannot remove locked block\n");
			return;
		}
		// else record found block & adjust hash table pointer to next block
		struct Block* temp = hash_table[row].block_ptr, *temp_prev=NULL;
		hash_table[row].block_ptr = hash_table[row].block_ptr->nextInRow;

		// if block is found at head of LRU list
		if (lru_list == temp)
		{
			// adjust LRU list, setting head to next block
			temp_prev->nextInRow = temp->nextInRow;
			lru_list = lru_list->next;

			if (lru_list->next != NULL)
				lru_list->prev = NULL;

			// free block & return
			free(temp);
			printf("Block %d removed from disk block cache\n", block_id);
			return;
		}

		// else adjust previous and next block pointers in LRU list
		// free block & return
		else
		{
			if (temp->next != NULL)
			{

				temp->prev->next = temp->next;
				temp->next->prev = temp->prev;
			}
			else
			{
				lru_end = temp->prev;
				lru_end->next = NULL;
			}

			printf("Block %d removed from disk block cache\n", block_id);
			free(temp);
			temp = NULL;
			return;
		}
	}
		
		
	// else traverse hash table row until block found or last block of hash table row reached
	else {
		struct Block* temp = hash_table[row].block_ptr, *temp_prev = NULL;

		while (temp->nextInRow != NULL && temp->block_num != block_id)
		{
			temp_prev = temp;
			temp = temp->nextInRow;
		}
			
		if (temp != NULL)
		{
			if (temp->block_num == block_id)
			{
				if (temp->status == 0)
					return;

				// else "remove" block from hash table row, adjusting pointers
				else
				{
					if (temp->nextInRow == NULL)
						temp_prev->nextInRow = NULL;

				}
			}
		}
		
	// else block not found, return
		else
		{
			return;
		}

	// if block is found at head of LRU list
		if (lru_list == temp)
		{
			// adjust LRU list, setting head to next block
			temp_prev->nextInRow = temp->nextInRow;
			lru_list = lru_list->next;

			if(lru_list->next!=NULL)
				lru_list->prev = NULL;

		// free block & return
			free(temp);
			printf("Block %d removed from disk block cache\n", block_id);
			return;
		}

		// else adjust previous and next block pointers in LRU list
		// free block & return
		else 
		{
			if (temp->next != NULL)
			{
				
				temp_prev->nextInRow = temp->nextInRow;

				temp->prev->next = temp->next;
				temp->next->prev = temp->prev;
			}
			else
			{
				temp_prev->nextInRow = temp->nextInRow;
				lru_end = temp->prev;
				lru_end->next = NULL;
			}
				
			printf("Block %d removed from disk block cache\n", block_id);
			free(temp);
			temp = NULL;
			return;
		}
	}
}

//OPTION 4
void quit()
{
	for (int i = 0; i < num_indices; i++)
	{
		struct Block* current = hash_table[i].block_ptr, *temp;
		
		while (current != NULL)
		{
			temp = current;
			current = current->nextInRow;
			free(temp);
		}
	}
}

int main()
{
	int sel = 0;

	do {
		sel = displayMenu();

		switch (sel)
		{
		case 1:
			enter_parameters();
			break;

		case 2:
			insert_block();
			print_blocks();
			break;

		case 3:
			remove_block();
			print_blocks();
			break;

		case 4:
			printf("Quitting program...");
			quit();
			break;

		default: printf("\nInvalid selection. Try Again.\n");
			break;
		}

	} while (sel != 4);



	return 0;

}