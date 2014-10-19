#include <iostream>
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
    char * token = strtok(copy, " ;|&");

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

        //to find out which delimiter was found
        //if it was the end, it will not go through this
        if (pos < a.size())
        {
            //store delimiter character found
            char delim = a.at(pos);
//            cout << delim << endl;

            if (delim != ' ' || (pos + 1 < a.size() && (a.at(pos + 1) == '&' ||
                a.at(pos + 1) == '|')))
            {
                //if it was not ' ' then we have reached a
                //delimiter that indicates the end of the
                //command so we are done. Convert
                //the vector into the proper char * []
                vec = conv_vec(temp);

//                for (unsigned i = 0; vec[i] != 0; ++i)
//                {
//                    cout << i << endl;
//                    cout << vec[i] <<  endl;
//                }

                //remember to get rid of the dynamically allocated
                delete copy;

                if (delim == '|' || (pos + 1 < a.size() && a.at(pos + 1) == '|'))
                {
                    //set flag bit
                    flag = 1;
                }
                else if (delim == '&' || (pos + 1 < a.size() && a.at(pos + 1)
                    == '&'))
                {
                    //set flag bit to 2
                    flag = 2;
                }
                else
                {
                    //then it was ; so set flag bit to 0
                    flag = 0;
                }

                cout << flag << endl;
                a = a.substr(pos + 1, a.size() - pos + 1);

                return vec;
            }

        }

        token = strtok(NULL, " ;|&");
    }

    a = "";

    vec = conv_vec(temp);

    delete copy;

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
    char *login = getlogin();
    if (login == 0)
    {
        perror("getlogin");
    }

    cout << login << '@' << host << "$ ";
}

int main()
{
    int flag = 0;
    while (1)
    {

        display_prompt();
        string command;
        getline(cin, command);
        while(flag != 0 || command != "")
        {
            char ** argv = get_command(command, flag);

            cout << command << endl;
            cout << flag << endl;

            if (strcmp(argv[0], "exit") == 0)
            {
                return 0;
            }

            for (int i = 0; argv[i] != 0; ++i)
            {
                cout << argv[i] << endl;
            }

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
//                    return 1;
                }
            }

            int wait_flag = wait(NULL);
            if (wait_flag == -1)
            {
                perror("wait");
                if (flag == 2)
                {
                    break;
                }
//                return 1;
            }
            if (flag == 1)
            {
                break;
            }
        }
    }

    return 0;
}
