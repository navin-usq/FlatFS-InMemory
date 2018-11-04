#define FUSE_USE_VERSION 30 //version of fuse used
#include <fuse.h> //header files included for flatFS
#include<iostream>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sstream>
#include <fstream>
#include "sqlite3.h"
#include "global.h"
#include "sqlQueries.cpp" //supporting cpp files for flatFS
#include "attrMapper.cpp"
#include "flatFSUtility.cpp"
#include "flatFSPassThrough.cpp"

//store database location
char dataBaseLocation[100];

//varaiables for flatFs logic
void *file_buffer;
size_t file_size;
char files[5500][100];
int fileNums[5500];
char fileWithNames[5500][100];
int num = 1;
int totalfilenums = 0;
int totalfiles = 0;
int move = 0;
int isFile = 0;
int fileReadCount = 0;
int isFileFromCreate = 0;

//arrays and integers for spliting the input string into key value pairs
char fileNumsForMove[5500][100];
int fileNumsForMoveCount = 0;
char querySpecFileName[100];
int splitKeyValuePairLimit = 0;
char **splitKeyPath;
char **splitValuePath;

//struct object and integer to get values from database
int countglobal = 0;
struct my_struct s;

int file10000 = 0;
int fileRename10000 = 0;
int four = 0;
int fourRename = 0;
char inMemoryNames[5500][100];

int createRecordsForPerformance(mode_t mode, struct fuse_file_info *fi);
int updateRecordsForPerformance();

//Method to know what kind of command user has specified
static int do_getattr( const char *path, struct stat *st )
{
	st->st_uid = getuid(); //setting st properties
	st->st_gid = getgid();
	st->st_atime = time( NULL );
	st->st_mtime = time( NULL );

	switch(file_type(path))
	{
		case FILE_ROOT: //if ls command is specified
			st->st_mode = S_IFDIR | 0777;
			st->st_nlink = 2;
			break;
		case FILE_REG: //if a file command is specified
			st->st_mode = S_IFREG | 0777;
			st->st_nlink = 1;
			st->st_size = file_size;
			break;
		case FILE_LIST: //if a ls ? command is specified
			st->st_mode = S_IFDIR | 0777;
			st->st_nlink = 2;
			break;
		case FILE_NAME: //if touch cpommand is specified
			st->st_mode = S_IFDIR | 0777;
			st->st_nlink = 2;
			break;
		case FILE_MOVE: //if mv command is specified
			st->st_mode = S_IFDIR | 0777;
			st->st_nlink = 2;
			break;
		case FILE_NONE: //if unknown command is specified
			return -ENOENT;
	}

	return 0;
}

