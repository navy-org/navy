#pragma once

#include <stddef.h>
#include <stdint.h>

#define MAX_ERRNO       4095
#define IS_ERR_VALUE(x) ((uintptr_t)(x) >= (uintptr_t)-MAX_ERRNO)

[[gnu::warn_unused_result]] static inline void *ERR_PTR(intptr_t error)
{
    return (void *)error;
}

[[gnu::warn_unused_result]] static inline long PTR_ERR(const void *ptr)
{
    return (long)ptr;
}

[[gnu::warn_unused_result]] static inline bool IS_ERR(const void *ptr)
{
    return IS_ERR_VALUE((uintptr_t)ptr);
}

[[gnu::warn_unused_result]] static inline bool IS_ERR_OR_NULL(const void *ptr)
{
    return ptr == NULL || IS_ERR_VALUE((unsigned long)ptr);
}

[[gnu::warn_unused_result]] static inline void *ERR_CAST(const void *ptr)
{
    return (void *)ptr;
}

#define E2BIG           1      // Argument list too long
#define EACCES          2      // Permission denied
#define EADDRINUSE      3      // Address already in use
#define EADDRNOTAVAIL   4      // Cannot assign requested address
#define EAFNOSUPPORT    5      // Address family not supported by protocol
#define EAGAIN          6      // Try again
#define EALREADY        7      // Operation already in progress
#define EBADF           8      // Bad file number
#define EBADMSG         9      // Not a data message
#define EBUSY           10     // Device or resource busy
#define ECANCELED       11     // Operation canceled
#define ECHILD          12     // No child processes
#define ECONNABORTED    13     // Software caused connection abort
#define ECONNREFUSED    14     // Connection refused
#define ECONNRESET      15     // Connection reset by peer
#define EDEADLK         16     // Resource deadlock would occur
#define EDESTADDRREQ    17     // Destination address required
#define EDOM            18     // Math argument out of domain of func
#define EDQUOT          19     // Reserved
#define EEXIST          20     // File exists
#define EFAULT          21     // Bad address
#define EFBIG           22     // File too large
#define EHOSTUNREACH    23     // No route to host
#define EIDRM           24     // Identifier removed
#define EILSEQ          25     // Illegal byte sequence
#define EINPROGRESS     26     // Operation now in progress
#define EINTR           27     // Interrupted function
#define EINVAL          28     // Invalid argument
#define EIO             29     // I/O error
#define EISCONN         30     // Socket is already connected
#define EISDIR          31     // Is a directory
#define ELOOP           32     // Too many symbolic links encountered
#define EMFILE          33     // Too many open file
#define EMLINK          34     // Too many links
#define EMSGSIZE        35     // Message too long
#define EMULTIHOP       36     // Reserved
#define ENAMETOOLONG    37     // File name too long
#define ENETDOWN        38     // Network is down
#define ENETRESET       39     // Network dropped connection because of reset
#define ENETUNREACH     40     // Network is unreachable
#define ENFILE          41     // File table overflow
#define ENOBUFS         42     // No buffer space available
#define ENODATA         43     // No data available
#define ENODEV          44     // No such device
#define ENOENT          45     // No such file or directory
#define ENOEXEC         46     // Exec format error
#define ENOLCK          47     // No record locks available
#define ENOLINK         48     // Reserved
#define ENOMEM          49     // Not enough space
#define ENOMSG          50     // No message of desired type
#define ENOPROTOOPT     51     // Protocol not available
#define ENOSPC          52     // No space left on device
#define ENOSR           53     // No STREAM resources
#define ENOSTR          54     // Not a STREAM
#define ENOSYS          55     // Function not implemented
#define ENOTCONN        56     // The socket is not connected
#define ENOTDIR         57     // Not a directory
#define ENOTEMPTY       58     // Directory not empty
#define ENOTRECOVERABLE 59     // State not recoverable
#define ENOTSOCK        60     // Not a socket
#define ENOTSUP         61     // Not supported
#define ENOTTY          62     // Inappropriate I/O control operation
#define ENXIO           63     // No such device or address
#define EOPNOTSUPP      64     // Operation not supported on transport endpoint
#define EOVERFLOW       65     // Value too large to be stored in data type
#define EOWNERDEAD      66     // Previous owner died
#define EPERM           67     // Operation not permitted
#define EPIPE           68     // Broken pipe
#define EPROTO          69     // Protocol error
#define EPROTONOSUPPORT 70     // Protocol not supported
#define EPROTOTYPE      71     // Protocol wrong type for socket
#define ERANGE          72     // Result too large
#define EROFS           73     // Read-only file system
#define ESPIPE          74     // Invalid seek
#define ESRCH           75     // No such process
#define EDTALE          76     // Reserved
#define ETIME           77     // Stream ioctl timeout
#define ETIMEDOUT       78     // Connection timed out
#define ETXTBSY         79     // Text file busy
#define EWOULDBLOCK     EAGAIN // Operation would block
#define EXDEV           80     // Cross-device link
