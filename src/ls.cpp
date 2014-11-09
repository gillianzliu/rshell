#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>

#define FLAG_a 1
#define FLAG_r 2
#define FLAG_l 4

using namespace std;

//for directories, it is ordered by alphabet and
//by path, not foldername
bool ledir(char* left, char* right)
{
    //FIXME
    return true;
}

bool lefile(char* left, char* right)
{
    //FIXME
    return true;
}

int setFlag(int argc, char* argv[], vector<char*>& d,
            vector<char*>& f, int& a)
{
    int flag = 0;
    struct stat sb;

    for (int i = 1; i < argc; ++i)
    {
        if (argv[i][0] != '-')
        {
            int err = stat(argv[i], &sb);
            if (err == -1)
            {
                cerr << "ls: cannot access " << argv[i]
                     << ": No such file or directory" << endl;
            }

            else if (S_ISDIR(sb.st_mode))
            {
                d.push_back(argv[i]);
            }
            else
            {
                f.push_back(argv[i]);
            }

            continue;
        }
        for (int j = 1; argv[i][j]; ++j)
        {
            if (argv[i][j] == 'R' || argv[i][j] == 'r')
            {
                flag = flag | FLAG_r;
            }
            else if (argv[i][j] == 'L' || argv[i][j] == 'l')
            {
                flag = flag | FLAG_l;
            }
            else if (argv[i][j] == 'A' || argv[i][j] == 'a')
            {
                flag = flag | FLAG_a;
            }
            else
            {
                cerr << "ls: invalid option -- '" << argv[i][j] << "'"
                     << endl;
                exit(1);
            }
        }
        a++;
    }

    return flag;
}

void outLong(const vector<char*>& files)
{
    struct stat sb;

    for(unsigned i = 0; i < files.size(); ++i)
    {
        int err = lstat(files.at(i), &sb);
        if (err != 0)
        {
            perror("stat");
            exit(1);
        }
        if (S_ISDIR(sb.st_mode))
        {
            //cout << 'd';
        }
        else
        {
            //cout << '-';
        }
    }
    return;
}

void ls(int flags, vector<char*>& dir)
{
    vector<char*> dir_r;
    vector<char*> s;

    for(unsigned i = 0; i < dir.size(); ++i)
    {
        DIR *dirp = opendir(dir.at(i));
        if (dirp == 0)
        {
            perror("opendir");
            exit(1);
        }

        dirent *direntp;
        while((direntp = readdir(dirp)))
        {
            char* f = direntp->d_name;
            char* file;
            if (!(flags & FLAG_a) && f[0] == '.')
            {
                continue;
            }
            if (strncmp(dir.at(i), ".", 2) != 0 &&
                    strncmp(dir.at(i), "./", 3) != 0)
            {
                if ((dir.at(i))[strlen(dir.at(i)) - 1] != '/')
                {
                    char temp[] = "/";
                    file = new char[strlen(dir.at(i)) + strlen(f) + 2];
                    strcpy(file, dir.at(i));
                    strcat(file, temp);
                    strcat(file, f);
                }
                else
                {
                    file = new char[strlen(dir.at(i)) + strlen(f) + 2];
                    strcpy(file, dir.at(i));
                    strcat(file, f);
                }


            }

            s.push_back(file);
        }
        if (errno != 0)
        {
            perror("readdir");
            exit(1);
        }
        if (closedir(dirp) != 0)
        {
            perror("closedir");
            exit(1);
        }
        outLong(s);

        for (int i = s.size() - 1; i >= 0; --i)
        {
            delete [] s.at(i);
        }

        if (flags & FLAG_r)
        {
            ls(flags, dir_r);
        }

    }

}

int main(int argc, char* argv[])
{
    int a = 0;
    vector<char*> dirs;
    vector<char*> files;
    int flags = setFlag(argc, argv, dirs, files, a);
    if (dirs.size() == 0 && files.size() == 0 && argc - a == 1)
    {
        char d[] = ".";
        dirs.push_back(d);
    }
    ls(flags, dirs);
    return 0;
}

