/*
 * Copyright (c) 2003-2008 IIMURA Takuji. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id$
 */
#ifndef PASTRY_LIKE_CONFIG_H
#define PASTRY_LIKE_CONFIG_H

#ifdef __NetBSD__
#undef HAVE_SHA_H
#undef HAVE_OPENSSL_SHA_H
#define HAVE_SHA1_H
#define HAVE_SHA1INIT
#undef HAVE_SHA_CTX
#define HAVE_STDINT_H
#define HAVE_SYS_TIME_H
#define HAVE_UNISTD_H
#define HAVE_INTTYPES_H
#define HAVE_SYS_TIME_H
#define HAVE_NETDB_H
#define HAVE_SYS_SOCKET_H
#define HAVE_NETINET_IN_H
#define HAVE_ARPA_INET_H
#define HAVE_SYS_RESOURCE_H
#define HAVE_FD_SET
#undef HAVE_WINSOCK2_H
#define HAVE_BZERO
#define HAVE_GETTIMEOFDAY
#undef HAVE_TIMEGETTIME
#define HAVE_SNPRINTF
#define HAVE_NETINET_TCP_H
#define HAVE_DEV_RANDOM
#define HAVE_RANDOM
#define HAVE_TIME
#define HAVE_RUSAGE
#undef HAVE_WS2TCPIP_H
#undef HAVE_CLOSESOCKET
#define HAVE_FCNTL
#define HAVE_BZERO
#undef HAVE_IO_H
#undef HAVE__READ
#undef HAVE__SNPRINTF
#define HAVE_POLL_H
#define HAVE_POLL
#define HAVE_SYS_SELECT_H
#define HAVE_SELECT
#undef HAVE_WSASOCKET
#define HAVE_SYS_SOCKIO_H
#define HAVE_WCTYPE_H
#define HAVE_CTYPE_H
#define HAVE_SYS_IOCTL_H
#define HAVE_SOCKLEN_T
#define HAVE_GETPID
#define HAVE_SYSLOG_H
#define HAVE_SYS_TYPES_H
#define HAVE_SYGNAL_H
#define HAVE_ERRNO_H
#define HAVE_STRERROR
#undef HAVE_FINDFIRSTFILE
#define HAVE_OPENDIR
#undef HAVE_STRTOK_S
#define HAVE_GETADDRINFO
#define HAVE_GETNAMEINFO
#define HAVE_SOCKADDR_STORAGE

#define USE_THREAD
#undef HAVE_WINDOWS_H
#define HAVE_PTHREAD_H

#define DLL_EXPORT /* */
#endif

#ifdef __FreeBSD__
#define HAVE_SHA_H
#undef HAVE_OPENSSL_SHA_H
#undef HAVE_SHA1_H
#undef HAVE_SHA1INIT
#undef HAVE_SHA_CTX
#define HAVE_SYS_TIME_H
#define HAVE_UNISTD_H
#define HAVE_INTTYPES_H
#define HAVE_SYS_TIME_H
#define HAVE_NETDB_H
#define HAVE_SYS_SOCKET_H
#define HAVE_NETINET_IN_H
#define HAVE_ARPA_INET_H
#define HAVE_SYS_RESOURCE_H
#define HAVE_FD_SET
#undef HAVE_WINSOCK2_H
#define HAVE_BZERO
#define HAVE_GETTIMEOFDAY
#undef HAVE_TIMEGETTIME
#define HAVE_SNPRINTF
#define HAVE_NETINET_TCP_H
#define HAVE_DEV_RANDOM
#define HAVE_RANDOM
#define HAVE_TIME
#define HAVE_RUSAGE
#undef HAVE_WS2TCPIP_H
#undef HAVE_CLOSESOCKET
#define HAVE_FCNTL
#define HAVE_BZERO
#undef HAVE_IO_H
#undef HAVE__READ
#undef HAVE__SNPRINTF
#undef HAVE_SYS_SOCKIO_H
#define HAVE_SYS_IOCTL_H
#define HAVE_GETPID
#define HAVE_SYSLOG_H
#define HAVE_SYS_TYPES_H

#define HAVE_POLL_H
#define HAVE_POLL
#define HAVE_SYS_SELECT_H
#define HAVE_SELECT
#undef HAVE_WSASOCKET
#undef HAVE_WCTYPE_H
#undef HAVE_CTYPE_H
#define HAVE_SOCKLEN_T
#define HAVE_SYGNAL_H
#define HAVE_GETENV
#define HAVE_ERRNO_H
#define HAVE_STRERROR
#undef HAVE_FINDFIRSTFILE
#define HAVE_OPENDIR
#undef HAVE_STRTOK_S
#define HAVE_GETADDRINFO
#define HAVE_GETNAMEINFO
#define HAVE_SOCKADDR_STORAGE

