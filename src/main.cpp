#include <iostream>
#include <map>
#include <stdlib.h>
#include <pwd.h>
#include <vector>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

#define FLAG_and 1
#define FLAG_or 2
#define FLAG_append 4
#define FLAG_pipe 8
#define FLAG_in 16

#define fd_set in_out[0] = 0;\
            in_out[1] = 0;\
            in_out[2] = 0;

using namespace std;

//function to convert a vector of char * to a
//char * [] for use as an execvp argument
char ** conv_vec(vector<char*> v)
{
    //create a char * [] of proper size
    char ** t = new char* [v.size() + 1];

    //counter to keep track of the position
    //for entering char*
    unsigned i = 0;
    char * temp = 0;

    for (; i < v.size(); ++i)
    {
        //first make char[] entry of the right length
        temp = new char[strlen(v.at(i)) + 1];
        //then copy the entire string from the vector
        //into the char *[]
        strcpy(temp, v.at(i));
        t[i] = temp;
    }

    //set the last position to a null character
    t[i] = NULL;

    return t;
}

void vec_delete(char** argv)
{
    int i = 0;

    for (; argv[i] != 0; ++i)
    {
        delete[] argv[i];
    }

    return;
}

char ** get_command(string& a, int& flag, char** in_out, map<int,
char*>& out_re, vector<bool>& b)
{
    //first make a copy of the command string
    //so that we can check what delimiter strtok
    //stopped at
    char * copy = new char [a.length() + 1];
    strcpy(copy, a.c_str());

    //create a vector because we do not know
    //how many tokens are in the command entered
    vector<char *> temp;

    //create an empty char * [] to return. It can
    //either be empty if there is no input or
    //filled later when we convert the vector
    //into an array
    char ** vec = NULL;

    //set a starting position to reference when
    //finding the position of the delimiter found
    char * begin = copy;

    //take the first token
    char * token = strtok(copy, " ;|&#<>");

    //for position of delimiter
    unsigned int pos;
    bool before_in_out = false;
    bool t_in = false;

    flag = 0;

    while (token != 0)
    {
        //the position of the delimiter with respect
        //to the beginning of the array
        if (!t_in)
        {
            pos = token - begin + strlen(token);
        }

        t_in = false;

        if (!before_in_out)
        {
            temp.push_back(token);
        }

        //to find out which delimiter was found
        //if it was the end, it will not go through this
        if (pos < a.size())
        {
            //store delimiter character found
            char delim = a.at(pos);

            if (delim == ' ')
            {
                while (pos < a.size() - 1 && delim == ' ')
                {
                    ++pos;
                    delim = a.at(pos);
                }
            }

            if (delim == '<' || delim == '>')
            {
                int n = 1;
                while (pos + n < a.size() && delim == a.at(pos + n))
                {
                    ++n;
                }
                if ((delim == '>' && n > 2) || ((delim == '<') &&
                    (n == 2 || n > 3)))
                {
                    cerr << "Rshell: Syntax error near '" << delim
                        << "'" << endl;
                    delete[] copy;
                    a.clear();
                    return vec;
                }
                if (delim == '>')
                {
                    bool is_num = true;
                    for (unsigned i = 0; i < strlen(token); ++i)
                    {
                        if (!isdigit(token[i]))
                        {
                            is_num = false;
                            break;
                        }
                    }
                    if (is_num)
                    {
                        int fd = atoi(token);
                        token = strtok(NULL, " ;|&<>");
                        if (token == 0)
                        {
                            cerr << "Rshell: Syntax error near '" << delim
                                << "'" << endl;
                            delete[] copy;
                            a.clear();
                            return vec;
                        }

                        char* tempy = new char[strlen(token) + 1];
                        strcpy(tempy, token);
                        out_re.insert(make_pair(fd, tempy));
                        if (n == 2)
                        {
                            b.push_back(true);
                        }
                        else
                        {
                            b.push_back(false);
                        }
                        temp.pop_back();
                        before_in_out = true;
                        continue;
                    }
                }
                else
                {
                    if (n == 3)
                    {
                        token = strtok(NULL, "\"");
                        if (token == 0)
                        {
                            cerr << "Rshell: Syntax error near '" <<
                            delim << "'" << endl;
                            delete[] copy;
                            a.clear();
                            return vec;
                        }
                        token = strtok(NULL, "\"");
                        if (token == 0)
                        {
                            cerr << "Rshell: Syntax error near '" <<
                            delim << "'" << endl;
                            delete[] copy;
                            a.clear();
                            return vec;

                        }
                        flag = flag | FLAG_in; //set flag
                        char* t = new char[strlen(token) + 1];
                        strcpy(t, token);
                        t[strlen(token)] = '\0';
                        if (in_out[0] != 0)
                        {
                            delete[] in_out[0];
                        }
                        in_out[0] = t;//put it into in_out[0] and continue
                        pos = token - begin + strlen(token) + 1;
                        while (pos < a.size() && a.at(pos) == ' ')
                        {
                            ++pos;
                        }
                        if (pos < a.size())
                        {
                            string all_delim = " ;|&<>";
                            if (all_delim.find(a.at(pos)) == string::npos)
                            {
                                cerr << "Rshell: Syntax error near '" <<
                                a.at(pos) << "'" << endl;
                                a.clear();
                                delete[] copy;
                                flag = 0;
                                return vec;
                            }
                        }

                        t_in = true;
                        before_in_out = true;
                        continue;
                    }
                }
                token = strtok(NULL, " ;|&<>");

                if (token == 0)
                {
                    cerr << "RShell: Syntax error near '" << delim
                        << "'" << endl; //error
                    delete[] copy;
                    a.clear();
                    return vec;
                }
                char* file = new char[strlen(token) + 1];
                strcpy(file, token);
                if (delim == '<')
                {
                    if (in_out[0] != 0)
                    {
                        delete[] in_out[0];
                    }
                    in_out[0] = file;
                    flag = flag & (~FLAG_in);
                }
                else
                {
                    if (in_out[1] != 0)
                    {
                        delete[] in_out[1];
                    }
                    in_out[1] = file;
                    if (n == 2)
                    {
                        flag = flag | FLAG_append;
                    }
                    else
                    {
                        flag = flag & (~FLAG_append);
                    }
                }

                before_in_out = true;
                continue;
            }

            before_in_out = false;

            //checking to see if there are two of the '|'
            //or '&'
            if (delim == '|' || delim == '&')
            {
                //increment this n as long as it is not the
                //end and they are equal
                int n = 1;
                while (pos + n < a.size() && delim
                        == a.at(pos + n))
                {
                    ++n;
                }

                if (n == 1 && delim == '|')
                {
                    flag = flag | FLAG_pipe;
                    //call piping
                }

                //if it is not 2, give an error message and clear
                //the vector so that no commands are executed
                else if (n != 2 || pos + n == a.size() || a.at(pos + n) == ';'
                        || a.at(pos + n) == '|' || a.at(pos + n) == '&')
                {
                    cerr << "Rshell: Syntax error near '" << delim
                        << "'" << endl;

                    temp.clear();
                }
            }

            //Now we know delim is either a delimiter
            //that signifies the end of a command
            //or is the first char of an argument
            //so we check
            if (delim == '|' || delim == '&' || delim == ';')
            {
                //so we are done with this command
                //now we convert it to a char**
                vec = conv_vec(temp);

                //            for (unsigned i = 0; vec[i] != 0; ++i)
                //            {
                //                cout << i << endl;
                //                cout << vec[i] <<  endl;
                //            }

                //remember to get rid of the dynamically allocated
                delete [] copy;

                //reset flag ('#' and ';' use 0 so reset it for
                //those and change for '|' and '&'
                flag = flag & ~(3);

                if (delim == '|' && !(flag & FLAG_pipe))
                {
                    //set flag bit
                    flag = flag | FLAG_or;
                }
                else if (delim == '&')
                {
                    //set flag bit to 2
                    flag = flag | FLAG_and;
                }

                a = a.erase(0, pos);

                return vec;
            }

            //this means that delim == a character
            //for the next argument so continue
        }
        token = strtok(NULL, " ;|&<>");
    }

    //so we have reached the end, set a to empty
    a.clear();

    //convert to proper char**
    vec = conv_vec(temp);

    //get rid of dynamically allocated
    delete [] copy;

    //we got to the end so the flag should be 0
    flag = flag & ~(3);

    return vec;
}

