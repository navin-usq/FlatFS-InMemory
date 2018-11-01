# FlatFS-InMemory

Install fuse and sqlite3 library before running the file system.

All sqlite methods are omitted in the code. This code has similar methods like FlatFs-sqlite but the storage part of sqlite is omitted in the code. The use of sqlite libraries is only to avoid errors while buliding the code. Only basic features of flatfs are implemented here because this whole In-Memory Project is just for testing the performance without sqlite code.    

command to build FlatFS: g++ flatFS.cpp -o flatFS 'pkg-config fuse --cflags --libs' -l sqlite3

(check below link for the error " pkg-config --cflags opencv: No such file or directory " - https://stackoverflow.com/questions/20625096/pkg-config-cflags-opencv-no-such-file-or-directory )

command to run FlatFS: ./flatFS -f test (create a directory named test to mount flatfs)

NOTE: Please ignore errors shown in the Mounted directory(test) while executing commands (Example: while executing touch command it may show some error but it is from the HFS - just ignore it). Only consider error log in terminal where you run the FlatFS. This terminal will also show some log info.

To Test Performance: for 5000 files

Run the below code to measure create performance:

/usr/bin/time -f "\n\nProcess Time : %e\n" touch 'a:b'

Run the below code to measure read performance:

/usr/bin/time -f "\n\nProcess Time : %e\n" ls ?

Run the below code to measure update performance:

/usr/bin/time -f "\n\nProcess Time : %e\n" mv 'a:b' '<c:d'

To test these features you should comment below performance code

comment the code from 237 to 255 and 425 to 445 in FlatFs.cpp and then compile and run flatfs then open the mounted directory in terminal

Features Implemented: (Open the Mounted Directory - "test" in Terminal and execute the commands)

1. Creating files using "touch" command (example: touch 'unit1:change1,unit2:change2')
2. Use "ls ?" to list all files created.
3. Searching using "ls ?" command (example: ls '?unit1:change1'). You can also list all files by simply specifying "ls ?" (performs similar to "ls")
4. Replace spec for a file using "mv" command (example: mv 'unit1:change1,unit2:change2,unit3:change3' '<unit5:change5')
