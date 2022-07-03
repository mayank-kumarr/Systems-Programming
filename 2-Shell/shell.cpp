/*
    Group No. 40
    Mayank Kumar  (19CS30029)
    Shristi Singh (19CS10057)
*/

#include <bits/stdc++.h>
#include <unistd.h>
#include <ctime>
#include <cstdio>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <fstream>
#include <dirent.h>
#include <termios.h>
#include <sys/inotify.h>
using namespace std;

#define MAX 100
#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1024 * (EVENT_SIZE + 16))

int shellpid;
long long int cnt = 0;
set < int > processes, wpcs;
int SET = 0, WSET = 0;
int GID = 0, WGID = 0;
int sz = 0;
string inp_file, out_file;
bool flag1 = 0, flag2 = 0, flag3 = 0;
ifstream hist;
vector < string > vhist;

void handle_sigint(int sig) {
    signal(SIGINT, handle_sigint);
    WSET = 0;
    for (auto it: processes)
        kill(it, SIGINT);
    for (auto it: wpcs)
        kill(it, SIGINT);
    DIR * d;
    struct dirent * dir;
    if (d = opendir(".")) {
        while ((dir = readdir(d)) != NULL) {
            string fname = dir -> d_name;
            if (fname.find(".temp") == 0) {
                remove(fname.c_str());
            }
        }
        closedir(d);
    }
    fflush(stdout);
}

void handle_sigtstp(int sig) {
    signal(SIGTSTP, handle_sigtstp);
    for (auto it: processes)
        kill(it, SIGTSTP);
    for (auto it: processes)
        kill(it, SIGCONT);
    for (auto it: wpcs)
        kill(it, SIGTSTP);
    for (auto it: wpcs)
        kill(it, SIGCONT);
    printf("\n<<< ");
    fflush(stdout);
}

void separatePipes(vector < string > & v, string str) {
    int l = 0, r = 0, i = 0;
    while (1) {
        if (str[i] == '|') {
            v.push_back(str.substr(l, (r - l)));
            l = i + 1;
            r = i + 1;
        } else if (str[i] == '\0') {
            v.push_back(str.substr(l, (r - l)));
            return;
        } else
            r++;
        i++;
    }
}

void flags(vector < string > tokens) {
    flag1 = 0, flag2 = 0, flag3 = 0;
    inp_file = "", out_file = "";
    sz = 0;
    for (auto it = tokens.begin(); it != tokens.end(); ++it) {
        if (( * it) == "<") {
            flag1 = 1;
            inp_file = * (it + 1);
        } else if (( * it) == ">") {
            flag2 = 1;
            out_file = * (it + 1);
        } else if (( * it) == "&")
            flag3 = 1;
        if (flag1 == 0 && flag2 == 0 && flag3 == 0)
            sz++;
    }
    return;
}

void execute(vector < string > tokens, int in_fd, int out_fd) {
    int i, status;
    const char ** argv;
    pid_t pid, wpid;
    argv = new
    const char * [sz + 1];
    for (i = 0; i < sz; i++) {
        argv[i] = tokens[i].c_str();
    }
    argv[i] = NULL;

    if ((pid = fork()) == 0) {
        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        if (SET == 1)
            setpgrp();
        else
            setpgid(0, GID);
        if (WSET == 1)
            setpgrp();
        else
            setpgid(0, WGID);
        if (flag2) {
            out_fd = open(out_file.c_str(), O_WRONLY | O_TRUNC | O_CREAT, 0644);
            if (out_fd == -1) {
                perror("Error in output file\n");
                exit(1);
            }
        }
        if (out_fd != 1 && dup2(out_fd, 1) == -1) {
            perror("Error in output redirection\n");
            exit(1);
        }
        if (flag1) {
            in_fd = open(inp_file.c_str(), O_RDONLY);
            if (in_fd == -1) {
                perror("Error in input file\n");
                exit(1);
            }
        }
        if (in_fd != 0 && dup2(in_fd, 0) == -1) {
            perror("Error in input redirection\n");
            exit(1);
        }
        int exe = execvp(tokens[0].c_str(), (char ** ) argv);
        if (exe == -1) {
            cout << "Command not found\n";
            exit(0);
        }
    } else {
        if (SET == 1) {
            GID = pid;
            setpgid(pid, GID);
            tcsetpgrp(STDIN_FILENO, GID);
        } else {
            setpgid(pid, GID);
        }
        if (WSET == 1) {
            WGID = pid;
            setpgid(pid, WGID);
            tcsetpgrp(STDIN_FILENO, WGID);
        } else {
            setpgid(pid, WGID);
        }
        processes.insert(pid);
        wpcs.insert(pid);
    }
    return;
}

void history() {
    int n = 1000;
    for (int j = 0; j < min(n, (int) vhist.size()); j++) {
        cout << vhist[j] << "\n";
    }
}

void search(string str) {
    if (find(vhist.begin(), vhist.end(), str) != vhist.end()) {
        cout << str << "\n";
        return;
    }
    string ans;
    int match = 0, mch;
    bool flag = 0;
    for (int k = 0; k < vhist.size(); k++) {
        if (vhist[k].find(str) != std::string::npos) {
            cout << vhist[k] << '\n';
            flag = 1;
        }
    }
    if (flag == 0) {
        cout << "No match for search term in history\n";
    }
}

