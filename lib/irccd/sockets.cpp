/*
 * sockets.cpp -- portable C++ socket wrappers
 *
 * Copyright (c) 2013-2016 David Demelier <markand@malikania.fr>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#define TIMEOUT_MSG "operation timeout"

#include <algorithm>
#include <atomic>
#include <cstring>
#include <mutex>

#include "sockets.hpp"

namespace irccd {

namespace net {

/*
 * Portable constants
 * ------------------------------------------------------------------
 */

/* {{{ Constants */

#if defined(_WIN32)

const Handle Invalid{INVALID_SOCKET};
const int Failure{SOCKET_ERROR};

#else

const Handle Invalid{-1};
const int Failure{-1};

#endif

/* }}} */

/*
 * Portable functions
 * ------------------------------------------------------------------
 */

/* {{{ Functions */

#if defined(_WIN32)

namespace {

static std::mutex s_mutex;
static std::atomic<bool> s_initialized{false};

} // !namespace

#endif // !_WIN32

void init() noexcept
{
#if defined(_WIN32)
	std::lock_guard<std::mutex> lock(s_mutex);

	if (!s_initialized) {
		s_initialized = true;

		WSADATA wsa;
		WSAStartup(MAKEWORD(2, 2), &wsa);

		/*
		 * If SOCKET_WSA_NO_INIT is not set then the user
		 * must also call finish himself.
		 */
#if !defined(SOCKET_NO_AUTO_INIT)
		atexit(finish);
#endif
	}
#endif
}

void finish() noexcept
{
#if defined(_WIN32)
	WSACleanup();
#endif
}

std::string error(int errn)
{
#if defined(_WIN32)
	LPSTR str = nullptr;
	std::string errmsg = "Unknown error";

	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		errn,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&str, 0, NULL);


	if (str) {
		errmsg = std::string(str);
		LocalFree(str);
	}

	return errmsg;
#else
	return strerror(errn);
#endif
}

std::string error()
{
#if defined(_WIN32)
	return error(WSAGetLastError());
#else
	return error(errno);
#endif
}

/* }}} */

/*
 * SSL stuff
 * ------------------------------------------------------------------
 */

/* {{{ SSL initialization */

#if !defined(SOCKET_NO_SSL)

namespace ssl {

namespace {

std::mutex mutex;
std::atomic<bool> initialized{false};

} // !namespace

void finish() noexcept
{
	ERR_free_strings();
}

void init() noexcept
{
	std::lock_guard<std::mutex> lock{mutex};

	if (!initialized) {
		initialized = true;

		SSL_library_init();
		SSL_load_error_strings();
		OpenSSL_add_all_algorithms();

#if !defined(SOCKET_NO_AUTO_SSL_INIT)
		atexit(finish);
#endif // SOCKET_NO_AUTO_SSL_INIT
	}
}

} // !ssl

#endif // SOCKET_NO_SSL

/* }}} */

/*
 * Error class
 * ------------------------------------------------------------------
 */

/* {{{ Error */

Error::Error(Code code, std::string function)
	: m_code{code}
	, m_function{std::move(function)}
	, m_error{error()}
{
}

Error::Error(Code code, std::string function, int n)
	: m_code{code}
	, m_function{std::move(function)}
	, m_error{error(n)}
{
}

Error::Error(Code code, std::string function, std::string error)
	: m_code{code}
	, m_function{std::move(function)}
	, m_error{std::move(error)}
{
}

/* }}} */

/*
 * Predefine addressed to be used
 * ------------------------------------------------------------------
 */

/* {{{ Addresses */

