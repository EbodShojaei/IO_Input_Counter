# include <stdio.h>
# include <stdlib.h>
# include <string.h>

# define ASCII_SIZE 128 // ASCII character set size
# define HASH_SIZE 11 // Prime number for hash table size
# define HASH_MAGIC 37 // Magic number for hash function
# define HASH_LOAD_FACTOR 0.75 // Load factor for hash table
# define HASH_GROWTH_FACTOR 2 // Factor to grow hash table by
# define MAX_STRING_SIZE 64 // Max size of a string
# define MIN_STRING_SIZE 1 // Min size of a string


/**
 * A string and the number of times it has been seen.
 */
typedef struct {
	char *str;
	int count;
} Word;


/**
 * A hash table of strings.
 */
typedef struct {
	Word *dict;
	int size;
	int count; // Number of strings in the hash table
} HashTable;


/**
 * Handles errors to stderr.
 * @param message The error message to print.
 * @return void
 */
void handleError(char *message) {
	if (message) fprintf(stderr, "%s\n", message);
}


/**
 * Build look-up table for illegal chars.
 * Each element is a char, and the value is the index of the char in the string.
 * @param illegal The string of illegal characters.
 * @return The look-up table or NULL if memory could not be allocated
 */
int *createLookupTable(const unsigned char *illegal) {
	int *lookup = (int *)calloc(ASCII_SIZE, sizeof(int));
	if (!lookup) {
		handleError("Error: Could not allocate memory for look-up table.");
		return NULL;
	}
	for (int i = 0; i <= sizeof(illegal); i++) {
		lookup[(unsigned char) illegal[i]] = 1; // Marked as illegal char
	}
	return lookup;
}


/**
 * Frees the memory of a look-up table.
 * @param lookup The look-up table to free.
 * @return void
 */
int freeLookupTable(int *lookup) {
	if (lookup) {
		free(lookup);
		return 0;
	} else {
		handleError("Error: Null look-up table.");
		return -1;
	}
}


/**
 * Hash a string to an index in the hash table.
 * @param str The string to hash.
 * @param size The size of the hash table.
 * @return The index in the hash table.
 */
int hash(char *str, int size) {
	int hash = 0;
	for (int i = 0; str[i] != '\0'; i++) {
		hash = (hash * HASH_MAGIC + str[i]) % size;
	}
	return hash;
}


/**
 * Create a hash table.
 * @param size The size of the hash table.
 * @return The hash table.
 */
HashTable *createHashTable(int size) {
	HashTable *ht = (HashTable *)malloc(sizeof(HashTable));
	if (!ht) {
		handleError("Error: Could not allocate memory for hash table.");
		return NULL;
	}
	ht->size = size;
	ht->count = 0;
	ht->dict = (Word *)calloc(size, sizeof(Word));
	if (!ht->dict) {
		handleError("Error: Could not allocate memory for dictionary.");
		return NULL;
	}
	return ht;
}


/**
 * Frees the memory of a hash table.
 * @param ht The hash table to free.
 * @return void
 */
int freeHashTable(HashTable *ht) {
	if (ht) {
		for (int i = 0; i < ht->size; i++) free(ht->dict[i].str);
		free(ht->dict);
		free(ht);
		return 0;
	} else {
		handleError("Error: Null hash table.");
		return -1;
	}
}

int freeDictionary(Word *dict, int size) {
	if (dict) {
		for (int i = 0; i < size; i++) free(dict[i].str);
		free(dict);
		return 0;
	} else {
		handleError("Error: Null dictionary.");
		return -1;
	}
}


/**
 * Write data to output text file.
 * @param data The data to write.
 * @param filename The name of the output file.
 * @return int The success (0) or failure (-1) of writing to the file.
 */
int writeToFile(char *data, FILE *file) {
	if (!data) {
		handleError("Error: Null data.");
		return -1;
	}
	if (!file) {
		handleError("Error: Null file.");
		return -1;
	}
	fprintf(file, "%s", data);
	return 0;
}


/**
 * Validate the string.
 * @param str The string to validate.
 * @return int The success (0) or failure (-1) of the validation.
 */
int checkString(char *str) {
	if (!str) return -1;

	const int SIZE = strlen(str);
	if (SIZE < MIN_STRING_SIZE || SIZE > MAX_STRING_SIZE) return -1;
	return 0;
}


/**
 * Check if size is greater than 0.
 * @param size The size to check.
 * @return The size if it is greater than 0, else -1.
 */
