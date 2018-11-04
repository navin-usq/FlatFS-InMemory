
enum { //enum to represent each type of file command
	FILE_NONE,
	FILE_ROOT,
	FILE_REG,
	FILE_CREATE,
	FILE_LIST,
	FILE_NAME,
	FILE_MOVE
};

extern char dataBaseLocation[100]; //store database location
extern size_t file_size; //file size variable
extern char files[5500][100]; //variables to represent a file
extern int fileNums[5500];
extern char fileWithNames[5500][100];
extern int num;
extern int totalfilenums;
extern int totalfiles;
extern int move;
extern int isFile;
extern int fileReadCount;
extern int isFileFromCreate;

extern char fileNumsForMove[5500][100]; //variables to perform query specs
extern int fileNumsForMoveCount;
extern char querySpecFileName[100];
extern int splitKeyValuePairLimit;
extern char **splitKeyPath;
extern char **splitValuePath;

struct my_struct //struct to get key,value pairs from database
{
	int argc;
	char** argv1;
	char** argv2;
	char** argv3;
};

extern int countglobal;
extern struct my_struct s; //struct object
