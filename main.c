#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <errno.h>

void format_size(char *str, long size)
{
    /*
     * Adapted from an implementation in Python.
    */
    float s = (float)size;
    char suffixes[5][3] = {"B", "KB", "MB", "GB", "TB"};
    int order = 0;
    while (s >= 1000 && order < 4)
    {
        ++order;
        s = s / 1000;
    }
    char fstring[] = "%.0f%s";
    sprintf(str, fstring, s, suffixes[order]);
}

void print_perms(int m)
{
    printf((m & S_IRUSR) ? "r" : "-");
    printf((m & S_IWUSR) ? "w" : "-");
    printf((m & S_IXUSR) ? "x" : "-");
    printf((m & S_IRGRP) ? "r" : "-");
    printf((m & S_IWGRP) ? "w" : "-");
    printf((m & S_IXGRP) ? "x" : "-");
    printf((m & S_IROTH) ? "r" : "-");
    printf((m & S_IWOTH) ? "w" : "-");
    printf((m & S_IXOTH) ? "x" : "-");
}

int main(int argc, char *argv[])
{
    DIR *d;
    char *dir_name;
    if (argc > 1)
    {
        d = opendir(argv[1]);
        dir_name = argv[1];
    }
    else
    {
        char buffer[128];
        printf("Directory to parse: ");
        fgets(buffer, sizeof(buffer), stdin);
        int len = strlen(buffer) - 1;
        if (buffer[len] == '\n')
        {
            buffer[len] = '\0';
        }
        d = opendir(buffer);
        dir_name = buffer;
    }

    if (d == NULL)
    {
        printf("error: cannot access '%s': %s\n", dir_name, strerror(errno));
        return 1;
    }
    printf("Statistics for directory: %s\n", dir_name);
    printf("%-20s\tType\tSize\tPermissions\n", "Name");
    struct dirent *entry;
    entry = readdir(d);
    long total_size = 0;
    while (entry != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            printf("%-20s\t%s\n", entry->d_name, "Dir");
        }
        else
        {
            struct stat file_stat;
            char path[PATH_MAX + 1];
            realpath(entry->d_name, path);
            stat(path, &file_stat);

            char type[5];
            if (S_ISREG(file_stat.st_mode))
            {
                strcpy(type, "File");
                total_size += file_stat.st_size;
            }
            else
            {
                strcpy(type, "Dir");
            }
            char formatted[10];
            format_size(formatted, file_stat.st_size);
            if (strlen(entry->d_name) >= 23)
            {
                printf("%-.20s... ", entry->d_name);
            }
            else
            {
                printf("%-20s\t", entry->d_name);
            }
            printf("%s\t%s\t", type, formatted);
            print_perms(file_stat.st_mode);
            printf("\n");
        }
        entry = readdir(d);
    }
    char formatted_size[10];
    format_size(formatted_size, total_size);
    printf("Total Size (excluding dirs):\t%s\n", formatted_size);
    closedir(d);
    return 0;
}