#define USE_THREAD
#undef HAVE_WINDOWS_H
#define HAVE_PTHREAD_H

#define DLL_EXPORT /* */
#endif

#ifdef __linux__
#define HAVE_OPENSSL_SHA_H
#define HAVE_SHA_CTX
#define HAVE_SYS_TIME_H
#define HAVE_UNISTD_H
#define HAVE_INTTYPES_H
#define HAVE_SYS_TIME_H
#define HAVE_NETDB_H
#define HAVE_SYS_SOCKET_H
#define HAVE_NETINET_IN_H
#define HAVE_ARPA_INET_H
#define HAVE_SYS_RESOURCE_H
#define HAVE_FD_SET
#undef HAVE_WINSOCK2_H
#define HAVE_BZERO
#define HAVE_GETTIMEOFDAY
#undef HAVE_TIMEGETTIME
#define HAVE_SNPRINTF
#define HAVE_NETINET_TCP_H
#define HAVE_DEV_RANDOM
#define HAVE_RANDOM
#define HAVE_TIME
#define HAVE_RUSAGE
#undef HAVE_WS2TCPIP_H
#undef HAVE_CLOSESOCKET
#define HAVE_FCNTL
#define HAVE_BZERO
#undef HAVE_IO_H
#undef HAVE__READ
#undef HAVE__SNPRINTF
#define HAVE_POLL_H
#define HAVE_POLL
#define HAVE_SYS_SELECT_H
#define HAVE_SELECT
#undef HAVE_WSASOCKET
#undef HAVE_SYS_SOCKIO_H
#undef HAVE_WCTYPE_H
#undef HAVE_CTYPE_H
#define HAVE_SYS_IOCTL_H
#define HAVE_SOCKLEN_T
#define HAVE_GETPID
#define HAVE_SYSLOG_H
#define HAVE_SYS_TYPES_H
#define HAVE_SYGNAL_H
#define HAVE_GETENV
#define HAVE_ERRNO_H
#define HAVE_STRERROR
#undef HAVE_FINDFIRSTFILE
#define HAVE_OPENDIR
#undef HAVE_STRTOK_S
#define HAVE_GETADDRINFO
#define HAVE_GETNAMEINFO
#define HAVE_SOCKADDR_STORAGE

#define USE_THREAD
#undef HAVE_WINDOWS_H
#define HAVE_PTHREAD_H

#define DLL_EXPORT /* */
#endif

#ifdef __sun
#undef HAVE_SHA_H
#undef HAVE_OPENSSL_SHA_H
#define HAVE_SHA1_H
#define HAVE_SHA1INIT
#undef HAVE_SHA_CTX
#define HAVE_STDINT_H
#define HAVE_SYS_TIME_H
#define HAVE_UNISTD_H
#define HAVE_INTTYPES_H
#define HAVE_SYS_TIME_H
#define HAVE_NETDB_H
#define HAVE_SYS_SOCKET_H
#define HAVE_NETINET_IN_H
#define HAVE_ARPA_INET_H
#define HAVE_SYS_RESOURCE_H
#define HAVE_FD_SET
#undef HAVE_WINSOCK2_H
#define HAVE_BZERO
#define HAVE_GETTIMEOFDAY
#undef HAVE_TIMEGETTIME
#define HAVE_SNPRINTF
#define HAVE_NETINET_TCP_H
#define HAVE_DEV_RANDOM
#define HAVE_RANDOM
#define HAVE_TIME
#define HAVE_RUSAGE
#undef HAVE_WS2TCPIP_H
#undef HAVE_CLOSESOCKET
#define HAVE_FCNTL
#define HAVE_BZERO
#undef HAVE_IO_H
#undef HAVE__READ
#undef HAVE__SNPRINTF
#define HAVE_POLL_H
#define HAVE_POLL
#define HAVE_SYS_SELECT_H
#define HAVE_SELECT
#undef HAVE_WSASOCKET
#define HAVE_SYS_SOCKIO_H
#define HAVE_WCTYPE_H
#define HAVE_CTYPE_H
#define HAVE_SYS_IOCTL_H
#define HAVE_SOCKLEN_T
#define HAVE_GETPID
#define HAVE_SYSLOG_H
#define HAVE_SYS_TYPES_H
#define HAVE_SYGNAL_H
#define HAVE_ERRNO_H
#define HAVE_STRERROR
#undef HAVE_FINDFIRSTFILE
#define HAVE_OPENDIR
#undef HAVE_STRTOK_S
#define HAVE_GETADDRINFO
#define HAVE_GETNAMEINFO
#define HAVE_SOCKADDR_STORAGE

