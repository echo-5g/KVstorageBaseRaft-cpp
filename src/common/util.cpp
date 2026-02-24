#include "util.h"
#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <ctime>
#include <iomanip>

// 工具类
// 1. 断言函数：功能：自定义断言，用于校验程序运行中的关键条件（比如 Raft 节点状态合法性、日志索引有效性）。
void myAssert(bool condition, std::string message) {
  if (!condition) {
    std::cerr << "Error: " << message << std::endl;
    std::exit(EXIT_FAILURE);
  }
}

// 2. 时间戳函数：功能：获取高精度系统当前时间戳，返回 time_point 类型（可用于计算时间差）。
std::chrono::_V2::system_clock::time_point now() { return std::chrono::high_resolution_clock::now(); }

// 3. 随即选举超时函数：功能：生成 Raft 核心的「随机选举超时时间」（默认 150-300ms 区间，由 min/maxRandomizedElectionTime 定义）。
std::chrono::milliseconds getRandomizedElectionTimeout() {
  std::random_device rd;
  std::mt19937 rng(rd());
  std::uniform_int_distribution<int> dist(minRandomizedElectionTime, maxRandomizedElectionTime);

  return std::chrono::milliseconds(dist(rng));
}

// 4. 休眠函数：功能：让当前线程休眠指定毫秒数。
void sleepNMilliseconds(int N) { std::this_thread::sleep_for(std::chrono::milliseconds(N)); };

// 5. 可用端口获取函数：功能：从指定端口开始，自动查找 30 个范围内的第一个可用端口（避免端口冲突）。
bool getReleasePort(short &port) {
  short num = 0;
  while (!isReleasePort(port) && num < 30) {
    ++port;
    ++num;
  }
  if (num >= 30) {
    port = -1;
    return false;
  }
  return true;
}

// 6. 端口检测函数：功能：检测指定端口是否被占用（核心逻辑是尝试绑定端口，绑定成功则未被占用）。
bool isReleasePort(unsigned short usPort) {
  int s = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(usPort);
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  int ret = ::bind(s, (sockaddr *)&addr, sizeof(addr));
  if (ret != 0) {
    close(s);
    return false;
  }
  close(s);
  return true;
}

// 7. 调试日志函数：功能：带时间戳的调试日志打印（仅在 Debug 模式下生效）。
void DPrintf(const char *format, ...) {
  if (Debug) {
    // 获取当前的日期，然后取日志信息，写入相应的日志文件当中 a+
    time_t now = time(nullptr);
    tm *nowtm = localtime(&now);
    va_list args;
    va_start(args, format);
    std::printf("[%d-%d-%d-%d-%d-%d] ", nowtm->tm_year + 1900, nowtm->tm_mon + 1, nowtm->tm_mday, nowtm->tm_hour,
                nowtm->tm_min, nowtm->tm_sec);
    std::vprintf(format, args);
    std::printf("\n");
    va_end(args);
  }
}
