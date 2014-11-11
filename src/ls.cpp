#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>
#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <cstring>

#define FLAG_a 1
#define FLAG_R 2
#define FLAG_l 4
#define PERMISSION(x, y) \
        if (sb.st_mode & x) \
        {cout << y;} \
        else {cout << '-';}
#define PER(x) PERMISSION(S_IR##x, 'r') PERMISSION(S_IW##x, 'w') \
        PERMISSION(S_IX##x, 'x')
#define ALL_PER() PER(USR) PER(GRP) PER(OTH)

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

void merge(vector<char*>& vec, int mid)
{
    vector<char*> temp(vec.size());

    int i = 0;
    unsigned j = mid;
    unsigned index = 0;

    while(i < mid && j < vec.size())
    {
        if (strcmp(vec.at(i), vec.at(j)) <= 0)
        {
            temp.at(index) = vec.at(i);
            ++i;
        }
        else
        {
            temp.at(index) = vec.at(j);
            ++j;
        }
        ++index;
    }
    while(i < mid)
    {
        temp.at(index) = vec.at(i);
        ++i;
        ++index;
    }
    while(j < vec.size())
    {
        temp.at(index) = vec.at(j);
        ++j;
        ++index;
    }

    vec = temp;

    return;
}

void merge_sort(vector<char*>& vec, int begin, int end)
{
    if (begin == end)
    {
        return;
    }
    int mid = (begin + end)/2;

    merge_sort(vec, begin, mid);
    merge_sort(vec, mid + 1, end);

    merge(vec, mid + 1);
}

int setFlag(int argc, char* argv[], vector<char*>& d,
        vector<char*>& f, int& a)
{
    int flag = 0;
    struct stat sb;
    char* temp;

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
            else
            {
                temp = new char[strlen(argv[i]) + 1];
                strcpy(temp, argv[i]);

                if (S_ISDIR(sb.st_mode))
                {
                    d.push_back(temp);
                }
                else
                {
                    f.push_back(temp);
                }
            }

            continue;
        }
        for (int j = 1; argv[i][j]; ++j)
        {
            if (argv[i][j] == 'R')
            {
                flag = flag | FLAG_R;
            }
            else if (argv[i][j] == 'l')
            {
                flag = flag | FLAG_l;
            }
            else if (argv[i][j] == 'a')
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

void outnorm(const vector<char*>& files, const char* dir, const int& flags)
{
    if (flags & FLAG_R)
    {
        if (dir[strlen(dir) - 1] == '/')
        {
            for (unsigned i = 0; i < strlen(dir) - 1; ++i)
            {
                cout << dir[i];
            }
        }
        else
        {
            cout << dir;
        }
        cout << ":" << endl << endl;
    }

    for (unsigned i = 0; i < files.size() ; ++i)
    {
        cout << files.at(i) << endl;
    }
    //int columns;

    return;
}

void outLong(const vector<char*>& files, const char* dir, const int& flags,
        const int& total)
{
    struct stat sb;
    char* path;

    cout << "total " << total/2 << endl; //i have no idea why this is twice
    //the value ls gives

    for(unsigned i = 0; i < files.size(); ++i)
    {
        path = new char[strlen(files.at(i)) + strlen(dir) + 2];
        strcpy(path, dir);
        if (dir[strlen(dir) - 1] != '/')
        {
            char temp[] = "/";
            strcat(path, temp);
        }
        strcat(path, files.at(i));
        int err = lstat(path, &sb);
        if (err != 0)
        {
            perror("stat");
            exit(1);
        }
        if (S_ISLNK(sb.st_mode))
        {
            cout << 'l';
        }
        else if (S_ISDIR(sb.st_mode))
        {
            cout << 'd';
        }
        else
        {
            cout << '-';
        }

        ALL_PER()

            cout << ' ';
        cout << sb.st_nlink << ' ';

        struct passwd *pw = getpwuid(sb.st_uid);
        if (pw == NULL)
        {
            perror("getpwuid");
            exit(1);
        }
        cout << pw->pw_name;

        cout << '\t';

        struct group *gr = getgrgid(sb.st_gid);
        if (gr == NULL)
        {
            perror("getgrgid");
            exit(1);
        }
        cout << gr->gr_name;

        cout << '\t';

        cout << sb.st_size;

        cout << '\t';

        struct tm* ptm = gmtime( &sb.st_mtime);
        string tim = asctime(ptm);
        cout << tim.substr(4, 12) << ' ';

        cout << files.at(i);
        cout << endl;
        delete [] path;
    }
    return;
}

void ls(int flags, vector<char*>& dir)
{
    for(unsigned i = 0; i < dir.size(); ++i)
    {
        vector<char*> dir_r;
        vector<char*> s;
        int total_block = 0;
        struct stat sb;
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
            char* directory;
            if (!(flags & FLAG_a) && *f == '.')
            {
                continue;
            }
            if (flags & FLAG_R && strncmp(f, "..", 3) == 0)
            {
                continue;
            }
            file = new char[strlen(f) + 1];
            strcpy(file, f);

            directory = new char[strlen(file) + strlen(dir.at(i)) + 2];
            strcpy(directory, dir.at(i));
            strcat(directory, "/");
            strcat(directory, file);


            if (lstat(directory, &sb) != 0)
            {
                perror("lstat");
                exit(1);
            }
            total_block += sb.st_blocks;
            if (flags & FLAG_R && S_ISDIR(sb.st_mode))
            {
                dir_r.push_back(directory);
            }
            else
            {
                delete [] directory;
            }

            //cout << "FIle: " << file << endl;
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
        //cout << "===================================" << endl;
        //for (int j = s.size() - 1; j >= 0; --j)
        //  {
        //  cout << s.at(j) << endl;
        //  }
        cout << "===================================" << endl;
        //merge_sort(s, 0, s.size());
        //merge_sort(dir_r, 0, dir_r.size());
        /*cout << "==========================" << endl;
          for (int j = s.size() - 1; j >= 0; --j)
          {
          cout << s.at(j) << endl;
          }*/

        if (flags & FLAG_l)
        {
            outLong(s, dir.at(i), flags, total_block);
        }
        else
        {
            outnorm(s, dir.at(i), flags);
        }

        for (unsigned i = 0; i < s.size(); ++i)
        {
            if (s.at(i) != 0)
            {
                delete [] s.at(i);
            }
        }

        if (flags & FLAG_R && dir_r.size() != 0)
        {
            ls(flags, dir_r);
        }

        /*for (unsigned i = 0; i < dir_r.size(); ++i)
        {
            cout << dir_r.at(i) << endl;
            if (dir_r.at(i) != 0)
            {
                delete [] dir_r.at(i);
            }
        }*/
    }
    for (unsigned i = 0; i < dir.size(); ++i)
    {
        if (dir.at(i) != 0)
        {
            delete [] dir.at(i);
        }
    }
}

int main(int argc, char* argv[])
{
    int a = 0; //initialize counter for number of flags
    vector<char*> dirs;
    vector<char*> files;
    int flags = setFlag(argc, argv, dirs, files, a);
    if (dirs.size() == 0 && files.size() == 0 && argc - a == 1)
    {
        char* d = new char[2];
        strcpy(d, ".");
        dirs.push_back(d);
    }
    ls(flags, dirs);
    return 0;
}