//method to list all files
static int do_readdir( const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi )
{
	//variables to count files
	fileReadCount = 0;
	int counter=0;
	int counterFile=0;

	(void) fi;
	(void) offset;

	if (file_type(path) == FILE_NAME) //search for a particular file
	{
		findFiles(path, 1, 0);
		for(int i=0; i<totalfiles; i++)
		{
			for(int j=0; j<fileNumsForMoveCount; j++)
			{
				if(strcmp(fileNumsForMove[j], files[i]) == 0)
				{std::cout << " " << i << "\n";
					filler(buffer, inMemoryNames[i], NULL, 0);
				}
			}
		}
		for(int jj=0; jj<fileNumsForMoveCount; jj++) //free array memory
		{
			for(int kk=0; kk<strlen(fileNumsForMove[jj]); kk++)
			{
				fileNumsForMove[jj][kk] = NULL;
			}
		}
		fileNumsForMoveCount = 0;
		//pointers for key and value pairs
		/*char **keyPath;
		char **valuePath;
		int searchCount = 1;

		for(int ii = 0; path[ii] != NULL; ii++) //count number of key value pairs
		{
			if(path[ii] == ',')
					searchCount++;
		}

		//dynamic memory allocation for the lenght of key value pairs
		keyPath = (char**)malloc((searchCount + 1) * sizeof(char*));
		valuePath = (char**)malloc((searchCount + 1) * sizeof(char*));

		//variables to count each key and value length
		int keylimit = 0;
		int vallimit = 0;
		int comlimit = 0;
		int valRegion = 0;
		int nSize = 0;

		for(int ii = 2; ii < strlen(path); ii++) //memory allocation for each key and value
		{
			if(path[ii] == ':')
			{
				keyPath[comlimit] = (char*)malloc(nSize);
				nSize = 0;
				valRegion = 1;
			}
			else if(path[ii] == ',' || path[ii + 1] == NULL)
			{
				if(path[ii + 1] == NULL)
					nSize++;
				valuePath[comlimit] = (char*)malloc(nSize);
				nSize = 0;
				valRegion = 0;
				comlimit++;
			}
			if(path[ii] != ':' && path[ii] != ',')
			{
				nSize++;
			}
		}

		comlimit = 0;

		for(int ii = 2; ii < strlen(path); ii++) //saving key and values in array
		{
			if(path[ii] == ':')
			{
				valRegion = 1;
				keyPath[comlimit][keylimit] = NULL;
				keylimit = 0;
			}
			else if(path[ii] == ',')
			{
				valRegion = 0;
				valuePath[comlimit][vallimit] = NULL;
				comlimit++;
				vallimit = 0;
			}
			if((path[ii] != ':' && path[ii] != ',') && valRegion == 0)
			{
				keyPath[comlimit][keylimit] = path[ii];
				keylimit++;
			}
			else if((path[ii] != ':' && path[ii] != ',') && valRegion == 1)
			{
				valuePath[comlimit][vallimit] = path[ii];
				vallimit++;
			}
		}

		valuePath[comlimit][vallimit] = NULL;
		comlimit++;

		for(int j=0; j<countglobal; ++j) //free struc object memory for getting information from database
		{
			free(s.argv1[j]);
			s.argv1[j] = NULL;
			free(s.argv2[j]);
			s.argv2[j] = NULL;
			free(s.argv3[j]);
			s.argv3[j] = NULL;
		}

		countglobal = 0;

		sqlite3 *db;
	  char *err_msg = 0;

	  int rc; //opening database
		if (FILE *file = fopen(dataBaseLocation, "r"))
		{
        fclose(file);
				rc = sqlite3_open(dataBaseLocation, &db);
    }

		if (rc != SQLITE_OK) //checking databse exist
		{
			std::cout << "open:error_file\n";
			sqlite3_close(db);
	    return 0;
	  }
		char *sql;
	  sql = "SELECT * FROM DataForFiles"; //query to select all key value pair records
	  rc = sqlite3_exec(db, sql, callback, 0, &err_msg); //database call

		if (rc != SQLITE_OK ) //checking query execution
		{
			std::cout << "open:error_data_file\n";
	    sqlite3_close(db);
	    return 0;
	  }
	  else
		{
			int iterationCount = 0;
			char *fileName;
			int countfileNames = 0;
			int n=0;
			int iteration = 0;
			int recordlocation = 0;

			for(int jj=0; jj<totalfiles; jj++) //comparing database key,value pair with the user input
			{
				for(int ii=0; ii<countglobal; ii++)
				{
					if(strcmp(files[jj], s.argv1[ii]) == 0)
					{
						for(int ij = 0; ij < comlimit; ij++)
						{
							if((strcmp(keyPath[ij],s.argv2[ii]) == 0) && (strcmp(valuePath[ij],s.argv3[ii]) == 0))
							{
								iterationCount++;
							}
						}
					}
				}
				//print start
				if(iterationCount == comlimit) //getting actual files for the search based on comparison result
				{
					for(int ii=0; ii<countglobal; ii++)
					{
						if(strcmp(files[jj], s.argv1[ii]) == 0)
						{
							n = n + strlen(s.argv2[ii]) + strlen(s.argv3[ii]) + 3;
							iteration++;
						}
					}
					fileName = (char*)malloc(n);
					int x = 0;
					for(int qq=0; qq<countglobal; qq++)
					{
						if(strcmp(files[jj], s.argv1[qq]) == 0)
						{
							if(x != 0)
							{
								fileName[x] = ',';
								x++;
							}
							for(int y=0; y<strlen(s.argv2[qq]); y++,x++)
							{
								fileName[x] = s.argv2[qq][y];
							}
							fileName[x] = ':';
							x++;
							for(int y=0; y<strlen(s.argv3[qq]); y++,x++)
							{
								fileName[x] = s.argv3[qq][y];
							}
						}
					}
					fileName[x] = NULL;
					isFile = 1;
					filler(buffer, fileName, NULL, 0); //printing the file name to the buffer
					recordlocation = recordlocation + iteration;
					iteration = 0;
					free(fileName);
					fileName = NULL;
					//print end
				}
				iterationCount = 0;
			}
		}

		sqlite3_close(db); // closiing database*/

		return 0;
	}
	else if(file_type(path) == FILE_ROOT || file_type(path) == FILE_LIST) //to list all files in the directory
	{
		/*for(int j=0; j<countglobal; ++j) //free memory for database key,value pairs
		{
			free(s.argv1[j]);
			s.argv1[j] = NULL;
			free(s.argv2[j]);
			s.argv2[j] = NULL;
			free(s.argv3[j]);
			s.argv3[j] = NULL;
		}

		countglobal = 0;

		sqlite3 *db;
    char *err_msg = 0;

    int rc;
		//opening database connection
		if (FILE *file = fopen(dataBaseLocation, "r"))
		{
        fclose(file);
				rc = sqlite3_open(dataBaseLocation, &db);
    }

    if (rc != SQLITE_OK) //checking databse connection
		{
				std::cout << "open:error\n";
        return 0;
    }

		char *sql;
    sql = "SELECT * FROM DataForFiles"; //selecting key value pairs
    rc = sqlite3_exec(db, sql, callback, 0, &err_msg); //database operations

    if (rc != SQLITE_OK ) //checking databse results
		{
    		sqlite3_close(db);
      	return 0;
    }
    else
		{
				char *fileName;
				int countfileNames = 0;
				int n=0;
				int iteration = 0;
				int recordlocation = 0;

				for(int jj=0; jj<totalfiles; jj++) //locating all key value pairs
				{
					for(int ii=0; ii<countglobal; ii++)
					{
						if(strcmp(files[jj], s.argv1[ii]) == 0)
						{
							n = n + strlen(s.argv2[ii]) + strlen(s.argv3[ii]) + 3;
							iteration++;
						}
					}
					fileName = (char*)malloc(n);

					int x = 0;
					for(int qq=0; qq<countglobal; qq++)
					{
						if(strcmp(files[jj], s.argv1[qq]) == 0)
						{
							if(x != 0)
							{
								fileName[x] = ',';
								x++;
							}
							for(int y=0; y<strlen(s.argv2[qq]); y++,x++)
							{
								fileName[x] = s.argv2[qq][y];
							}
							fileName[x] = ':';
							x++;
							for(int y=0; y<strlen(s.argv3[qq]); y++,x++)
							{
								fileName[x] = s.argv3[qq][y];
							}
						}
					}
					fileName[x] = NULL;
					isFile = 1;
					filler(buffer, fileName, NULL, 0); //printing key values pairs using buffer
					recordlocation = recordlocation + iteration;
					iteration = 0;
					free(fileName);
					fileName = NULL;
				}

			}

			sqlite3_close(db); //close database connection*/

			/*for(int ii=0; ii<totalfiles; ii++)
			{
				std::cout<< "\nname: " << fileWithNames[ii] << "\n";
			}*/

			for(int r=0; r<totalfiles; r++)
			{
				filler(buffer, inMemoryNames[r], NULL, 0);
			}

			return 0;
		}

		return 0;
}

