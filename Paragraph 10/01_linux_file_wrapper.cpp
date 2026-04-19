#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <iostream>
#include <string>
#include <system_error>
#include <utility>
#include <vector>

template <class T>
inline T check(T ret)
{
    if (ret == T(-1))
        throw std::system_error(errno, std::generic_category());
    return ret;
}

class File
{
private:
    int fd = -1;

    void writeAll(const char *data, size_t size)
    {
        size_t total_written = 0;

        while (total_written < size)
        {
            ssize_t nwritten = check(write(fd, data + total_written, size - total_written));
            total_written += static_cast<size_t>(nwritten);
        }

        std::cout << "Bytes written: " << total_written << std::endl;
    }

public:
    File(const std::string &filename, int flags, mode_t mode = 0644)
    {
        fd = check(open(filename.c_str(), flags, mode));
    }

    ~File() noexcept
    {
        if (fd != -1)
            close(fd);
    }

    File(const File &rhs)
    {
        fd = check(dup(rhs.fd));
    }

    File &operator=(const File &rhs)
    {
        if (this != &rhs)
        {
            if (fd == -1)
                fd = check(dup(rhs.fd));
            else
                fd = check(dup2(rhs.fd, fd));
        }
        return *this;
    }

    File(File &&rhs) noexcept
    {
        fd = rhs.fd;
        rhs.fd = -1;
    }

    File &operator=(File &&rhs) noexcept
    {
        if (this != &rhs)
        {
            if (fd != -1)
                close(fd);

            fd = rhs.fd;
            rhs.fd = -1;
        }
        return *this;
    }

    void writeToFile(const std::string &data)
    {
        writeAll(data.c_str(), data.size());
    }

    std::string readFromFile(size_t size)
    {
        std::vector<char> buffer(size);
        ssize_t nread = check(read(fd, buffer.data(), buffer.size()));
        return std::string(buffer.data(), static_cast<size_t>(nread));
    }

    off_t getPosition()
    {
        return check(lseek(fd, 0, SEEK_CUR));
    }

    void setPosition(off_t offset)
    {
        check(lseek(fd, offset, SEEK_SET));
    }
};

int main()
{
    try
    {
        File file("testfile.txt", O_CREAT | O_WRONLY | O_TRUNC);
        file.writeToFile("Hello, World!\n");

        File file_read("testfile.txt", O_RDONLY);
        std::string content = file_read.readFromFile(256);
        std::cout << "File contents: " << content << std::endl;

        File moved_file = std::move(file_read);
        moved_file.setPosition(0);
        std::string moved_content = moved_file.readFromFile(256);
        std::cout << "Moved file contents: " << moved_content << std::endl;
    }
    catch (const std::system_error &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
