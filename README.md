# FlatFS-InMemory

Install fuse and sqlite3 library before running the file system.

All sqlite methods are omitted in the code. This code has similar methods like FlatFs-sqlite but the storage part of sqlite is omitted in the code. The use of sqlite libraries is only to avoid errors while buliding the code. Only basic features of flatfs are implemented here because this whole In-Memory Project is just for testing the performance without sqlite code.    

command to build FlatFS: g++ flatFS.cpp -o flatFS 'pkg-config fuse --cflags --libs' -l sqlite3

(check below link for the error " pkg-config --cflags opencv: No such file or directory " - https://stackoverflow.com/questions/20625096/pkg-config-cflags-opencv-no-such-file-or-directory )

command to run FlatFS: ./flatFS -f test (create a directory named test to mount flatfs)

NOTE: Please ignore errors shown in the Mounted directory(test) while executing commands (Example: while executing touch command it may show some error but it is from the HFS - just ignore it). Only consider error log in terminal where you run the FlatFS. This terminal will also show some log info.

To Test Performance: for 5000 files (Note: This module is only to test performance of Flatfs. Please do not try any other scenario other than below commands)

Run the below code to measure create performance:

/usr/bin/time -f "\n\nProcess Time : %e\n" touch 'create:value1'

Run the below code to measure read performance (reading 5000 files):

/usr/bin/time -f "\n\nProcess Time : %e\n" ls '?'

Run the below code to measure search performance:

/usr/bin/time -f "\n\nProcess Time : %e\n" ls '?name:zpr7R' (you can enter any file name, I have given a sample file name)

Run the below code to measure update performance:

/usr/bin/time -f "\n\nProcess Time : %e\n" mv 'create:value1' '<rename:newval'

Run the below code to measure read performance (reading 5000 files):

/usr/bin/time -f "\n\nProcess Time : %e\n" ls '?'

Run the below code to measure search performance:

/usr/bin/time -f "\n\nProcess Time : %e\n" ls '?nam:znHT5'