int checkSize(int size) {
	if (size < 1) return -1;
	return size;
}


/**
 * Recalculate the load factor of the hash table.
 * @param ht The hash table to recalculate the load factor of.
 * @return The load factor or -1 if the hash table is null.
 */
float recalcLoadFactor(HashTable *ht) {
	if (!ht) {
		handleError("Error: Null hash table.");
		return -1;
	}
	return (float)(ht->count + 1) / ht->size;
}


/**
 * Grow the hash table.
 * @param ht The hash table to grow.
 * @return int The success (0) or failure (-1) of growing the hash table.
 */
int growHashTable(HashTable *ht) {
	int new_size = ht->size * HASH_GROWTH_FACTOR;
	Word *new_dict = (Word *)calloc(new_size, sizeof(Word));
	for (int i = 0; i < ht->size; i++) {
		if (ht->dict[i].str != NULL) {
			int index = hash(ht->dict[i].str, new_size);
			while (new_dict[index].str != NULL) {
				index = (index + 1) % new_size;
			}
			new_dict[index].str = ht->dict[i].str;
			new_dict[index].count = ht->dict[i].count;
		}
	}
	free(ht->dict);
	ht->dict = new_dict;
	ht->size = new_size;
	return 0;
}


/**
 * Add a string to the hash table.
 * Handles collisions by linear probing.
 * @param ht The hash table to add the string to.
 * @param str The string to add.
 * @return void
 */
void addString(HashTable *ht, char *str) {
	// Grow the hash table if the load factor is too high
	if (recalcLoadFactor(ht) > HASH_LOAD_FACTOR) growHashTable(ht);
	int index = hash(str, ht->size);
	int start_index = index;

	do {
		if (ht->dict[index].str == NULL) {
			// Found an empty slot
			ht->dict[index].str = strdup(str);
			ht->dict[index].count = 1;
			ht->count++;
			return;
		}
		if (strcmp(ht->dict[index].str, str) == 0) {
			// String already exists, so increment the count
			ht->dict[index].count++;
			return;
		}
		// Linear probing to find the next empty slot
		index = (index + 1) % ht->size;
	} while (index != start_index);
	
	handleError("Error: Could not add string to hash table.");
}


int fileReader(char *filename) {
	FILE *file = fopen(filename, "r");
	if (!file) {
		handleError("Error: Could not open file.");
		return -1;
	}

	int word_count = 0;
	char *word = (char *)malloc(MAX_STRING_SIZE * sizeof(char));
	return 0;
}



/**
 * Driver function.
 */
int main(int argc, char *argv[]) {
	// Check inputs
	// Read file
	// Get unique strings
	
	// Create a test hash table
	HashTable *ht = createHashTable(HASH_SIZE);

	// Print total number of strings
	printf("Total number of strings: %d\n", ht->count);
	
	// Print the current size of the hash table
	printf("Current size of the hash table: %d\n", ht->size);

	// Add test strings to the hash table
	addString(ht, "hello");
	addString(ht, "world");
	addString(ht, "hello");
	addString(ht, "world");
	addString(ht, "aaaaa");
	addString(ht, "bbbbb");
	addString(ht, "ccccc");
	addString(ht, "ddddd");
	addString(ht, "eeeee");
	addString(ht, "fffff");
	addString(ht, "ggggg");
	addString(ht, "hhhhh");
	addString(ht, "iiiii");
	addString(ht, "jjjjj");

	// Print the hash table
	for (int i = 0; i < ht->size; i++) {
		if (ht->dict[i].str != NULL) {
			printf("%s: %d\n", ht->dict[i].str, ht->dict[i].count);
		}
	}

	// Print total number of strings
	printf("Total number of strings: %d\n", ht->count);

	// Print the current size of the hash table
	printf("Current size of the hash table: %d\n", ht->size);

	// Free the hash table
	freeHashTable(ht);
	printf("Hash table status: %d\n", ht->size);

	const unsigned char COMMON_PUNCTUATION[] = {',', '*', ';', '.', ':', '(', '[', ']', ')'};
	int *lookup = createLookupTable(COMMON_PUNCTUATION);
	printf("Lookup table status: %d\n", lookup[',']);

	// Print the look-up table
	for (int i = 0; i < ASCII_SIZE; i++) {
		if (lookup[i] != 0) {
			printf("Illegal char: %c\n", i);
		}
	}
	freeLookupTable(lookup);
	printf("Lookup table status: %d\n", lookup[',']);


	return 0;
}

