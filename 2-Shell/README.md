# Shell
A rudimentary Command-Line Interpreter for Linux on C++ with the following functionalities

## 1. Run an external command
Run external commands by invoking their executable files through the execvp() syscall\
Example user commands:
```
./a.out myprog.c
cc –o myprog myprog.c
ls -l
```

## 2. Input-Output Redirection ( < / > )
Open the necessary file descriptor which will act as the source of input / output and duplicate it at the STDIN_FILENO (0) / STDOUT_FILENO (1)\
Use dup2() system call to carry out the redirection\
Example user commands:
```
sort < somefile.txt
./a.out < infile.txt > outfile.txt
```

## 3. Background Process Handling ( & )
Set the flag for Background Process to 1 if a syntactically correct presence of '&' operator is detected\
Do not wait for the child processes to complete in order to exhibit this functionality\
Example user commands:
```
./a.out &
./myprog < in.txt > out.txt &
```

## 4. Piped Processes Handling ( | )
Creates pipes using pipe() system call by initialising the PIPE File Descriptors at pipefd[0]-the READ end, and pipefd[1]-the WRITE end\
Output of the i-th process is fed as input to the (i+1)-th process; pipes are created by the parent just before fork() call to limit its scope to that particular child\
Each piped process is executed in a child process forked by the parent, and execvp() function is invoked with the necessary arguments\
Example user commands:
```
ls | more
cat abc.c | sort | more
```

## 5. Interrupting running commands
Ctrl-C : Halt the command running in shell during runtime by sending the SIGINT signal\
Ctrl-Z : Suspend the process in execution and continue it in background by trapping SIGTSTP signal

## 6. Searchable Shell History
Use “history” to show the most recent 1000 commands\
For searching through the shell history, press “Ctrl-R” and enter the search term

## 7. Auto-completion
Press “tab” key after writing the first few letters of a file to check the list of files starting with those characters\
termios library is used to store the input characters control asynchronous communications ports; this helps in detecting backspace, tab, Ctrl-R, etc.

## 8. multiWatch
Use the command to start executing cmd1, cmd2, cmd3... parallelly with multiple processes, and print their outputs along with command name and unix timestamp\
Fork processes and create (hidden) temporary files for each of the commands which are run; the command should write their output to the corresponding file\
Use inotify to monitor the directory. Read from the file changes using its file descriptor. Delete the file and remove the item from inotify watch list if the file is closed (execution is completed)\
Example user command:
```
multiWatch ["cmd1", "cmd2", "cmd3", ...]
multiWatch ["./a1", "./a2", "cat abc.txt", "ls"]
```
Sample Output :
```
"cmd1", current_time :
<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-
Output1
->->->->->->->->->->->->->->->->->->->
"cmd2", current_time:
<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-
Output2
->->->->->->->->->->->->->->->->->->->
"cmd1", current_time :
<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-
Output1
->->->->->->->->->->->->->->->->->->->
"cmd1", current_time :
<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-
Output1
->->->->->->->->->->->->->->->->->->->
```