#define USE_THREAD
#undef HAVE_WINDOWS_H
#define HAVE_PTHREAD_H

#define DLL_EXPORT /* */
#endif

#ifdef __APPLE__ /* for MacOS X */
#define HAVE_STDINT_H
#define HAVE_OPENSSL_SHA_H
#define HAVE_SHA_CTX
#define HAVE_SYS_TIME_H
#define HAVE_UNISTD_H
#define HAVE_INTTYPES_H
#define HAVE_SYS_TIME_H
#define HAVE_NETDB_H
#define HAVE_SYS_SOCKET_H
#define HAVE_NETINET_IN_H
#define HAVE_ARPA_INET_H
#define HAVE_SYS_RESOURCE_H
#define HAVE_FD_SET
#undef HAVE_WINSOCK2_H
#define HAVE_BZERO
#define HAVE_GETTIMEOFDAY
#undef HAVE_TIMEGETTIME
#define HAVE_SNPRINTF
#define HAVE_NETINET_TCP_H
#define HAVE_DEV_RANDOM
#define HAVE_RANDOM
#define HAVE_TIME
#define HAVE_RUSAGE
#undef HAVE_WS2TCPIP_H
#undef HAVE_CLOSESOCKET
#define HAVE_FCNTL
#define HAVE_BZERO
#undef HAVE_IO_H
#undef HAVE__READ
#undef HAVE__SNPRINTF
#define HAVE_POLL_H
#define HAVE_POLL
#define HAVE_SYS_SELECT_H
#define HAVE_SELECT
#undef HAVE_WSASOCKET
#undef HAVE_WCTYPE_H
#define HAVE_CTYPE_H
#define HAVE_SYS_IOCTL_H
#define HAVE_SOCKLEN_T
#define HAVE_GETPID
#define HAVE_SYSLOG_H
#define HAVE_SYS_TYPES_H
#define HAVE_SYGNAL_H
#define HAVE_GETENV
#define HAVE_ERRNO_H
#define HAVE_STRERROR
#undef HAVE_FINDFIRSTFILE
#define HAVE_OPENDIR
#undef HAVE_STRTOK_S
#define HAVE_GETADDRINFO
#define HAVE_GETNAMEINFO
#define HAVE_SOCKADDR_STORAGE

#define USE_THREAD
#undef HAVE_WINDOWS_H
#define HAVE_PTHREAD_H
#undef HAVE_SYS_SOCKIO_H

#define DLL_EXPORT /* */
#endif

#if defined(_WIN32) || defined(WIN64)

#define _LIBCOOKAI_WINDOWS_

/* for warning C4996 (_read _write _close and so on) */
#pragma warning ( disable : 4996 )
#define _CRT_SECURE_NO_DEPRECATE

#undef HAVE_SYS_TIME_H
#undef HAVE_UNISTD_H
#undef HAVE_INTTYPES_H
#undef HAVE_SYS_TIME_H
#undef HAVE_NETDB_H
#undef HAVE_SYS_SOCKET_H
#undef HAVE_NETINET_IN_H
#undef HAVE_ARPA_INET_H
#undef HAVE_SYS_RESOURCE_H
#undef HAVE_FD_SET
#define HAVE_WINSOCK2_H
#include "Win32/inttypes.h"
#undef HAVE_BZERO
#undef HAVE_GETTIMEOFDAY
#define HAVE_TIMEGETTIME
#undef HAVE_SNPRINTF
#undef HAVE_NETINET_TCP_H
#undef HAVE_DEV_RANDOM
#undef HAVE_RANDOM
#undef HAVE_TIME
#undef HAVE_RUSAGE
#define HAVE_WS2TCPIP_H
#define HAVE_CLOSESOCKET
#define HAVE_SHA_H
#undef HAVE_FCNTL
#define HAVE_IOCTLSOCKET
#define HAVE_WSAGETLASTERROR
#undef HAVE_BZERO
#define HAVE_WSASTARTUP
#define HAVE_mciSendStringW
#define HAVE_MMSYSTEM_H
#define HAVE__READ
#define HAVE__SNPRINTF
#define HAVE__POPEN
#define HAVE_SLEEPEX
#undef HAVE_POLL_H
#undef HAVE_POLL
#undef HAVE_SYS_SELECT_H
#define HAVE_SELECT
#define HAVE_WSASOCKET
#define HAVE_WSACONNECT
#undef HAVE_SYS_SOCKIO_H
#undef HAVE_WCTYPE_H
#undef HAVE_CTYPE_H
#undef HAVE_SYS_IOCTL_H
#define HAVE_GETPID
#undef HAVE_SYSLOG_H
#define HAVE_ERRNO_H
#define HAVE_GETADDRINFO
#define HAVE_GETNAMEINFO
#define HAVE_SOCKADDR_STORAGE