string longest_match(vector < string > v) {
    if (v.size() == 1)
        return v[0];
    string st = v[0];
    int i, j, n;
    for (i = 1; i < v.size(); i++) {
        n = min(st.length(), v[i].length());
        for (j = 0; j < n; j++) {
            if (st[j] != v[i][j])
                break;
        }
        st = st.substr(0, j);
    }
    return st;
}

void process_cmd(string str) {
    vector < string > v;
    separatePipes(v, str);
    int fd[2];
    int in_fd = 0;

    for (int i = 0; i < v.size(); i++) {
        if (i == 0) SET = 1;
        else SET = 0;
        vector < string > tokens;
        istringstream iss(v[i]);
        string s;

        while (iss >> s) {
            tokens.push_back(s);
        }
        if (pipe(fd) == -1) {
            perror("Error in piping\n");
            exit(1);
        }
        flags(tokens);
        if (i < v.size() - 1) {
            execute(tokens, in_fd, fd[1]);
            in_fd = fd[0];
        } else {
            execute(tokens, in_fd, 1);
        }
        close(fd[1]);
    }
    int status;
    pid_t wpid;
    if (flag3 == 0 && WSET == 0) {
        do {
            wpid = waitpid(-WGID, & status, WUNTRACED | WNOHANG);
            if (wpid > 0)
                wpcs.erase(wpid);
        } while (wpid != -1 && wpcs.size() != 0);
        if (WIFSTOPPED(status)) {
            kill(-WGID, SIGCONT);
        }
    }
    if (flag3 == 0) {
        do {
            wpid = waitpid(-GID, & status, WUNTRACED | WNOHANG);
            if (wpid > 0)
                processes.erase(wpid);
        } while (wpid != -1 && processes.size() != 0);
        if (WIFSTOPPED(status)) {
            kill(-GID, SIGCONT);
        }
    }
    tcsetpgrp(STDIN_FILENO, getpid());
}

void process_multiWatch(string str) {
    WSET = 1;
    signal(SIGINT, handle_sigint);
    signal(SIGTSTP, handle_sigtstp);
    vector < string > cmds;
    vector < string > fnames;
    map < string, int > fds;
    string str2 = "";
    bool ok = 0;
    long long int cnt2 = 0;
    int j = 0, length, wd;
    char buffer[BUF_LEN];
    for (j = 0; j < str.length(); j++) {
        if (str[j] == '\"' && ok == 0)
            ok = 1;
        else if (str[j] == '\"' && ok == 1) {
            str2 = str2 + " & > .temp" + to_string(cnt2) + ".txt";
            cmds.push_back(str2);
            str2 = "";
            cnt2++;
            ok = 0;
        } else if (ok == 1)
            str2 += str[j];
    }
    int inofd = inotify_init();
    for (j = 0; j < cnt2; j++) {
        string fname = ".temp" + to_string(j) + ".txt";
        fds[fname] = open(fname.c_str(), O_RDONLY);
        if (fds[fname] < 0) {
            int fdf1 = open(fname.c_str(), O_CREAT, 0644);
            close(fdf1);
            fds[fname] = open(fname.c_str(), O_RDONLY | O_NONBLOCK);
        }
        fnames.push_back(fname);
        wd = inotify_add_watch(inofd, fname.c_str(), IN_MODIFY | IN_CREATE | IN_DELETE | IN_CLOSE);
        if (wd < 0)
            perror("Error in adding to list");
        process_cmd(cmds[j]);
    }
    long long int cnt3 = cnt2;
    while (cnt2 > 0) {
        j = 0;
        length = read(inofd, buffer, BUF_LEN);
        if (length < 0)
            perror("Error in reading");
        while (j < length) {
            struct inotify_event * event = (struct inotify_event * ) & buffer[j];
            if (event -> mask & IN_MODIFY) {
                int l = 0;
                char buff[1000];
                bool f = false;
                while (1) {
                    l = read(fds[fnames[event -> wd - 1]], buff, sizeof(buff));
                    if (l <= 0)
                        break;
                    if (!f) {
                        time_t curr_time = time(0);
                        cout << "\n\"" << cmds[event -> wd - 1].substr(0, cmds[event -> wd - 1].find("&") - 1) << "\"" << ", " << asctime(localtime( & curr_time));
                        cout << "<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-<-\n";
                        f = true;
                    }
                    cout << buff;
                    bzero(buff, 1000);
                }
                if (f)
                    cout << "->->->->->->->->->->->->->->->->->->->\n";
            } else if (event -> mask & IN_CLOSE) {
                cnt2--;
            }
            j += EVENT_SIZE + event -> len;
        }
    }
    (void) inotify_rm_watch(inofd, wd);
    (void) close(inofd);
    for (j = 0; j < cnt3; j++) {
        string fname = ".temp" + to_string(j) + ".txt";
        remove(fname.c_str());
    }
    WSET = 0;
}

