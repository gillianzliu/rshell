#include <iostream>
#include <pwd.h>
#include <vector>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>

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

    for (; i < v.size(); ++i)
    {
        //first make char[] entry of the right length
        t[i] = new char[strlen(v.at(i))];
        //then copy the entire string from the vector
        //into the char *[]
        strcpy(t[i], v.at(i));
    }

    //set the last position to a null character
    t[i] = '\0';

    return t;
}

char ** get_command(string& a, int& flag)
{
    //cout << a << endl;

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

    //bool done = false;
    //set a starting position to reference when
    //finding the position of the delimiter found
    char * begin = copy;

    //take the first token
    char * token = strtok(copy, " ;|&#");

    //for position of delimiter
    unsigned int pos;

    while (token != 0)
    {
//        cout << token << endl;

        //the position of the delimiter with respect
        //to the beginning of the array
        pos = token - begin + strlen(token);

        //put the token at the end of the vector
        temp.push_back(token);

        //cout << pos << endl;

        //to find out which delimiter was found
        //if it was the end, it will not go through this
        if (pos < a.size())
        {
            //store delimiter character found
            char delim = a.at(pos);
//            cout << delim << endl;

            if (delim == ' ')
            {
                while (pos < a.size() - 1 && delim == ' ')
                {
                    ++pos;
                    delim = a.at(pos);
                }
            }

            //Now we know delim is either a delimiter
            //that signifies the end of a command
            //or is the first char of the next one
            //so we check
            if (delim == '|' || delim == '&' || delim == ';'
                || delim == '#')
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
                delete copy;

                //reset flag ('#' and ';' use 0 so reset it for
                //those and change for '|' and '&'
                flag = 0;

                if (delim == '|')
                {
                    //set flag bit
                    flag = 1;
                }
                else if (delim == '&')
                {
                    //set flag bit to 2
                    flag = 2;
                }
                else if(delim == '#')
                {
                    pos = 0;
                    a = "";
                }

//            cout << flag << endl;
            a = a.substr(pos + 1, a.size() - pos + 1);

            return vec;
            }

            //this means that delim == a character
            //for the next argument so continue
        }

        token = strtok(NULL, " ;|&");
    }

    //so we have reached the end, set a to empty
    a = "";

    //convert to proper char**
    vec = conv_vec(temp);

    //get rid of dynamically allocated
    delete copy;

    //we got to the end so the flag should be 0
    flag = 0;

    return vec;
}

void display_prompt()
{
    char host[15];

    int host_flag = gethostname(host, sizeof host);
    if(host_flag == -1)
    {
        perror("gethostname");
    }
    char *login = (getpwuid(getuid()))->pw_name;
    if (login == 0)
    {
        perror("getlogin");
    }

    cout << login << '@' << host << "$ ";
}

int main()
{
    int flag = 0;
    int error_flag;
    while (1)
    {

        display_prompt();
        string command;
        getline(cin, command);

        //first try to find a '#' sign to indicate
        //a comment
        int p = command.find('#');

        if (p != -1)
        {
            command = command.substr(0, p);
        }

        while(flag != 0 || command != "")
        {
            char ** argv = get_command(command, flag);

//            cout << command << endl;
//            cout << flag << endl;

            if (argv[0] == NULL)
            {
                break;
            }

            else if (strcmp(argv[0], "exit") == 0)
            {
                return 0;
            }

//            for (int i = 0; argv[i] != 0; ++i)
//            {
//                cout << argv[i] << endl;
//            }

            error_flag = 0;

            int fork_flag = fork();
            if (fork_flag == -1)
            {
                perror("fork");

                return 1;
            }
            else if (fork_flag == 0)
            {
                int execvp_flag = execvp(argv[0], argv);
                if (execvp_flag == -1)
                {
                    perror("execvp");
                    return 1;
                }
            }

            int wait_flag = wait(&error_flag);
            if (wait_flag == -1)
            {
                perror("wait");
                return 1;
            }
//            cout << "Error flag is " << error_flag << endl;
            if (error_flag != 0)
            {
                if (flag == 2)
                {
                    break;
                }
            }
            else if (flag == 1)
            {
                break;
            }
        }
    }

    return 0;
}
