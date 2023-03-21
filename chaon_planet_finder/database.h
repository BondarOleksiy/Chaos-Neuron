#define POSITIVE 1
#define NEGATIVE 0

typedef struct pattern
{
	char *name;
	
	double *data;
	double *spectrum;
	int size;
	
}pattern;

typedef struct database
{
	pattern **input;
	double *output;
	
	int patterns_num;
	int size;
	
	int pattern_size;
	
	char *path;
	
}database;

database *init_database(char *input_patterns_dir, int pattern_size);
void free_database(database *base);
