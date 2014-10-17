#include <iostream>


using namespace std;

void display_prompt()
{
    cout << "$";
}

int main()
{
    display_prompt();
    string command = getline();
    cout << command << endl;
    return 0,
}