namespace address {

/* Default domain */
int Ip::m_default{AF_INET};

Ip::Ip(Type domain) noexcept
	: m_domain(static_cast<int>(domain))
{
	assert(m_domain == AF_INET6 || m_domain == AF_INET);

	if (m_domain == AF_INET6) {
		std::memset(&m_sin6, 0, sizeof (sockaddr_in6));
	} else {
		std::memset(&m_sin, 0, sizeof (sockaddr_in));
	}
}

Ip::Ip(const std::string &host, int port, Type domain)
	: m_domain(static_cast<int>(domain))
{
	assert(m_domain == AF_INET6 || m_domain == AF_INET);

	if (host == "*") {
		if (m_domain == AF_INET6) {
			std::memset(&m_sin6, 0, sizeof (sockaddr_in6));

			m_length = sizeof (sockaddr_in6);
			m_sin6.sin6_addr = in6addr_any;
			m_sin6.sin6_family = AF_INET6;
			m_sin6.sin6_port = htons(port);
		} else {
			std::memset(&m_sin, 0, sizeof (sockaddr_in));

			m_length = sizeof (sockaddr_in);
			m_sin.sin_addr.s_addr = INADDR_ANY;
			m_sin.sin_family = AF_INET;
			m_sin.sin_port = htons(port);
		}
	} else {
		addrinfo hints, *res;

		std::memset(&hints, 0, sizeof (addrinfo));
		hints.ai_family = domain;

		auto error = getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &res);
		if (error != 0) {
			throw Error{Error::System, "getaddrinfo", gai_strerror(error)};
		}

		if (m_domain == AF_INET6) {
			std::memcpy(&m_sin6, res->ai_addr, res->ai_addrlen);
		} else {
			std::memcpy(&m_sin, res->ai_addr, res->ai_addrlen);
		}

		m_length = res->ai_addrlen;
		freeaddrinfo(res);
	}
}

Ip::Ip(const sockaddr_storage *ss, socklen_t length) noexcept
	: m_length{length}
	, m_domain{ss->ss_family}
{
	assert(ss->ss_family == AF_INET6 || ss->ss_family == AF_INET);

	if (ss->ss_family == AF_INET6) {
		std::memcpy(&m_sin6, ss, length);
	} else if (ss->ss_family == AF_INET) {
		std::memcpy(&m_sin, ss, length);
	}
}

#if !defined(_WIN32)

Local::Local() noexcept
{
	std::memset(&m_sun, 0, sizeof (sockaddr_un));
}

Local::Local(std::string path, bool rm) noexcept
	: m_path{std::move(path)}
{
	/* Silently remove the file even if it fails */
	if (rm) {
		::remove(m_path.c_str());
	}

	/* Copy the path */
	std::memset(m_sun.sun_path, 0, sizeof (m_sun.sun_path));
	std::strncpy(m_sun.sun_path, m_path.c_str(), sizeof (m_sun.sun_path) - 1);

	/* Set the parameters */
	m_sun.sun_family = AF_LOCAL;
}

Local::Local(const sockaddr_storage *ss, socklen_t length) noexcept
{
	assert(ss->ss_family == AF_LOCAL);

	if (ss->ss_family == AF_LOCAL) {
		std::memcpy(&m_sun, ss, length);
		m_path = reinterpret_cast<const sockaddr_un &>(m_sun).sun_path;
	}
}

#endif // !_WIN32

} // !address

/* }}} */

/*
 * Select
 * ------------------------------------------------------------------
 */

/* {{{ Select */

std::vector<ListenerStatus> Select::wait(const ListenerTable &table, int ms)
{
	timeval maxwait, *towait;
	fd_set readset;
	fd_set writeset;

	FD_ZERO(&readset);
	FD_ZERO(&writeset);

	Handle max = 0;

	for (const auto &pair : table) {
		if ((pair.second & Condition::Readable) == Condition::Readable) {
			FD_SET(pair.first, &readset);
		}
		if ((pair.second & Condition::Writable) == Condition::Writable) {
			FD_SET(pair.first, &writeset);
		}

		if (pair.first > max) {
			max = pair.first;
		}
	}

	maxwait.tv_sec = 0;
	maxwait.tv_usec = ms * 1000;

	// Set to nullptr for infinite timeout.
	towait = (ms < 0) ? nullptr : &maxwait;

	auto error = ::select(max + 1, &readset, &writeset, nullptr, towait);
	if (error == Failure) {
		throw Error{Error::System, "select"};
	}
	if (error == 0) {
		throw Error{Error::Timeout, "select", TIMEOUT_MSG};
	}

	std::vector<ListenerStatus> sockets;

	for (const auto &pair : table) {
		if (FD_ISSET(pair.first, &readset)) {
			sockets.push_back(ListenerStatus{pair.first, Condition::Readable});
		}
		if (FD_ISSET(pair.first, &writeset)) {
			sockets.push_back(ListenerStatus{pair.first, Condition::Writable});
		}
	}

	return sockets;
}