//creating a file
static int do_create (const char *path, mode_t mode, struct fuse_file_info *fi )
{
	/*clock_t begin_m,end_m;
	double time_spent_m;

	begin_m = clock();*/

	char compareName[100];
	strncpy(compareName, path , 100);

	if(findFiles(path, 0, 1) == 1) // check already file exist
	{
		std::cout << "\nFile to touch already exist\n";
		return -ENOSPC;
	}
	/*for(int i=0; i<totalfiles; i++)
	{
		if(strcmp(fileWithNames[i],compareName) == 0)
		{
			return -ENOSPC;
		}
	}*/

		char fname[100];
		int x=0;
		for(int w=1;path[w]!=NULL;w++)
		{
			fname[x] = path[w];
			x++;
		}
			fname[x] = NULL;
		strncpy(inMemoryNames[totalfiles], fname, 100);

	strncpy(fileWithNames[totalfiles], path , 100);
	char fileName[100];
	int n=0,l=0,fileNum;
	totalfiles++;

	//fileNum = do_sql(0, attr, val); //call sql function to store key,value pair
	fileNum = num;
	num++;
	totalfilenums++;

	static int file_counter = 0;
	int res;
	if(file_counter >= 5500)
	{
		return -ENOSPC;
	}

	fileNums[file_counter] = fileNum;
	//file name - number to string conversion
	int test1 = fileNum;
	int test2 = test1, u =0, ascii = 48, test3 = 0;
	float numberOfZero = 0;
	int numCount = -1;
	while(test1 != 0)
	{
		test2 = test1 % 10;
		test1 = test1 / 10;
		if(test3 == 0)
		{
			test3 = test2;
		}
		else
		{
			test3 = test3 * 10;
			test3 = test3 + test2;
		}
	}
	test1 = test2 = test3;
	while(test1 != 0)
	{
		numCount++;
		test2 = test1 % 10;
		test1 = test1 / 10;
		for(int i=0; i!=test2; i++)
		{
			ascii++;
		}
		fileName[u] = ascii;
		ascii = 48;
		u++;
	}
	for(int ww=0; ww < (floor(log10 (abs(fileNum))) - numCount); ww++)
	{
		fileName[u] = 48;
		u++;
	}
	fileName[u] = '\0';
	//
	strncpy(files[file_counter], fileName , 100);
	file_counter++;

	createRecordsForPerformance(mode, fi);

	if(isFileFromCreate == 0) //check if it is a file create
		isFileFromCreate = 1;

		/*end_m = clock();
		time_spent_m = (double)(end_m-begin_m);
		std::cout << "\nsql time_m: " << time_spent_m << "\n";*/

	return 0;
}