void shell() {
    struct termios old_tio, new_tio;
    char c;

    while (1) {
        tcgetattr(STDIN_FILENO, & old_tio);
        new_tio = old_tio;
        new_tio.c_lflag &= (~ICANON & ~ECHO);
        new_tio.c_cc[VMIN] = 1;
        new_tio.c_cc[VTIME] = 0;
        tcsetattr(STDIN_FILENO, TCSANOW, & new_tio);
        cnt++;
        int i = 0;
        string line;
        hist.open("history.txt");
        vhist.clear();
        while (getline(hist, line)) {
            vhist.push_back(line);
            i++;
        }
        processes.clear();
        cout << "<<< ";
        string str = "";

        int flag = 0;
        int num = 0;
        string buff;

        while ((c = getchar()) != '\n') {
            if (c == 127 || c == 8) {
                if (num > 0) {
                    printf("\b \b");
                    str.pop_back();
                    num--;
                } else
                    continue;
            } else if (c == 18) {
                hist.close();
                flag = 1;
                break;
            } else if (c == '\t') {
                int spc = str.find_last_of(" ");
                int sls = str.find_last_of("/");
                spc = (spc > str.length()) ? -1 : spc;
                sls = (sls > str.length()) ? -1 : sls;
                int mx = max(spc, sls);
                string str2 = str.substr(mx + 1);
                DIR * d;
                struct dirent * dir;
                vector < string > rstart;
                if (d = opendir(".")) {
                    while ((dir = readdir(d)) != NULL) {
                        string fname = dir -> d_name;
                        if (fname.find(str2) == 0)
                            rstart.push_back(fname);
                    }
                    closedir(d);
                    if (rstart.size() == 1) {
                        bool match = 1;
                        for (int ab = 0; ab < rstart[0].length(); ab++) {
                            if (match == 0)
                                printf("%c", rstart[0][ab]), str += rstart[0][ab], num += 1;
                            else if (rstart[0][ab] != str2[ab])
                                match = 0, printf("%c", rstart[0][ab]), str += rstart[0][ab], num += 1;
                        }
                    } else if (rstart.size() > 1) {
                        string lmatch = longest_match(rstart);
                        bool match = 1;
                        for (int ab = 0; ab < lmatch.length(); ab++) {
                            if (match == 0)
                                printf("%c", lmatch[ab]), str += lmatch[ab], num += 1;
                            else if (lmatch[ab] != str2[ab])
                                match = 0, printf("%c", lmatch[ab]), str += lmatch[ab], num += 1;
                        }
                        cout << "\n";
                        for (int ab = 0; ab < rstart.size(); ab++) {
                            cout << ab + 1 << ". " << rstart[ab] << endl;
                        }
                        cout << "Enter your choice: ";
                        char ch;
                        int num2 = 0;
                        string choice = "";
                        while ((ch = getchar()) != '\n') {
                            if (ch == 127 || ch == 8) {
                                if (num2 > 0) {
                                    printf("\b \b");
                                    choice.pop_back();
                                    num2--;
                                } else
                                    continue;
                            } else if (ch >= 48 && ch <= 57) {
                                choice = choice + ch;
                                printf("%c", ch);
                                num2++;
                            }
                        }
                        cout << endl;
                        if(choice != "")
                        {
                            num2 = stoi(choice);
                            match = 1;
                            cout << "<<< " << str;
                            for (int ab = 0; ab < rstart[num2 - 1].length(); ab++) {
                                if (match == 0)
                                    printf("%c", rstart[num2 - 1][ab]), str += rstart[num2 - 1][ab], num += 1;
                                else if (rstart[num2 - 1][ab] != lmatch[ab])
                                    match = 0, printf("%c", rstart[num2 - 1][ab]), str += rstart[num2 - 1][ab], num += 1;
                            }
                        }
                        else
                            cout << "<<< " << str;
                    }
                }
            } else if (c >= 32 && c <= 126) {
                str = str + c;
                num++;
                printf("%c", c);
            }
        }

        tcsetattr(STDIN_FILENO, TCSANOW, & old_tio);

        if (flag == 1) {
            cout << "Enter search term: ";
            string st;
            getline(cin, st);
            search(st);
            continue;
        }
        cout << "\n";
        ofstream out("outfile.txt");

        if (str == "exit") {
            cout << "Exiting from SHELL\n";
            out << str << "\n";
            for (int j = 0; j < min(9999, (int) vhist.size()); j++) {
                out << vhist[j] << "\n";
            }
            out.close();
            hist.close();
            remove("history.txt");
            rename("outfile.txt", "history.txt");
            break;
        } else if (str == "history") {
            history();
        } else if (str.substr(0, str.find(" ")) == "multiWatch") {
            process_multiWatch(str);
        } else {
            process_cmd(str);
        }
        fflush(stdin);
        out << str << "\n";
        for (int j = 0; j < min(9999, (int) vhist.size()); j++) {
            out << vhist[j] << "\n";
        }
        out.close();
        hist.close();
        remove("history.txt");
        rename("outfile.txt", "history.txt");
    }
    exit(1);
}

int main() {
    shellpid = getpid();
    cout << "Welcome to SHELL:\n";
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    shell();
    return 0;
}