/* }}} */

/*
 * Poll
 * ------------------------------------------------------------------
 */

/* {{{ Poll */

/*
 * Poll implementation
 * ------------------------------------------------------------------
 */

#if defined(SOCKET_HAVE_POLL)

#if defined(_WIN32)
#  define poll WSAPoll
#endif

short Poll::toPoll(Condition condition) const noexcept
{
	short result(0);

	if ((condition & Condition::Readable) == Condition::Readable) {
		result |= POLLIN;
	}
	if ((condition & Condition::Writable) == Condition::Writable) {
		result |= POLLOUT;
	}

	return result;
}

Condition Poll::toCondition(short &event) const noexcept
{
	Condition condition{Condition::None};

	/*
	 * Poll implementations mark the socket differently regarding
	 * the disconnection of a socket.
	 *
	 * At least, even if POLLHUP or POLLIN is set, recv() always
	 * return 0 so we mark the socket as readable.
	 */
	if ((event & POLLIN) || (event & POLLHUP)) {
		condition |= Condition::Readable;
	}
	if (event & POLLOUT) {
		condition |= Condition::Writable;
	}

	/* Reset event for safety */
	event = 0;

	return condition;
}

void Poll::set(const ListenerTable &, Handle h, Condition condition, bool add)
{
	if (add) {
		m_fds.push_back(pollfd{h, toPoll(condition), 0});
	} else {
		auto it = std::find_if(m_fds.begin(), m_fds.end(), [&] (const pollfd &pfd) {
			return pfd.fd == h;
		});

		it->events |= toPoll(condition);
	}
}

void Poll::unset(const ListenerTable &, Handle h, Condition condition, bool remove)
{
	auto it = std::find_if(m_fds.begin(), m_fds.end(), [&] (const pollfd &pfd) {
		return pfd.fd == h;
	});

	if (remove) {
		m_fds.erase(it);
	} else {
		it->events &= ~(toPoll(condition));
	}
}

std::vector<ListenerStatus> Poll::wait(const ListenerTable &, int ms)
{
	auto result = poll(m_fds.data(), m_fds.size(), ms);
	if (result == 0) {
		throw Error{Error::Timeout, "select", TIMEOUT_MSG};
	}
	if (result < 0) {
		throw Error{Error::System, "poll"};
	}

	std::vector<ListenerStatus> sockets;
	for (auto &fd : m_fds) {
		if (fd.revents != 0) {
			sockets.push_back(ListenerStatus{fd.fd, toCondition(fd.revents)});
		}
	}

	return sockets;
}

#endif // !SOCKET_HAVE_POLL

/* }}} */

/*
 * Epoll implementation
 * ------------------------------------------------------------------
 */

/* {{{ Epoll */

#if defined(SOCKET_HAVE_EPOLL)

uint32_t Epoll::toEpoll(Condition condition) const noexcept
{
	uint32_t events = 0;

	if ((condition & Condition::Readable) == Condition::Readable) {
		events |= EPOLLIN;
	}
	if ((condition & Condition::Writable) == Condition::Writable) {
		events |= EPOLLOUT;
	}

	return events;
}

Condition Epoll::toCondition(uint32_t events) const noexcept
{
	Condition condition{Condition::None};

	if ((events & EPOLLIN) || (events & EPOLLHUP)) {
		condition |= Condition::Readable;
	}
	if (events & EPOLLOUT) {
		condition |= Condition::Writable;
	}

	return condition;
}

void Epoll::update(Handle h, int op, int eflags)
{
	epoll_event ev;

	std::memset(&ev, 0, sizeof (epoll_event));

	ev.events = eflags;
	ev.data.fd = h;

	if (epoll_ctl(m_handle, op, h, &ev) < 0) {
		throw Error{Error::System, "epoll_ctl"};
	}
}

Epoll::Epoll()
	: m_handle{epoll_create1(0)}
{
	if (m_handle < 0) {
		throw Error{Error::System, "epoll_create"};
	}
}

Epoll::~Epoll()
{
	close(m_handle);
}

/*
 * For set and unset, we need to apply the whole flags required, so if the socket
 * was set to Connection::Readable and user add Connection::Writable, we must
 * place both.
 */