static int do_rename (const char *path1, const char *path2)
{
	move = 0;
	if(path1[1] == '?' && path2[1] == '<') //if it is query spec and a replace spec
	{
		std::cout << "\nError: Set of files can not have the same name\n";
	}
	else if(path1[1] == '?' && path2[1] == '+') //if it is query spec and a add spec
	{
		findFiles(path1, 1, 0);//check file exist
		if(fileNumsForMove[0][0] == NULL)
		{
			std::cout << "\nNo File exist to add spec\n";
		}
		else
		{
			for(int i=0; i<fileNumsForMoveCount; i++)
			{
				getFileNameForQuerySpec(i); //get the file name
				if(checkSpecExistForAddSpec(querySpecFileName, path2) == 0) //check file spec exist
				{
					std::cout << "\nSpec already Exist in the File" << querySpecFileName << "\n";
				}
				else if(checkFileExistForAddSpec(querySpecFileName, path2, i) == 0) //check add spec exist
				{
					std::cout << "\nFile already exist with the name for " << querySpecFileName << "\n";
				}
				else
				{
					addOrDeleteFileSpec(path2, 0, i); //add file spec
				}
			}
		}
		for(int jj=0; jj<fileNumsForMoveCount; jj++) //free array memory
		{
			for(int kk=0; kk<strlen(fileNumsForMove[jj]); kk++)
			{
				fileNumsForMove[jj][kk] = NULL;
			}
		}
		fileNumsForMoveCount = 0;
	}
	else if(path1[1] == '?' && path2[1] == '_') //if it is a query spec and a delete spec
	{
		findFiles(path1, 1, 0); //check file exist
		if(fileNumsForMove[0][0] == NULL)
		{
			std::cout << "\nNo File exist to delete spec\n";
		}
		else
		{
			for(int i=0; i<fileNumsForMoveCount; i++)
			{
				getFileNameForQuerySpec(i); //get file name
				int returnVal = checkFileExistForDeleteSpec(querySpecFileName, path2, i); //check delete spec exist
				if(returnVal == 1)
				{
					std::cout << "\nDelete spec does not exist in the given file" << querySpecFileName << "\n";
				}
				else if(returnVal == 2)
				{
					std::cout << "\nEntire file is being deleted" << querySpecFileName << "\n";
				}
				else if(returnVal == 3)
				{
					std::cout << "\nFile already exist with the name" << querySpecFileName << "\n";
				}
				else
				{
					addOrDeleteFileSpec(path2, 1, i); //delete spec from file function
				}
			}
		}
		for(int jj=0; jj<fileNumsForMoveCount; jj++)
		{
			for(int kk=0; kk<strlen(fileNumsForMove[jj]); kk++)
			{
				fileNumsForMove[jj][kk] = NULL;
			}
		}
		fileNumsForMoveCount = 0;
	}
	else if(path1[1] != NULL && path2[1] == '<') //if it is a file speac and a replace spec
	{
		char pathToCheck[100];
		int qw = 0;
		pathToCheck[qw] = '/';
		qw++;
		for(int i=2; path2[i]!='/'; i++)
		{
			pathToCheck[qw] = path2[i];
			qw++;
		}
		pathToCheck[qw] = NULL;
		findFiles(pathToCheck, 0, 0); //check file exist
		if(fileNumsForMove[fileNumsForMoveCount][0] != NULL)
		{
			std::cout << "\nFile to replace already exist\n";
			for(int kk=0; kk<strlen(fileNumsForMove[fileNumsForMoveCount]); kk++)
			{
				fileNumsForMove[fileNumsForMoveCount][kk] = NULL;
			}
		}
		else
		{
			for(int kk=0; kk<strlen(fileNumsForMove[fileNumsForMoveCount]); kk++)
			{
				fileNumsForMove[fileNumsForMoveCount][kk] = NULL;
			}
			findFiles(path1, 0, 0);//check file exist
			if(fileNumsForMove[fileNumsForMoveCount][0] == NULL)
			{
				std::cout << "\nFile with spec does not exist\n";
			}
			else
			{
				//char pathToDelete[24];
				char pathToAdd[100];
				/*qw = 0;
				pathToDelete[qw] = '/';
				qw++;
				pathToDelete[qw] = '_';
				qw++;
				for(int i=1; path1[i]!=NULL; i++)
				{
					pathToDelete[qw] = path1[i];
					qw++;
				}
				pathToDelete[qw] = '/';
				qw++;
				pathToDelete[qw] = NULL;
				qw++;*/
				qw=0;
				pathToAdd[qw] = '/';
				qw++;
				for(int i=2; path2[i]!='/'; i++)
				{
					pathToAdd[qw] = path2[i];
					qw++;
				}
				pathToAdd[qw] = NULL;

				char fname[100];
				int x=0;
				for(int w=2;path2[w]!='/';w++)
				{
					fname[x] = path2[w];
					x++;
				}
					fname[x] = NULL;


				for(int i=0; i<totalfiles; i++)
				{
					if(strcmp(files[i], fileNumsForMove[fileNumsForMoveCount]) == 0)
					{
						strncpy(fileWithNames[i], pathToAdd, 100);
						strncpy(inMemoryNames[i], fname, 100);
						break;
					}
				} //modifiy file based on the new spec
				/*addOrDeleteFileSpec(pathToDelete, 1, fileNumsForMoveCount);
				addOrDeleteFileSpec(path2, 0, fileNumsForMoveCount);*/
				for(int kk=0; kk<strlen(fileNumsForMove[fileNumsForMoveCount]); kk++)
				{
					fileNumsForMove[fileNumsForMoveCount][kk] = NULL;
				}

				updateRecordsForPerformance();
			}
		}
	}
	else if(path1[1] != NULL && path2[1] == '+') //if it is a file spec and a add spec
	{
		findFiles(path1, 0, 0); //check file exist
		if(fileNumsForMove[fileNumsForMoveCount][0] == NULL)
		{
			std::cout << "\nFile to add spec does not exist\n";
		}
		else if(checkFileExistForAddSpec(path1, path2, 0) == 0) //check new file exist
		{
			std::cout << "\nFile already exist with the name\n";
		}
		else if(checkSpecExistForAddSpec(path1, path2) == 0) //check spec does not exist in file
		{
			std::cout << "\nSpec already Exist in the File\n";
		}
		else
		{
			addOrDeleteFileSpec(path2, 0, fileNumsForMoveCount); //add spec to the file
		}
		for(int kk=0; kk<strlen(fileNumsForMove[fileNumsForMoveCount]); kk++) //clear memory
		{
			fileNumsForMove[fileNumsForMoveCount][kk] = NULL;
		}
	}
	else if(path1[1] != NULL && path2[1] == '_') //if it is a file spec and a delete spec
	{
		findFiles(path1, 0, 0); //check file exist
		if(fileNumsForMove[fileNumsForMoveCount][0] == NULL)
		{
			std::cout << "\nFile to delete spec does not exist\n";
		}
		else
		{
			int returnVal = checkFileExistForDeleteSpec(path1, path2, 0); //check file exist for delete
			if(returnVal == 1)
			{
				std::cout << "\nDelete spec does not exist in the given file\n";
			}
			else if(returnVal == 2)
			{
				std::cout << "\nEntire file is being deleted\n";
			}
			else if(returnVal == 3)
			{
				std::cout << "\nFile already exist with the name\n";
			}
			else
			{
				addOrDeleteFileSpec(path2, 1, fileNumsForMoveCount); //delete spec from file
			}
		}
		for(int kk=0; kk<strlen(fileNumsForMove[fileNumsForMoveCount]); kk++) //clear memory
		{
			fileNumsForMove[fileNumsForMoveCount][kk] = NULL;
		}
	}

	return 0;
}

