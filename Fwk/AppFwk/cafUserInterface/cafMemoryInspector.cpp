#include "cafMemoryInspector.h"

#include <QFile>
#include <QRegExp>
#include <QString>
#include <QStringList>

#ifdef _WIN32
#include "windows.h"
#include "psapi.h"
#elif defined (__linux__)
#include <map>
#include <unistd.h>
#include "sys/types.h"
#include "sys/sysinfo.h"
#endif

#define MIB_DIV 1048576

#ifdef __linux__
//--------------------------------------------------------------------------------------------------
/// Read bytes of memory of different types for current process from /proc/self/statm
/// See: http://man7.org/linux/man-pages/man5/proc.5.html
/// The first three columns in statm are:
///  * VmSize: size of virtual memory
///  * RSS: resident memory size of process
///  * Shared: shared memory used by process
//--------------------------------------------------------------------------------------------------
std::map<QString, uint64_t> readProcessBytesLinux()
{
    std::map<QString, uint64_t> quantities;
    int pageSize = getpagesize();
    QFile procSelfStatus("/proc/self/statm");
    if (procSelfStatus.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString line(procSelfStatus.readLine(256));
        QStringList lineWords = line.split(QRegExp("\\s+"), QString::SkipEmptyParts);
        quantities["VmSize"] = static_cast<uint64_t>(lineWords[0].toLongLong() * pageSize);
        quantities["RSS"] = static_cast<uint64_t>(lineWords[1].toLongLong() * pageSize);
        quantities["Shared"] = static_cast<uint64_t>(lineWords[2].toLongLong() * pageSize);
    }
    return quantities;
}
#endif
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
uint64_t caf::MemoryInspector::getApplicationPhysicalMemoryUsageMiB()
{
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS_EX pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), (PPROCESS_MEMORY_COUNTERS) &pmc, sizeof(pmc));
    SIZE_T physicalMemUsedByMe = pmc.WorkingSetSize;
    return static_cast<uint64_t>(physicalMemUsedByMe / MIB_DIV);
#elif defined(__linux__)
    return readProcessBytesLinux()["RSS"] / MIB_DIV;
#else
    return 0u;
#endif
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
uint64_t caf::MemoryInspector::getApplicationVirtualMemoryUsageMiB()
{
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS_EX pmc;
    GetProcessMemoryInfo(GetCurrentProcess(), (PPROCESS_MEMORY_COUNTERS)&pmc, sizeof(pmc));
    SIZE_T virtualMemUsedByMe = pmc.PrivateUsage;
    return static_cast<uint64_t>(virtualMemUsedByMe / MIB_DIV);
#elif defined(__linux__)
    return readProcessBytesLinux()["VmSize"] / MIB_DIV;
#else
    return 0u;
#endif
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
uint64_t caf::MemoryInspector::getTotalVirtualMemoryMiB()
{
#ifdef _WIN32
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    DWORDLONG totalVirtualMem = memInfo.ullTotalPageFile;
    return static_cast<uint64_t> (totalVirtualMem / MIB_DIV);
#elif defined(__linux__)
    struct sysinfo memInfo;
    sysinfo(&memInfo);
    long long totalVirtualMem = memInfo.totalram;
    totalVirtualMem += memInfo.totalswap;
    totalVirtualMem *= memInfo.mem_unit;
    return totalVirtualMem / MIB_DIV;
#else
    return 0u;
#endif
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
uint64_t caf::MemoryInspector::getTotalPhysicalMemoryMiB()
{
#ifdef _WIN32
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    DWORDLONG totalPhysMem = memInfo.ullTotalPhys;
    return static_cast<uint64_t>(totalPhysMem / MIB_DIV);
#elif defined(__linux__)
    struct sysinfo memInfo;
    sysinfo(&memInfo);
    long long totalPhysicalMem = memInfo.totalram;
    totalPhysicalMem *= memInfo.mem_unit;
    return totalPhysicalMem / MIB_DIV;
#else
    return 0u;
#endif
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
uint64_t caf::MemoryInspector::getAvailableVirtualMemoryMiB()
{
#ifdef _WIN32
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    DWORDLONG availVirtualMem = memInfo.ullAvailPageFile;
    return static_cast<uint64_t> (availVirtualMem / MIB_DIV);
#elif defined(__linux__)
    struct sysinfo memInfo;
    sysinfo(&memInfo);
    long long availVirtualMem = memInfo.freeram;
    availVirtualMem += memInfo.freeswap;
    availVirtualMem *= memInfo.mem_unit;
    return availVirtualMem / MIB_DIV;
#else
    return 0u;
#endif
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
float caf::MemoryInspector::getRemainingMemoryCriticalThresholdFraction()
{
#ifdef __linux__
    return 0.175f;
#else
    return 0.05f;
#endif
}
