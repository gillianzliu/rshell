#include <iostream>
#include <string>
#include <cstring>

using namespace std;

char ** get_command(string a)
{
    char * copy = new char [a.length() + 1];
    strcpy(copy, a.c_str());

    //bool done = false;
    char * begin = copy;
    char * token = strtok(copy, " ;|&");

    while (token != 0)
    {
        cout << token << endl;

        if (token - begin + strlen(token) < a.size())
        {
            cout << a.at(token - begin + strlen(token)) << endl;
        }

        token = strtok(NULL, " ;|&");
    }

    return 0;
}

void display_prompt()
{
    cout << "$ ";
}

int main()
{
    display_prompt();
    string command;
    getline(cin, command);
    cout << command << endl;

    get_command(command);

    return 0;
}