int createRecordsForPerformance(mode_t mode, struct fuse_file_info *fi)
{
	if(file10000 < 4999)
	{
			int i = file10000 + (499 * four);
			char test[100];
			char *random = gen_random(test, 5);
			char fileCreate[100];

			int re;
			//std::cout << "\nnum:" << " " << i;
//1
			if(i < 100)
			{
				strncpy(fileCreate, "/year:2010,unit:unit1,change:change1,version:version1,requirement:requirement1,name:", 100);
				re = 84;
			}
			else if(i < 200)
			{
				strncpy(fileCreate, "/year:2010,unit:unit1,change:change1,version:version1,requirement:requirement2,name:", 100);
				re = 84;
			}
			else if(i < 400)
			{
				strncpy(fileCreate, "/year:2010,unit:unit1,change:change1,version:version2,name:", 100);
				re = 59;
			}
			else if(i < 600)
			{
				strncpy(fileCreate, "/year:2010,unit:unit1,change:change2,name:", 100);
				re = 42;
			}
			else if(i < 800)
			{
				strncpy(fileCreate, "/year:2010,unit:unit2,name:", 100);
				re = 27;
			}
			else if(i < 1000)
			{
				strncpy(fileCreate, "/year:2010,unit:unit3,name:", 100);
				re = 27;
			}
//2
			else if(i < 1100)
			{
				strncpy(fileCreate, "/year:2011,unit:unit1,change:change1,version:version1,requirement:requirement1,name:", 100);
				re = 84;
			}
			else if(i < 1200)
			{
				strncpy(fileCreate, "/year:2011,unit:unit1,change:change1,version:version1,requirement:requirement2,name:", 100);
				re = 84;
			}
			else if(i < 1400)
			{
				strncpy(fileCreate, "/year:2011,unit:unit1,change:change1,version:version2,name:", 100);
				re = 59;
			}
			else if(i < 1600)
			{
				strncpy(fileCreate, "/year:2011,unit:unit1,change:change2,name:", 100);
				re = 42;
			}
			else if(i < 1800)
			{
				strncpy(fileCreate, "/year:2011,unit:unit2,name:", 100);
				re = 27;
			}
			else if(i < 2000)
			{
				strncpy(fileCreate, "/year:2011,unit:unit3,name:", 100);
				re = 27;
			}

//3
			else if(i < 2100)
			{
				strncpy(fileCreate, "/year:2012,unit:unit1,change:change1,version:version1,requirement:requirement1,name:", 100);
				re = 84;
			}
			else if(i < 2200)
			{
				strncpy(fileCreate, "/year:2012,unit:unit1,change:change1,version:version1,requirement:requirement2,name:", 100);
				re = 84;
			}
			else if(i < 2400)
			{
				strncpy(fileCreate, "/year:2012,unit:unit1,change:change1,version:version2,name:", 100);
				re = 59;
			}
			else if(i < 2600)
			{
				strncpy(fileCreate, "/year:2012,unit:unit1,change:change2,name:", 100);
				re = 42;
			}
			else if(i < 2800)
			{
				strncpy(fileCreate, "/year:2012,unit:unit2,name:", 100);
				re = 27;
			}
			else if(i < 3000)
			{
				strncpy(fileCreate, "/year:2012,unit:unit3,name:", 100);
				re = 27;
			}

//4
			else if(i < 3100)
			{
				strncpy(fileCreate, "/year:2013,unit:unit1,change:change1,version:version1,requirement:requirement1,name:", 100);
				re = 84;
			}
			else if(i < 3200)
			{
				strncpy(fileCreate, "/year:2013,unit:unit1,change:change1,version:version1,requirement:requirement2,name:", 100);
				re = 84;
			}
			else if(i < 3400)
			{
				strncpy(fileCreate, "/year:2013,unit:unit1,change:change1,version:version2,name:", 100);
				re = 59;
			}
			else if(i < 3600)
			{
				strncpy(fileCreate, "/year:2013,unit:unit1,change:change2,name:", 100);
				re = 42;
			}
			else if(i < 3800)
			{
				strncpy(fileCreate, "/year:2013,unit:unit2,name:", 100);
				re = 27;
			}
			else if(i < 4000)
			{
				strncpy(fileCreate, "/year:2013,unit:unit3,name:", 100);
				re = 27;
			}


//5
			else if(i < 4100)
			{
				strncpy(fileCreate, "/year:2014,unit:unit1,change:change1,version:version1,requirement:requirement1,name:", 100);
				re = 84;
			}
			else if(i < 4200)
			{
				strncpy(fileCreate, "/year:2014,unit:unit1,change:change1,version:version1,requirement:requirement2,name:", 100);
				re = 84;
			}
			else if(i < 4400)
			{
				strncpy(fileCreate, "/year:2010,unit:unit1,change:change1,version:version2,name:", 100);
				re = 59;
			}
			else if(i < 4600)
			{
				strncpy(fileCreate, "/year:2014,unit:unit1,change:change2,name:", 100);
				re = 42;
			}
			else if(i < 4800)
			{
				strncpy(fileCreate, "/year:2014,unit:unit2,name:", 100);
				re = 27;
			}
			else
			{
				strncpy(fileCreate, "/year:2014,unit:unit3,name:", 100);
				re = 27;
			}


for(int rr=0; rr<strlen(random); rr++)
			{
				fileCreate[re] = random[rr];
				re++;
			}
			fileCreate[re] = NULL;

			std::cout << "\n" << fileCreate << "\n";
file10000++;
			do_create(fileCreate,mode,fi);

}
/*else
	{
		if(four <= 8)
		{
			file10000 = 0;
		}
		four++;
	}*/

return 0;
}

