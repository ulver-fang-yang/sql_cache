
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string>
#include <vector>
#include <fstream>


using namespace std;


#define SQL_CACHE_PATH "./sql_cache"
#define SQL_CACHE_FILE "./sql_cache/dump.osz"


const uint32_t max_mmap_size = 200 * 1000 * 1000;


string get_sql_buffer(string filename)
{
    ifstream fin(filename.c_str());
    char *buffer;
    fin.seekg(0, std::ios_base::end);
    uint64_t file_size = fin.tellg();
    buffer = (char *)malloc((size_t)(file_size + 1));
    fin.seekg(0, std::ios_base::beg);
    fin.read(buffer, file_size);
    fin.close();

    buffer[file_size] = 0;
    string str = buffer;
    free(buffer);

    return str;
}

int main()
{
    string str = get_sql_buffer("./sql.log");

    if (access(SQL_CACHE_PATH, R_OK) == -1)
        mkdir(SQL_CACHE_PATH, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    int fd = open(SQL_CACHE_FILE, O_CREAT | O_RDWR | O_TRUNC, 00777);
    if (fd == -1)
    {
        fprintf(stderr, "fail to open mmap file\n");
        exit(1);
    }
    lseek(fd, max_mmap_size - 1, SEEK_SET);
    write(fd, "", 1);

    struct stat sb;
    if (fstat(fd, &sb) == -1)
    {
        fprintf(stderr, "stat mmap file fail\n");
        exit(1);
    }
    uint8_t *mmap_buf = (uint8_t *)mmap(NULL, max_mmap_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    if (mmap_buf == MAP_FAILED)
    {
        fprintf(stderr, "fail to mmap\n");
        exit(1);
    }

    uint32_t mmap_buf_len = 0;
    string::size_type pos = str.find("\n");
    while (pos != string::npos)
    {
        string sql_str = str.substr(0, pos);
        str = str.substr(pos + 1, str.length() - pos - 1);
        pos = str.find("\n");
        //  先读取写入SQL到共享内存的程序的写入偏移
        uint32_t str_len = sql_str.length() + 1;
        memcpy(mmap_buf + 2 * sizeof(mmap_buf_len) + mmap_buf_len, &str_len, sizeof(str_len));
        memcpy(mmap_buf + 2 * sizeof(mmap_buf_len) + mmap_buf_len + sizeof(str_len),  sql_str.c_str(), str_len);
        if ((mmap_buf_len < 5000) || (str_len < 100))
            printf("SQL len: %d, %s\n", str_len, sql_str.c_str());
        mmap_buf_len += str_len + sizeof(str_len);
        memcpy(mmap_buf, &mmap_buf_len, sizeof(mmap_buf_len));
        printf("mmap_buf_len: %d\n", mmap_buf_len);
    }
    printf("str: %s\n", str.c_str());
    munmap(mmap_buf, max_mmap_size);

    return 0;
}

