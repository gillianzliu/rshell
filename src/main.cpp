#include <iostream>
#include <stdlib.h>
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

                //if it is not 2, give an error message and clear
                //the vector so that no commands are executed
                if (n != 2)
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

        //if there was a comment then delete everything
        //after that AKA set a substring from 0 to the '#'
        if (p != -1)
        {
            command = command.substr(0, p);
        }

        //so continue doing and getting while there is
        //still an && or || or until the end of the
        //commands
        while(flag != 0 || command != "")
        {
            //get the command in argv
            char ** argv = get_command(command, flag);

//            cout << command << endl;
//            cout << flag << endl;

            //if it is empty, then go back to prompt
            if (argv[0] == NULL)
            {
                break;
            }

            //if the command is exit, exit program
            else if (strcmp(argv[0], "exit") == 0)
            {
                return 0;
            }

//            for (int i = 0; argv[i] != 0; ++i)
//            {
//                cout << argv[i] << endl;
//            }

            error_flag = 0;

            //call the fork and check for error
            int fork_flag = fork();
            if (fork_flag == -1)
            {
                perror("fork");

                exit(1);
            }
            //if it is the child do the command while checking
            //for error
            else if (fork_flag == 0)
            {
                int execvp_flag = execvp(argv[0], argv);
                if (execvp_flag == -1)
                {
                    perror("execvp");
                    exit(1);
                }
            }

            //if parent then wait for child
            int wait_flag = wait(&error_flag);
            if (wait_flag == -1)
            {
                perror("wait");
                exit(1);
            }
//            cout << "Error flag is " << error_flag << endl;
            //if there was an error
            if (error_flag != 0)
            {
                //if it was the first in an &&, go back
                //to prompt
                if (flag == 2)
                {
                    break;
                }
            }
            //if there wasn't an error and it was in an  ||
            //statement then go back to prompt
            else if (flag == 1)
            {
                break;
            }
        }
    }

    return 0;
}