int updateRecordsForPerformance()
{
		if(fileRename10000 < 4999)
				{

			int i = fileRename10000 + (499 * fourRename);
			char test[100];
			char *random = gen_random(test, 5);
			char fileCreate[100];

			int re;
			//std::cout << "\nnum:" << i;
//1
			if(i < 100)
			{
				strncpy(fileCreate, "/<year:2010,unit:unit1,change:change1,version:version1,requirement:requirement1,nam:", 100);
				re = 84;
			}
			else if(i < 200)
			{
				strncpy(fileCreate, "/<year:2010,unit:unit1,change:change1,version:version1,requirement:requirement2,nam:", 100);
				re = 84;
			}
			else if(i < 400)
			{
				strncpy(fileCreate, "/<year:2010,unit:unit1,change:change1,version:version2,nam:", 100);
				re = 59;
			}
			else if(i < 600)
			{
				strncpy(fileCreate, "/<year:2010,unit:unit1,change:change2,nam:", 100);
				re = 42;
			}
			else if(i < 800)
			{
				strncpy(fileCreate, "/<year:2010,unit:unit2,nam:", 100);
				re = 27;
			}
			else if(i < 1000)
			{
				strncpy(fileCreate, "/<year:2010,unit:unit3,nam:", 100);
				re = 27;
			}

//2
			else if(i < 1100)
			{
				strncpy(fileCreate, "/<year:2011,unit:unit1,change:change1,version:version1,requirement:requirement1,nam:", 100);
				re = 84;
			}
			else if(i < 1200)
			{
				strncpy(fileCreate, "/<year:2011,unit:unit1,change:change1,version:version1,requirement:requirement2,nam:", 100);
				re = 84;
			}
			else if(i < 1400)
			{
				strncpy(fileCreate, "/<year:2011,unit:unit1,change:change1,version:version2,nam:", 100);
				re = 59;
			}
			else if(i < 1600)
			{
				strncpy(fileCreate, "/<year:2011,unit:unit1,change:change2,nam:", 100);
				re = 42;
			}
			else if(i < 1800)
			{
				strncpy(fileCreate, "/<year:2011,unit:unit2,nam:", 100);
				re = 27;
			}
			else if(i < 2000)
			{
				strncpy(fileCreate, "/<year:2011,unit:unit3,nam:", 100);
				re = 27;
			}

//3
			else if(i < 2100)
			{
				strncpy(fileCreate, "/<year:2012,unit:unit1,change:change1,version:version1,requirement:requirement1,nam:", 100);
				re = 84;
			}
			else if(i < 2200)
			{
				strncpy(fileCreate, "/<year:2012,unit:unit1,change:change1,version:version1,requirement:requirement2,nam:", 100);
				re = 84;
			}
			else if(i < 2400)
			{
				strncpy(fileCreate, "/<year:2012,unit:unit1,change:change1,version:version2,nam:", 100);
				re = 59;
			}
			else if(i < 2600)
			{
				strncpy(fileCreate, "/<year:2012,unit:unit1,change:change2,nam:", 100);
				re = 42;
			}
			else if(i < 2800)
			{
				strncpy(fileCreate, "/<year:2012,unit:unit2,nam:", 100);
				re = 27;
			}
			else if(i < 3000)
			{
				strncpy(fileCreate, "/<year:2012,unit:unit3,nam:", 100);
				re = 27;
			}

//4
			else if(i < 3100)
			{
				strncpy(fileCreate, "/<year:2013,unit:unit1,change:change1,version:version1,requirement:requirement1,nam:", 100);
				re = 84;
			}
			else if(i < 3200)
			{
				strncpy(fileCreate, "/<year:2013,unit:unit1,change:change1,version:version1,requirement:requirement2,nam:", 100);
				re = 84;
			}
			else if(i < 3400)
			{
				strncpy(fileCreate, "/<year:2013,unit:unit1,change:change1,version:version2,nam:", 100);
				re = 59;
			}
			else if(i < 3600)
			{
				strncpy(fileCreate, "/<year:2013,unit:unit1,change:change2,nam:", 100);
				re = 42;
			}
			else if(i < 3800)
			{
				strncpy(fileCreate, "/<year:2013,unit:unit2,nam:", 100);
				re = 27;
			}
			else if(i < 4000)
			{
				strncpy(fileCreate, "/<year:2013,unit:unit3,nam:", 100);
				re = 27;
			}


//5
			else if(i < 4100)
			{
				strncpy(fileCreate, "/<year:2014,unit:unit1,change:change1,version:version1,requirement:requirement1,nam:", 100);
				re = 84;
			}
			else if(i < 4200)
			{
				strncpy(fileCreate, "/<year:2014,unit:unit1,change:change1,version:version1,requirement:requirement2,nam:", 100);
				re = 84;
			}
			else if(i < 4400)
			{
				strncpy(fileCreate, "/<year:2010,unit:unit1,change:change1,version:version2,nam:", 100);
				re = 59;
			}
			else if(i < 4600)
			{
				strncpy(fileCreate, "/<year:2014,unit:unit1,change:change2,nam:", 100);
				re = 42;
			}
			else if(i < 4800)
			{
				strncpy(fileCreate, "/<year:2014,unit:unit2,nam:", 100);
				re = 27;
			}
			else
			{
				strncpy(fileCreate, "/<year:2014,unit:unit3,nam:", 100);
				re = 27;
			}


for(int rr=0; rr<strlen(random); rr++)
			{
				fileCreate[re] = random[rr];
				re++;
			}
			fileCreate[re] = '/';
			re++;
			fileCreate[re] = NULL;

			std::cout << "\nNew Name: " << fileCreate << "\n";
std::cout << "\nOld Name: " << fileWithNames[i] << "\n";
fileRename10000++;
			do_rename(fileWithNames[i],fileCreate);
				}

/*else
	{
		if(fourRename <= 8)
		{
			fileRename10000 = 0;
		}
		fourRename++;
	}*/
}

