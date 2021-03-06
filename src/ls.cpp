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
#include <cctype>
#include <iomanip>

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

bool lfile(const char* left, const char* right)
{
    int i = 0;
    int j = 0;

    while(left[i] != '\0' && !isalnum(left[i]))
    {
        ++i;
    }
    while(right[j] != '\0' && !isalnum(right[j]))
    {
        ++j;
    }
    if (left[i] == '\0' && right[j] == '\0')
    {
        return i < j;
    }
    if (left[i] == '\0')
    {
        return true;
    }
    if (right[j] == '\0')
    {
        return false;
    }
    while(left[i] != '\0' && right[j] != '\0')
    {
        if (!isalnum(left[i]) || !isalnum(right[j]))
        {
            if(!isalnum(left[i]))
            {
                ++i;
            }
            if (!isalnum(right[j]))
            {
                ++j;
            }

            continue;
        }

        if (tolower(left[i]) == tolower(right[j]))
        {
            ++i;
            ++j;
            continue;
        }
        else if (tolower(left[i]) < tolower(right[j]))
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    if (left[i] == 0 && right[j] == 0)
    {
        if (strlen(left) != strlen(right))
        {
            return (strlen(left) < strlen(right));
        }
        while(left[i] != '\0' && !isalnum(left[i]))
        {
            ++i;
        }
        while(right[j] != '\0' && !isalnum(right[j]))
        {
            ++j;
        }
        while(right[i] != '\0' && left[j] != '\0')
        {
            if (!isalnum(left[i]) || !isalnum(right[j]))
            {
                if(!isalnum(left[i]))
                {
                    ++i;
                }
                if (!isalnum(right[j]))
                {
                    ++j;
                }
                continue;
            }
            if (left[i] == right[j])
            {
                ++i;
                ++j;
            }
            else if (left[i] < right[j])
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }
    else if (left[i] == '\0')
    {
        return true;
    }

    return false;
}

void merge(vector<char*>& vec, int begin, int mid, int end)
{
    vector<char*> temp(end - begin + 1);

    int i = begin;
    int j = mid + 1;
    unsigned index = 0;

    while(i <= mid && j <= end)
    {
        if (lfile(vec.at(i), vec.at(j)))
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
    while(i <= mid)
    {
        temp.at(index) = vec.at(i);
        ++i;
        ++index;
    }
    while(j <= end)
    {
        temp.at(index) = vec.at(j);
        ++j;
        ++index;
    }

    for (i = begin, index = 0; index < temp.size() ; ++i, ++index)
    {
        vec.at(i) = temp.at(index);
    }
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

    merge(vec, begin, mid, end);
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

void get_color(string& color, const char* file, const struct stat& sb)
{
    if (file[0] == '.')
    {
        color += "100";
    }
    else
    {
        color += "49";
    }
    if (S_ISLNK(sb.st_mode))
    {
        color += ";1;96";
    }
    else if (S_ISDIR(sb.st_mode))
    {
        color += ";1;94";
    }
    else if (S_ISREG(sb.st_mode) && sb.st_mode & 0111)
    {
        color += ";92";
    }
    else
    {
        color += ";39";
    }

    color += "m";
}

vector<int> get_columns(const vector<char*>& files)
{
    vector<int> total_col;
    unsigned max_size = 0;
    unsigned total = 0;
    for (unsigned i = 0; i < files.size(); ++i)
    {
        if (strlen(files.at(i)) > max_size)
        {
            max_size = strlen(files.at(i));
        }
        total += strlen(files.at(i));
    }
    if (max_size >= 80)
    {
        total_col.push_back(80);
        return total_col;
    }
    int row_num = total / 80;
    if ((total + (2 * (files.size() - 1)))/80 == 0)
    {
        for (unsigned i = 0; i < files.size(); ++i)
        {
            total_col.push_back(strlen(files.at(i)));
        }
        return total_col;
    }
    int col_num = 2;
    if (row_num != 0)
    {
        col_num = total / (row_num + 1);
    }

    bool done = false;

    while (!done)
    {
        vector<int> col(col_num);
        int total_row = 0;
        unsigned max = 0;
        int count;
        int c = 0;
        for (unsigned i = 0; i < files.size(); i += count)
        {
            count = 0;
            for (unsigned row = c; row < files.size(); row += col_num)
            {
                count++;
            }
            //cout << count << endl;
            max = 0;
            for(unsigned j = i; j < i + count; ++j)
            {
                //cout << "Max: " << max << endl;
                //cout << files.at(j) << ": " << strlen(files.at(j)) << endl;
                if(strlen(files.at(j)) > max)
                {
                    max = strlen(files.at(j));
                }
            }
            //cout << "+++++" << endl;
            //cout << "Max for column " << i << ": " << max << endl;
            //cout << "+++++++++++++" << endl;
            col.at(c) = max;
            ++c;
            total_row += max;
        }
        if (total_row + (2 * (col_num - 1)) > 80)
        {
            total_row = 0;
            if (total_col.size() == 0)
            {
                total_col = col;
            }
            for (unsigned p = 0; p < total_col.size(); ++p)
            {
                total_row += total_col.at(p);
            }
            if(total_col.size() == 0 ||
                ((total_row + (2 * (total_col.size() - 1))) > 80))
            {
                --col_num;
            }
            else
            {
                done = true;
                return total_col;
            }
        }
        else
        {
            col_num++;
            total_col = col;
        }
    }
    return total_col;
}

void outnorm(const vector<char*>& files, const char* dir, const int& flags)
{
    if (dir == 0)
    {
        ;
    }
    else if (flags & FLAG_R)
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
        cout << ":" << endl;
    }

    char* path;
    struct stat sb;
    string color_default = "\033[0m";
    vector<int> col = get_columns(files);
    vector<int> rows;
    for (unsigned c = 0; c < col.size(); ++c)
    {
        int count = 0;
        for (unsigned row = c; row < files.size(); row += col.size())
        {
            count++;
        }
        rows.push_back(count);
    }
    if(rows.size() == 0)
    {
        rows.push_back(1);
    }

    /*    for(unsigned i = 0; i < col.size(); ++i)
          {
          cout << "Column size: " << col.at(i) << endl;
          }
          cout << endl;*/
    for (int i = 0; i < rows.at(0); ++i)
    {
        for (unsigned c = 0; c < col.size() && i + 1 <= rows.at(c); ++c)
        {
            string color = "\033[";
            unsigned int index = i;
            for (unsigned s = 0; s < c ; ++s)
            {
                index += rows.at(s);
            }

            if(index >= files.size())
            {
                break;
            }
            if (dir != 0)
            {
                path = new char[strlen(files.at(index)) + strlen(dir) + 2];
                strcpy(path, dir);
                if (dir[strlen(dir) - 1] != '/')
                {
                    char temp[] = "/";
                    strcat(path, temp);
                }
                strcat(path, files.at(index));
            }
            else
            {
                path = new char[strlen(files.at(index)) + 1];
                strcpy(path, files.at(index));
            }
            int err = lstat(path, &sb);
            if (err != 0)
            {
                perror("stat");
                exit(1);
            }



            get_color(color, files.at(index), sb);
            cout << color << setfill(' ') << setw(col.at(c))
                << files.at(index);
            cout << color_default;

            if (c != col.size() - 1)
            {
                cout << "  ";
            }

            /*if (counter == 0)
              {
              cout << color << files.at(i);
              counter += strlen(files.at(i));
              }
              else if (strlen(files.at(i)) + counter + 2 <= 80)
              {
              cout << "  "  << color << files.at(i);
              counter += strlen(files.at(i)) + 2;
              }
              else
              {
              counter = strlen(files.at(i));
              cout << endl << color << files.at(i);
              }*/

            delete [] path;
        }
        cout << endl;
    }
    return;
}

void outLong(const vector<char*>& files, const char* dir, const int& flags,
        const int& total)
{
    struct stat sb;
    char* path;

    if (dir != 0)
    {
        if (flags & FLAG_R)
        {
            cout << dir << ':' << endl;
        }
        cout << "total " << total/2 << endl; //i have no idea why this is twice
        //the value ls gives
    }
    string color_default = "\033[0m";
    for(unsigned i = 0; i < files.size(); ++i)
    {
        string color = "\033[";

        if (dir != 0)
        {
            path = new char[strlen(files.at(i)) + strlen(dir) + 2];
            strcpy(path, dir);
            if (dir[strlen(dir) - 1] != '/')
            {
                char temp[] = "/";
                strcat(path, temp);
            }
            strcat(path, files.at(i));
        }
        else
        {
            path = new char[strlen(files.at(i)) + 1];
            strcpy(path, files.at(i));
        }
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

        struct tm* ptm = localtime( &sb.st_mtime);
        string tim = asctime(ptm);
        cout << tim.substr(4, 12) << ' ';

        get_color(color, files.at(i),sb);

        cout << color << files.at(i);
        cout << color_default << endl;
        delete [] path;
    }
    return;
}

void ls(int flags, vector<char*>& dir)
{
    for(unsigned i = 0; i < dir.size(); ++i)
    {
        errno = 0;
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
            if (!(flags & FLAG_a) && f[0] == '.')
            {
                continue;
            }

            file = new char[strlen(f) + 1];
            strcpy(file, f);
            if (flags & FLAG_R && (strncmp(f, "..", 3) == 0 ||
                        strncmp(f, ".", 2) == 0))
            {
                s.push_back(file);
                continue;
            }
            if ((dir.at(i))[strlen(dir.at(i)) - 1] == '/')
            {
                directory = new char[strlen(file) + strlen(dir.at(i)) + 1];
                strcpy(directory, dir.at(i));
            }
            else
            {
                directory = new char[strlen(file) + strlen(dir.at(i)) + 2];
                strcpy(directory, dir.at(i));
                strcat(directory, "/");
            }
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
        if (s.size() != 0)
        {
            merge_sort(s, 0, s.size() - 1);
        }

        if (flags & FLAG_l)
        {
            outLong(s, dir.at(i), flags, total_block);
        }
        else
        {
            outnorm(s, dir.at(i), flags);
        }
        if (i != dir.size() - 1 || dir_r.size() != 0)
        {
            cout << endl;
        }

        for (unsigned j = 0; j < s.size(); ++j)
        {
            if (s.at(j) != 0)
            {
                delete [] s.at(j);
            }
        }

        if (flags & FLAG_R && dir_r.size() != 0)
        {
            merge_sort(dir_r, 0, dir_r.size() - 1);

            ls(flags, dir_r);
            if (i != dir.size() - 1)
            {
                cout << endl;
            }
        }
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
    if (files.size() != 0)
    {
        if (flags & FLAG_l)
        {
            outLong(files, 0, flags, 0);
        }
        else
        {
            outnorm(files, 0, flags);
        }
    }
    for (unsigned i = 0; i < files.size(); ++i)
    {
        if (files.at(i) != 0)
        {
            delete [] files.at(i);
        }
    }
    if (dirs.size() == 0 && files.size() == 0 && argc - a == 1)
    {
        char* d = new char[2];
        strcpy(d, ".");
        dirs.push_back(d);
    }
    ls(flags, dirs);
    return 0;
}

