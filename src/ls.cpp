#include <iostream>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <errno.h>

using namespace std;

#define FLAG_a 1
#define FLAG_l 2
#define FLAG_R 4

int main(int argc, char * argv[])
{
    int FLAG = 0;

    for (int i = 0; argv[i] != '\0'; ++i)
    {
        if ((FLAG & 7) == 7)
        {
            break;
        }
        if (argv[i][0] == '-')
        {
            for (int j = 1; argv[i][j] != '\0'; ++j)
            {
                if (argv[i][j] == 'a')
                {
                    FLAG = FLAG | FLAG_a;
                }
                else if (argv[i][j] == 'l')
                {
                    FLAG = FLAG | FLAG_l;
                }
                else if (argv[i][j] == 'R')
                {
                    FLAG = FLAG | FLAG_R;
                }
            }
        }
    }

    char *dirName = new char('.');

    DIR *dirp  = opendir(dirName);
    if (dirp == 0)
    {
        perror("opendir");
        return 1;
    }
    dirent *direntp;

    errno = 0;

    while ((direntp = readdir(dirp)))
    {
        cout << direntp->d_name << endl;
    }

    if (errno != 0)
    {
        perror("readdir");
        return 1;
    }

    int CLOSEDIR_ERR = closedir(dirp);
    if (CLOSEDIR_ERR == -1)
    {
        perror("closedir");
        return 1;
    }

    delete[] dirName;

    return 0;
}