void display_prompt()
{
    //set size of hostname
    char host[50];

    //get the hostname and check to see if there was
    //an error
    int host_flag = gethostname(host, sizeof host);
    if(host_flag == -1)
    {
        perror("gethostname");
    }

    //get login and if there was an error give message
    char *login = (getpwuid(getuid()))->pw_name;
    if (login == 0)
    {
        perror("getlogin");
    }

    cout << login << '@' << host << "$ ";
}

struct sigaction act;
int child_pid = 0;
vector<int> stopped_pid;
int cin_stat = 0;

void sig_handler(int signum, siginfo_t *info, void* ptr)
{
    cout << "This processes pid: " << getpid() << endl;
    cout << "Child processes pid: " << child_pid << endl;

    if (getpid() == child_pid)
    {
        ;
    }
    else if (signum == SIGINT && child_pid != 0) //^C == 2
    {
        cerr << "Killing process: " << child_pid << endl;
        if(kill(child_pid, SIGKILL) == -1)
        {
            perror("kill");
            exit(1);
        }
    }
    else if (signum == SIGTSTP && child_pid != 0) //^Z == 20
    {
        stopped_pid.push_back(child_pid);
        cerr << '[' << stopped_pid.size() << "]+ Stopped" << endl;
        if(kill(child_pid, SIGSTOP) == -1)
        {
            perror("kill");
            exit(1);
        }
    }
    else if (child_pid == 0 && cin_stat == 1)
    {
        cout << endl;
    }
}

