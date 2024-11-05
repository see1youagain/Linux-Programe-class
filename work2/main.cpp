#include <sys/stat.h>  // 用于获取文件状态信息的库
#include <unistd.h>    // 提供对 POSIX 操作系统 API 的访问，例如文件操作
#include <dirent.h>    // 提供操作目录的函数，如 opendir, readdir 等
#include <iostream>    // 提供输入输出流
#include <string>      // 提供对 std::string 类的支持

// 声明函数：打印指定文件的元信息
void printFileInfo(const std::string &filePath);

// 声明函数：列出指定目录中的所有文件和子目录，参数 recursive 控制是否递归
void listDirectory(const std::string &dirPath, bool recursive);

// 定义函数：列出目录中的所有文件和子目录
void listDirectory(const std::string &dirPath, bool recursive) {
    // 打开目录
    DIR *dir = opendir(dirPath.c_str());
    if (dir == nullptr) {  // 如果打开目录失败，打印错误并返回
        perror("opendir"); // 输出错误信息，如权限不足
        return;
    }

    struct dirent *entry;  // 用于存储目录中的条目信息
    // 使用循环读取目录中的所有条目
    while ((entry = readdir(dir)) != nullptr) {
        // 跳过当前目录（.）和父目录（..）的特殊条目
        if (std::string(entry->d_name) == "." || std::string(entry->d_name) == "..") {
            continue; // 跳过当前目录和父目录的特殊条目
        }

        // 构建完整的文件或目录路径（目录路径 + 文件名）
        std::string fullPath = dirPath + "/" + entry->d_name;

        // 打印文件或目录的元信息
        printFileInfo(fullPath);

        // 如果需要递归且当前条目是目录，则递归调用 listDirectory
        if (recursive && entry->d_type == DT_DIR) {
            listDirectory(fullPath, recursive);
        }
    }

    // 关闭目录指针以释放资源
    closedir(dir);
}

// 定义函数：打印指定文件或目录的详细信息
void printFileInfo(const std::string &filePath) {
    struct stat fileStat;

    // 获取文件元数据，使用 stat 系统调用
    if (stat(filePath.c_str(), &fileStat) < 0) {
        perror("stat"); // 如果获取文件状态失败，输出错误信息
        return;         // 退出函数
    }

    // 判断文件类型并打印文件类型符号
    if (S_ISREG(fileStat.st_mode)) {
        std::cout << "-";  // 普通文件
    } else if (S_ISDIR(fileStat.st_mode)) {
        std::cout << "d";  // 目录
    } else if (S_ISLNK(fileStat.st_mode)) {
        std::cout << "l";  // 符号链接
    } else {
        std::cout << "?";  // 未知类型
    }

    // 打印文件权限（9个权限位：用户、组、其他用户的读写执行权限）
    std::cout << ((fileStat.st_mode & S_IRUSR) ? "r" : "-"); // 用户读权限
    std::cout << ((fileStat.st_mode & S_IWUSR) ? "w" : "-"); // 用户写权限
    std::cout << ((fileStat.st_mode & S_IXUSR) ? "x" : "-"); // 用户执行权限
    std::cout << ((fileStat.st_mode & S_IRGRP) ? "r" : "-"); // 组读权限
    std::cout << ((fileStat.st_mode & S_IWGRP) ? "w" : "-"); // 组写权限
    std::cout << ((fileStat.st_mode & S_IXGRP) ? "x" : "-"); // 组执行权限
    std::cout << ((fileStat.st_mode & S_IROTH) ? "r" : "-"); // 其他用户读权限
    std::cout << ((fileStat.st_mode & S_IWOTH) ? "w" : "-"); // 其他用户写权限
    std::cout << ((fileStat.st_mode & S_IXOTH) ? "x" : "-"); // 其他用户执行权限

    // 输出文件大小（以字节为单位）
    std::cout << " " << fileStat.st_size;

    // 输出文件名
    std::cout << " " << filePath << std::endl;
}

// 主函数：程序入口
int main(int argc, char *argv[]) {
    bool recursive = false;  // 标志位：是否递归
    std::string targetPath = "."; // 目标路径，默认为当前目录

    // 检查输入参数
    if (argc > 1) {
        if (std::string(argv[1]) == "-R") {  // 如果参数是 -R，表示递归
            recursive = true;  // 设置递归标志位
            if (argc > 2) {
                targetPath = argv[2];  // 如果有第二个参数，设置目标路径
            }
        } else {
            targetPath = argv[1];  // 否则，目标路径是第一个参数
        }
    }

    // 获取目标路径的文件状态
    struct stat fileStat;
    if (stat(targetPath.c_str(), &fileStat) == 0) {
        if (S_ISDIR(fileStat.st_mode)) {
            // 如果目标路径是目录，调用 listDirectory
            listDirectory(targetPath, recursive);
        } else {
            // 如果是文件，则直接打印文件信息
            printFileInfo(targetPath);
        }
    } else {
        // 如果 stat 失败，输出错误信息
        perror("stat");
    }

    return 0;
}