void Epoll::set(const ListenerTable &table, Handle sc, Condition condition, bool add)
{
	if (add) {
		update(sc, EPOLL_CTL_ADD, toEpoll(condition));
		m_events.resize(m_events.size() + 1);
	} else {
		update(sc, EPOLL_CTL_MOD, toEpoll(table.at(sc) | condition));
	}
}

/*
 * Unset is a bit complicated case because Listener tells us which
 * flag to remove but to update epoll descriptor we need to pass
 * the effective flags that we want to be applied.
 *
 * So we put the same flags that are currently effective and remove the
 * requested one.
 */
void Epoll::unset(const ListenerTable &table, Handle sc, Condition condition, bool remove)
{
	if (remove) {
		update(sc, EPOLL_CTL_DEL, 0);
		m_events.resize(m_events.size() - 1);
	} else {
		update(sc, EPOLL_CTL_MOD, toEpoll(table.at(sc) & ~(condition)));
	}
}

std::vector<ListenerStatus> Epoll::wait(const ListenerTable &, int ms)
{
	int ret = epoll_wait(m_handle, m_events.data(), m_events.size(), ms);
	std::vector<ListenerStatus> result;

	if (ret == 0) {
		throw Error{Error::Timeout, "epoll_wait", TIMEOUT_MSG};
	}
	if (ret < 0) {
		throw Error{Error::System, "epoll_wait"};
	}

	for (int i = 0; i < ret; ++i) {
		result.push_back(ListenerStatus{m_events[i].data.fd, toCondition(m_events[i].events)});
	}

	return result;
}

#endif // !SOCKET_HAVE_EPOLL

/* }}} */

/*
 * Kqueue implementation
 * ------------------------------------------------------------------
 */

/* {{{ Kqueue */

#if defined(SOCKET_HAVE_KQUEUE)

Kqueue::Kqueue()
	: m_handle(kqueue())
{
	if (m_handle < 0) {
		throw Error{Error::System, "kqueue"};
	}
}

Kqueue::~Kqueue()
{
	close(m_handle);
}

void Kqueue::update(Handle h, int filter, int kflags)
{
	struct kevent ev;

	EV_SET(&ev, h, filter, kflags, 0, 0, nullptr);

	if (kevent(m_handle, &ev, 1, nullptr, 0, nullptr) < 0) {
		throw Error{Error::System, "kevent"};
	}
}

void Kqueue::set(const ListenerTable &, Handle h, Condition condition, bool add)
{
	if ((condition & Condition::Readable) == Condition::Readable) {
		update(h, EVFILT_READ, EV_ADD | EV_ENABLE);
	}
	if ((condition & Condition::Writable) == Condition::Writable) {
		update(h, EVFILT_WRITE, EV_ADD | EV_ENABLE);
	}

	if (add) {
		m_result.resize(m_result.size() + 1);
	}
}

void Kqueue::unset(const ListenerTable &, Handle h, Condition condition, bool remove)
{
	if ((condition & Condition::Readable) == Condition::Readable) {
		update(h, EVFILT_READ, EV_DELETE);
	}
	if ((condition & Condition::Writable) == Condition::Writable) {
		update(h, EVFILT_WRITE, EV_DELETE);
	}

	if (remove) {
		m_result.resize(m_result.size() - 1);
	}
}

std::vector<ListenerStatus> Kqueue::wait(const ListenerTable &, int ms)
{
	std::vector<ListenerStatus> sockets;
	timespec ts = { 0, 0 };
	timespec *pts = (ms <= 0) ? nullptr : &ts;

	ts.tv_sec = ms / 1000;
	ts.tv_nsec = (ms % 1000) * 1000000;

	int nevents = kevent(m_handle, nullptr, 0, &m_result[0], m_result.capacity(), pts);

	if (nevents == 0) {
		throw Error{Error::Timeout, "kevent", TIMEOUT_MSG};
	}
	if (nevents < 0) {
		throw Error{Error::System, "kevent"};
	}

	for (int i = 0; i < nevents; ++i) {
		sockets.push_back(ListenerStatus{
			static_cast<Handle>(m_result[i].ident),
			m_result[i].filter == EVFILT_READ ? Condition::Readable : Condition::Writable
		});
	}

	return sockets;
}

#endif // !SOCKET_HAVE_KQUEUE

/* }}} */

} // !net

} // !irccd