#define USE_THREAD
#define HAVE_WINDOWS_H
#define HAVE_WINSOCK2_H
#define HAVE_PROCESS_H
#undef HAVE_PTHREAD_H
#undef HAVE_SOCKLEN_T
#define HAVE_SYS_TYPES_H
#define HAVE_SYGNAL_H
#define HAVE_GETENV
#define HAVE_IO_H
#define HAVE_STRERROR
#define HAVE_STRTOK_S
#define HAVE_GETSYSTEMTIMEASFILETIME

#define NSNS_NONBLOCK
#define NSNS_NODELAY

#define DLL_EXPORT __declspec(dllexport)
#define _CRT_SECURE_NO_WARNINGS
#define HAVE_FINDFIRSTFILE
#undef HAVE_OPENDIR

#ifdef _WIN32_WCE
#undef HAVE_SYS_TYPES_H
#undef HAVE_SIGNAL_H
#undef HAVE_GETENV
#undef HAVE_IO_H
#undef HAVE_PROCESS_H
#undef HAVE_ERRNO_H
#undef HAVE_STRERROR
#undef HAVE_GETPID
#undef HAVE_STRTOK_S
#undef HAVE_GETSYSTEMTIMEASFILETIME
#endif

#define strtok_r strtok_s
#define strdup _strdup
#define snprintf _snprintf
#define getpid _getpid
#define close _close
#define read _read

#else /* _WIN32 */
typedef int SOCKET;
#endif

#ifdef __NDS

#undef HAVE_SYS_TIME_H
#define HAVE_UNISTD_H
#define HAVE_INTTYPES_H
#undef HAVE_SYS_TIME_H
#define HAVE_NETDB_H
#define HAVE_SYS_SOCKET_H
#define HAVE_NETINET_IN_H
#undef HAVE_ARPA_INET_H
#undef HAVE_SYS_RESOURCE_H
#undef HAVE_FD_SET
#undef HAVE_WINSOCK2_H
#undef HAVE_BZERO
#undef HAVE_GETTIMEOFDAY
#undef HAVE_TIMEGETTIME
#undef HAVE_SNPRINTF
#undef HAVE_NETINET_TCP_H
#undef HAVE_DEV_RANDOM
#undef HAVE_RANDOM
#undef HAVE_TIME
#undef HAVE_RUSAGE
#undef HAVE_WS2TCPIP_H
#undef HAVE_CLOSESOCKET
#undef HAVE_SHA_H
#undef HAVE_FCNTL
#undef HAVE_IOCTLSOCKET
#undef HAVE_WSAGETLASTERROR
#undef HAVE_BZERO
#undef HAVE_WSASTARTUP
#undef HAVE_mciSendStringW
#undef HAVE_MMSYSTEM_H
#undef HAVE__READ
#undef HAVE__SNPRINTF
#undef HAVE__POPEN
#undef HAVE_SLEEPEX
#undef HAVE_POLL_H
#undef HAVE_POLL
#undef HAVE_SYS_SELECT_H
#define HAVE_SELECT
#undef HAVE_WSASOCKET
#undef HAVE_WSACONNECT
#undef HAVE_SYS_SOCKIO_H
#undef HAVE_WCTYPE_H
#undef HAVE_CTYPE_H
#undef HAVE_SYS_IOCTL_H
#undef HAVE_GETPID
#undef HAVE_SYSLOG_H
#define HAVE_ERRNO_H
#undef HAVE_GETADDRINFO
#undef HAVE_GETNAMEINFO
#undef HAVE_SOCKADDR_STORAGE

#undef USE_THREAD
#undef HAVE_WINDOWS_H
#undef HAVE_WINSOCK2_H
#undef HAVE_PROCESS_H
#undef HAVE_PTHREAD_H
#undef HAVE_SOCKLEN_T
#define HAVE_SYS_TYPES_H
#define HAVE_SYGNAL_H
#undef HAVE_GETENV
#undef HAVE_IO_H
#undef HAVE_STRERROR
#undef HAVE_STRTOK_S
#undef HAVE_GETSYSTEMTIMEASFILETIME

#define NSNS_NONBLOCK
#define NSNS_NODELAY

#undef DLL_EXPORT
#undef HAVE_FINDFIRSTFILE
#undef HAVE_OPENDIR

#endif

#endif /* PASTRY_LIKE_CONFIG_H */