void get_paths(vector<char*>& paths, char* env_paths)
{
    char* temp = strtok(env_paths, ":");
    while (temp != NULL)
    {
        paths.push_back(temp);
        temp = strtok(NULL, ":");
    }
}

int main()
{
    int flag = 0;
    bool prev_pipe = false;
    int error_flag;

    act.sa_sigaction = sig_handler;
    act.sa_flags = SA_SIGINFO;

    char *env_PATH = getenv("PATH");
    if (env_PATH == NULL)
    {
        cerr << "Rshell: no path specified" << endl;
    }
    cout << env_PATH << endl;

    vector<char*> paths;
    get_paths(paths, env_PATH);

    while (1)
    {
        sigaction(SIGINT, &act, NULL);
        sigaction(SIGTSTP, &act, NULL);

        cin.clear();
        display_prompt();
        string command;
        cin_stat = 1;
        getline(cin, command);
        cin_stat = 0;

        //first try to find a '#' sign to indicate
        //a comment
        int p = command.find('#');

        //if there was a comment then delete everything
        //after that AKA set a substring from 0 to the '#'
        if (p != -1)
        {
            command = command.substr(0, p);
        }

        int fd_c[3];
        fd_c[0] = 0;
        fd_c[1] = 0;
        fd_c[2] = 0;
        int fd_p[3];
        fd_p[0] = 0;
        fd_p[1] = 0;
        fd_p[2] = 0;

        flag = 0;
        prev_pipe = false;

        //so continue doing and getting while there is
        //still an && or || or until the end of the
        //commands
        while(flag != 0 || command.size() != 0)
        {
            map<int, char*> out_re;
            vector<bool> b;
            char* in_out[3];
            fd_set
                //get the command in argv
                char** argv = get_command(command, flag, in_out, out_re, b);

            //if it is empty, then go back to prompt
            if (argv == 0 || argv[0] == NULL)
            {
                break;
            }

            //if the command is exit, exit program
            else if (strcmp(argv[0], "exit") == 0)
            {
                vec_delete(argv);
                delete [] argv;
                return 0;
            }
            else if (strcmp(argv[0], "bg") == 0)
            {
                if (stopped_pid.size() == 0)
                {
                    cout << "rshell: bg: no current background jobs" << endl;
                }
                else
                {
                    cout << '[' << stopped_pid.size() << "]+ Stopped" << endl;
                    for (unsigned i = 0; i < stopped_pid.size(); ++i)
                    {
                        cout << '[' << i + 1 << "]+ Stopped" << endl; //ALSO OUTPUT NAME/PATH OF PROCESS
                    }
                }
                continue;
            }
            else if (strcmp(argv[0], "fg") == 0)
            {
                if (stopped_pid.size() == 0)
                {
                    cout << "rshell: fg: no current background jobs" << endl;
                }
                else
                {
                    int pid = stopped_pid.at(stopped_pid.size() - 1);
                    stopped_pid.pop_back();
                    if(kill(pid, SIGCONT) == -1)
                    {
                        perror("kill");
                        exit(1);
                    }
                    if (wait(&error) == -1)
                    {
                        perror("wait");
                        exit(1);
                    }
                }
                continue;
            }

            error_flag = 0;

            //int std_in = 1;
            //int std_out = 2;
            //int std_err = 3;
            if (flag & FLAG_pipe)
            {
                int err = pipe(fd_c);
                if (err == -1)
                {
                    perror("pipe");
                    exit (1);
                }
            }


            //call the fork and check for error
            child_pid = fork();
            if (child_pid == -1)
            {
                perror("fork");

                exit(1);
            }
            //if it is the child do the command while checking
            //for error
            else if (child_pid == 0)
            {
                cout << child_pid << ' ' << getpid() << endl;
                if (prev_pipe)
                {
                    int err = close(0);
                    if (err == -1)
                    {
                        perror("close");
                        exit (1);
                    }

                    err = dup2(fd_p[0], 0);
                    if (err == -1)
                        cerr << fd_p << ": READ" << endl;
                    if (err == -1)
                    {
                        perror("dup2");
                        exit (1);
                    }

                    err = close(fd_p[0]);
                    if (err == -1)
                    {
                        perror("close");
                        exit (1);
                    }
                }

                if (flag & FLAG_pipe)
                {
                    int err = close(1);
                    if (err == -1)
                    {
                        perror("close");
                        exit (1);
                    }

                    err = dup2(fd_c[1], 1);
                    if (err == -1)
                    {
                        perror("dup2");
                        exit (1);
                    }

                    err = close(fd_c[1]);
                    if (err == -1)
                    {
                        perror("close");
                        exit (1);
                    }
                }
                if (flag & FLAG_in)
                {
                    int fd_temp[3];
                    int err = pipe(fd_temp);
                    if (err == -1)
                    {
                        perror("pipe");
                        exit (1);
                    }

                    err = close(0);
                    if (err == -1)
                    {
                        perror("close");
                        exit (1);
                    }

                    err = dup2(fd_temp[0], 0);
                    if (err == -1)
                    {
                        perror("dup2");
                        exit (1);
                    }

                    err = write(fd_temp[1], in_out[0], strlen(in_out[0]));
                    if (err == -1)
                    {
                        perror("write");
                        exit (1);
                    }

                    err = close(fd_temp[1]);
                    if (err == -1)
                    {
                        perror("close");
                        exit (1);
                    }
                }

                else if (in_out[0] != 0)
                {
                    int err = close(0);
                    if (err == -1)
                    {
                        perror("close");
                        exit (1);
                    }

                    int fd = open(in_out[0], O_RDONLY);
                    if (fd == -1)
                    {
                        perror("open");
                        exit(1);
                    }
                    if (fd != 0)
                    {
                        err = dup2(fd, 0);
                        if (err == -1)
                        {
                            perror("dup2");
                            exit (1);
                        }

                        err = close(fd);
                        if (err == -1)
                        {
                            perror("close");
                            exit (1);
                        }
                    }
                }

                int j = 0;

                for (map<int, char*>::iterator i = out_re.begin();
                        i != out_re.end(); ++i)
                {
                    int err = close(i->first);
                    if (err == -1)
                    {
                        perror("close");
                        exit (1);
                    }

                    int app = 0;
                    if (b.at(j))
                    {
                        app = O_APPEND;
                    }

                    int fd = open(i->second, O_WRONLY | O_CREAT | app, 0666);
                    if (fd == -1)
                    {
                        perror("open");
                        exit(1);
                    }

                    if (fd != i->first)
                    {
                        err = dup2(fd, i->first);
                        if (err == -1)
                        {
                            perror("dup2");
                            exit (1);
                        }

                        err = close(fd);
                        if (err == -1)
                        {
                            perror("close");
                            exit (1);
                        }
                    }

                    j++;
                }

                if (in_out[1] != 0)
                {
                    int err = close(1);
                    if (err == -1)
                    {
                        perror("close");
                        exit (1);
                    }

                    int app = 0;

                    if (flag & FLAG_append)
                    {
                        app = O_APPEND;
                    }

                    int fd = open(in_out[1], O_WRONLY | O_CREAT |
                            app, 0666);
                    if (fd == -1)
                    {
                        perror("open");
                        exit(1);
                    }
                    if (fd != 1)
                    {
                        err = dup2(fd, 1);
                        if (err == -1)
                        {
                            perror("dup2");
                            exit (1);
                        }
                        err = close(fd);
                        if (err == -1)
                        {
                            perror("close");
                            exit (1);
                        }
                    }
                }
                char* temp_comm = argv[0];
                for (unsigned k = 0; k < paths.size(); ++k)
                {
                    int slash = 1;
                    if (paths.at(k)[strlen(paths.at(k)) - 1] != '/')
                    {
                        slash = 2;
                    }
                    char* comm = new char[strlen(paths.at(k)) + strlen(temp_comm) + slash];
                    strcpy(comm, paths.at(k));
                    if (slash == 2)
                    {
                        strcat(comm, "/");
                    }
                    strcat(comm, temp_comm);

                    argv[0] = comm;

                    cout << argv[0] << endl;

                    execv(argv[0], argv);
                }

                perror("execv");
                exit(1);
                //int execvp_flag = execvp(argv[0], argv);
                //if (execvp_flag == -1)
                //{
                //    perror("execvp");
                //    exit(1);
                //}
            }

            //if parent then wait for child
            int wait_flag = wait(&error_flag);
            if (wait_flag == -1)
            {
                if (errno != EINTR)
                {
                    perror("wait");
                    exit(1);
                }
            }
            //            cout << "Error flag is " << error_flag << endl;
            //if there was an error
            child_pid = 0;
            for (map<int, char*>::iterator i = out_re.begin();
                    i != out_re.end(); ++i)
            {
                delete[] i->second;
            }
            if (in_out[0] != 0)
            {
                delete [] in_out[0];
            }
            if (in_out[1] != 0)
            {
                delete [] in_out[1];
            }
            if (prev_pipe)
            {
                prev_pipe = false;
            }
            if (flag & FLAG_pipe)
            {
                int err = close(fd_c[1]);
                if (err == -1)
                {
                    perror("close");
                    exit (1);
                }

                fd_p[0] = fd_c[0];

                prev_pipe = true;
            }

            vec_delete(argv);
            delete [] argv;
            if (error_flag != 0)
            {
                //if it was the first in an &&, go back
                //to prompt
                if (flag & FLAG_and)
                {
                    break;
                }
            }
            //if there wasn't an error and it was in an  ||
            //statement then go back to prompt
            else if (flag & FLAG_or)
            {
                break;
            }
        }
    }

    return 0;
}