int main( int argc, char *argv[] )
{
	//std::cout << "\nEnter database location:\n";
	//std::cin >> dataBaseLocation;
	//calling corresponding fuse operation using structure
	static struct fuse_operations operations = {
      getattr : do_getattr,
      readlink : NULL,
      getdir : NULL,
      mknod : NULL,
      mkdir : NULL,
      unlink : NULL,
      rmdir : NULL,
      symlink : NULL,
      rename : do_rename,
      link : NULL,
      chmod : do_chmod,
      chown : do_chown,
      truncate : NULL,
      utime : NULL,
      open : NULL,
      read : NULL,
      write : NULL,
      statfs : NULL,
      flush : NULL,
      release : NULL,
      fsync : NULL,
      setxattr : NULL,
      getxattr : NULL,
      listxattr : NULL,
      removexattr : NULL,
      opendir : NULL,
      readdir : do_readdir,
      releasedir : NULL,
      fsyncdir : NULL,
      init : NULL,
      destroy : NULL,
      access : do_access,
      create : do_create,
      ftruncate : NULL,
      fgetattr : NULL,
      lock : NULL,
      utimens : do_utimens,
      bmap : NULL,
      flag_nullpath_ok : NULL,
      flag_nopath : NULL,
      flag_utime_omit_ok : NULL,
      flag_reserved : NULL,
      ioctl : NULL,
      poll : NULL,
      write_buf : NULL,
      read_buf : NULL,
      flock : NULL,
      fallocate : NULL
};
	return fuse_main( argc, argv, &operations, NULL );
}
