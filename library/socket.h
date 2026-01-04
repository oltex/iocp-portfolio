#pragma once
#include "function.h"
#include "pair.h"
#include "tuple.h"
#include "string.h"
#include "overlap.h"
#include <intrin.h>
#include <optional>
#pragma comment(lib,"ws2_32.lib")
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "mswsock.lib")
#include <MSWSock.h>

namespace library {
	inline static void wsa_start_up(void) noexcept {
		WSAData wsadata;
		if (0 != ::WSAStartup(0x0202, &wsadata))
			::__debugbreak();
	};
	inline static void wsa_clean_up(void) noexcept {
		if (SOCKET_ERROR == ::WSACleanup())
			::__debugbreak();
	};
	inline static auto select(fd_set* read, fd_set* write, fd_set* exception, timeval* time) noexcept -> int {
		int result = ::select(0, read, write, exception, time);
		if (SOCKET_ERROR == result)
			::__debugbreak();
		return result;
	}

	class socket_address {
	public:
		inline socket_address(void) noexcept = default;
		inline socket_address(socket_address const&) noexcept = default;
		inline socket_address(socket_address&&) noexcept = default;
		inline auto operator=(socket_address const&) noexcept -> socket_address & = default;
		inline auto operator=(socket_address&&) noexcept -> socket_address & = default;
		inline ~socket_address(void) noexcept = default;

		inline virtual auto family(void) const noexcept -> ADDRESS_FAMILY = 0;
		inline virtual auto size(void) const noexcept -> int = 0;
		inline virtual auto data(void) const noexcept -> sockaddr const& = 0;
		inline virtual auto data(void) noexcept -> sockaddr & = 0;
	};
	class socket_address_ipv4 final : public socket_address {
		sockaddr_in _sockaddr;
	public:
		inline socket_address_ipv4(void) noexcept
			: _sockaddr() {
			_sockaddr.sin_family = AF_INET;
		}
		inline socket_address_ipv4(char const* const address, unsigned short port) noexcept 
			: socket_address_ipv4() {
			socket_address_ipv4::ip(address);
			socket_address_ipv4::port(port);
		}
		inline socket_address_ipv4(sockaddr_in sockaddr) noexcept
			: _sockaddr(sockaddr) {
		}
		inline socket_address_ipv4(socket_address_ipv4 const&) noexcept = default;
		inline socket_address_ipv4(socket_address_ipv4&&) noexcept = default;
		inline auto operator=(socket_address_ipv4 const&) noexcept -> socket_address_ipv4 & = default;
		inline auto operator=(socket_address_ipv4&&) noexcept -> socket_address_ipv4 & = default;
		inline ~socket_address_ipv4(void) noexcept = default;

		inline void ip(unsigned long address) noexcept {
			_sockaddr.sin_addr.S_un.S_addr = ::htonl(INADDR_ANY);
		}
		inline void ip(char const* const address) noexcept {
			if (1 != ::inet_pton(AF_INET, address, &_sockaddr.sin_addr))
				::__debugbreak();
		}
		inline auto ip(void) const noexcept -> library::string {
			char string[INET_ADDRSTRLEN];
			if (0 == ::inet_ntop(AF_INET, &_sockaddr.sin_addr, string, INET_ADDRSTRLEN))
				::__debugbreak();
			return library::string(string);
		}
		inline void port(unsigned short port) noexcept {
			_sockaddr.sin_port = htons(port);
		}
		inline auto port(void) const noexcept -> unsigned short {
			return ::ntohs(_sockaddr.sin_port);
		}
		inline virtual auto family(void) const noexcept -> ADDRESS_FAMILY override {
			return AF_INET;
		}
		inline virtual auto size(void) const noexcept -> int override {
			return sizeof(sockaddr_in);
		}
		inline virtual auto data(void) const noexcept -> sockaddr const& override {
			return library::cast<sockaddr const&>(_sockaddr);
		}
		inline virtual auto data(void) noexcept -> sockaddr & override {
			return library::cast<sockaddr&>(library::cast<socket_address_ipv4 const&>(*this).data());
		}
	};
	class socket_address_ipv6 : public socket_address {
		sockaddr_in6 _sockaddr;
	public:
		inline socket_address_ipv6(void) noexcept
			: _sockaddr() {
			_sockaddr.sin6_family = AF_INET6;
		}
		inline socket_address_ipv6(sockaddr_in6 sockaddr) noexcept
			: _sockaddr(sockaddr) {
		}
		inline socket_address_ipv6(socket_address_ipv6 const&) noexcept = default;
		inline socket_address_ipv6(socket_address_ipv6&&) noexcept = default;
		inline auto operator=(socket_address_ipv6 const&) noexcept -> socket_address_ipv6 & = default;
		inline auto operator=(socket_address_ipv6&&) noexcept -> socket_address_ipv6 & = default;
		inline ~socket_address_ipv6(void) noexcept = default;

		inline virtual auto family(void) const noexcept -> ADDRESS_FAMILY override {
			return _sockaddr.sin6_family;
		}
		inline virtual auto size(void) const noexcept -> int override {
			return sizeof(sockaddr_in6);
		}
		inline virtual auto data(void) const noexcept -> sockaddr const& override {
			return library::cast<sockaddr const&>(_sockaddr);
		}
		inline virtual auto data(void) noexcept -> sockaddr & override {
			return library::cast<sockaddr&>(library::cast<socket_address_ipv6 const&>(*this).data());
		}
	};
	class socket_address_storage final : public socket_address {
	protected:
		union {
			sockaddr _sockaddr;
			sockaddr_in _sockaddr_in4;
			sockaddr_in6 _sockaddr_in6;
		};
	public:
		inline socket_address_storage(void) noexcept
			:_sockaddr_in6{ .sin6_family = AF_UNSPEC } {
		};
		inline socket_address_storage(sockaddr const& sockaddr, int length) noexcept
			: _sockaddr_in6{} {
			assert(0 < length && sizeof(sockaddr_in6) >= length);
			library::memory_copy(library::cast<void*>(&_sockaddr), library::cast<void const*>(&sockaddr), length);
		}
		inline socket_address_storage(socket_address const& address) noexcept
			: _sockaddr_in6{} {
			assert(0 < address.size() && sizeof(sockaddr_in6) >= address.size());
			library::memory_copy(library::cast<void*>(&_sockaddr), library::cast<void const*>(&address.data()), static_cast<size_t>(address.size()));
		}
		inline socket_address_storage(socket_address_storage const&) noexcept = default;
		inline socket_address_storage(socket_address_storage&&) noexcept = default;
		inline auto operator=(socket_address_storage const&) noexcept -> socket_address_storage & = default;
		inline auto operator=(socket_address_storage&&) noexcept -> socket_address_storage & = default;
		inline ~socket_address_storage(void) noexcept = default;

		inline virtual auto family(void) const noexcept -> ADDRESS_FAMILY override {
			return _sockaddr.sa_family;
		}
		inline virtual auto size(void) const noexcept -> int override {
			switch (family()) {
			case AF_INET:
				return sizeof(sockaddr_in);
			case AF_INET6:
				return sizeof(sockaddr_in6);
			}
			return sizeof(sockaddr_in6);
		}
		inline virtual auto data(void) const noexcept -> sockaddr const& override {
			return _sockaddr;
		}
		inline virtual auto data(void) noexcept -> sockaddr & override {
			return library::cast<sockaddr&>(library::cast<socket_address_storage const&>(*this).data());
		}
	};

	class socket final {
		SOCKET _socket;
	public:
		enum class result : unsigned char {
			complet, pending, close, fail
		};
		inline socket(void) noexcept
			: _socket(INVALID_SOCKET) {
		}
		inline socket(ADDRESS_FAMILY const address_family, int const type, int const protocol) noexcept
			: _socket(::socket(address_family, type, protocol)) {
			if (INVALID_SOCKET == _socket) {
				switch (::GetLastError()) {
				case WSANOTINITIALISED:
				default:
					::__debugbreak();
				}
			}
		}
		inline socket(ADDRESS_FAMILY const address_family, int const type, int const protocol, unsigned long const flag) noexcept
			: _socket(::WSASocketW(address_family, type, protocol, nullptr, 0, flag)) {
			if (_socket == INVALID_SOCKET) {
				switch (::WSAGetLastError()) {
				case WSANOTINITIALISED:
				default:
					::__debugbreak();
				}
			}
		}
		inline socket(SOCKET const sock) noexcept
			: _socket(sock) {
		}
		inline socket(socket const&) noexcept = delete;
		inline socket(socket&& rhs) noexcept
			: _socket(library::exchange(rhs._socket, INVALID_SOCKET)) {
		}
		inline auto operator=(socket const&) noexcept -> socket & = delete;
		inline auto operator=(socket&& rhs) noexcept -> socket& {
			if (INVALID_SOCKET != _socket)
				::closesocket(_socket);
			_socket = library::exchange(rhs._socket, INVALID_SOCKET);
			return *this;
		};
		inline ~socket(void) noexcept {
			if (INVALID_SOCKET != _socket)
				::closesocket(_socket);
		}

		inline void create(ADDRESS_FAMILY const address_family, int const type, int const protocol) noexcept {
			_socket = ::socket(address_family, type, protocol);
			if (INVALID_SOCKET == _socket) {
				switch (::GetLastError()) {
				case WSANOTINITIALISED:
				default:
					::__debugbreak();
				}
			}
		}
		inline void create(ADDRESS_FAMILY const address_family, int const type, int const protocol, unsigned long const flag) noexcept {
			_socket = ::WSASocketW(address_family, type, protocol, nullptr, 0, flag);
			if (_socket == INVALID_SOCKET) {
				switch (::WSAGetLastError()) {
				case WSANOTINITIALISED:
				default:
					::__debugbreak();
				}
			}
		}
		inline auto bind(socket_address const& socket_address) const noexcept -> int {
			int result = ::bind(_socket, &socket_address.data(), socket_address.size());
			if (SOCKET_ERROR == result) {
				switch (GetLastError()) {
				case WSAEADDRINUSE:
				default:
					::__debugbreak();
				}
			}
			return result;
		}
		inline auto listen(int backlog) const noexcept -> int {
			int result = ::listen(_socket, SOMAXCONN == backlog ? backlog : SOMAXCONN_HINT(backlog));
			if (SOCKET_ERROR == result) {
				switch (::GetLastError()) {
				case WSAEINVAL:
				default:
					::__debugbreak();
				}
			}
			return result;
		}
		inline auto accept(void) const noexcept -> library::pair<socket, socket_address_storage> {
			socket_address_storage socket_address;
			int length = socket_address.size();
			SOCKET sock = ::accept(_socket, &socket_address.data(), &length);
			if (INVALID_SOCKET == sock) {
				switch (::GetLastError()) {
				case WSAEWOULDBLOCK:
				case WSAEINVAL:
				case WSAENOTSOCK:
				case WSAEINTR:
					break;
				default:
					::__debugbreak();
				}
			}
			return library::pair<socket, socket_address_storage>(sock, socket_address);
		}
		inline auto connect(socket_address const& socket_address) noexcept -> int {
			int result = ::connect(_socket, &socket_address.data(), socket_address.size());
			if (SOCKET_ERROR == result) {
				switch (::GetLastError()) {
				case WSAEWOULDBLOCK:
					break;
				case WSAETIMEDOUT:
				case WSAECONNREFUSED:
					close();
					break;
				default:
					::__debugbreak();
				}
			}
			return result;
		}
		inline void shutdown(int const how) const noexcept {
			if (SOCKET_ERROR == ::shutdown(_socket, how)) {
				switch (::WSAGetLastError()) {
				default:
					::__debugbreak();
#pragma warning(suppress: 4065)
				}
			}
		}
		inline void close(void) noexcept {
			if (INVALID_SOCKET != _socket) {
				if (SOCKET_ERROR == ::closesocket(_socket)) {
					switch (::WSAGetLastError()) {
					default:
						::__debugbreak();
#pragma warning(suppress: 4065)
					}
				}
				_socket = INVALID_SOCKET;
			}
		}
		inline auto receive(char* const buffer, int const length, int const flag) noexcept -> int {
			int result = ::recv(_socket, buffer, length, flag);
			if (SOCKET_ERROR == result) {
				switch (::GetLastError()) {
				case WSAEWOULDBLOCK:
					break;
				case WSAECONNRESET:
				case WSAECONNABORTED:
					close();
					break;
				case WSAENOTCONN:
				default:
					::__debugbreak();
				}
			}
			else if (0 == result)
				close();
			return result;
		}
		inline auto receive_from(char* const buffer, int const length, int const flag) noexcept -> library::tuple<int, socket_address_storage, int> {
			library::tuple<int, socket_address_storage, int> result;
			result.get<2>() = result.get<1>().size();
			result.get<0>() = ::recvfrom(_socket, buffer, length, flag, &result.get<1>().data(), &result.get<2>());
			if (SOCKET_ERROR == result.get<0>()) {
				switch (::WSAGetLastError()) {
				default:
					::__debugbreak();
#pragma warning(suppress: 4065)
				}
			}
			return result;
		}
		inline auto receive(WSABUF* buffer, unsigned long count, unsigned long* byte, unsigned long* flag) noexcept -> int {
			int result = ::WSARecv(_socket, buffer, count, byte, flag, nullptr, nullptr);
			if (SOCKET_ERROR == result) {
				switch (::GetLastError()) {
				case WSAECONNRESET:
				case WSAECONNABORTED:
					close();
					break;
				case WSAENOTSOCK:
				default:
					::__debugbreak();
				}
			}
			return result;
		}
		inline auto receive(WSABUF* buffer, unsigned long count, unsigned long* byte, unsigned long* flag, overlap& overlap) noexcept -> result {
			overlap.clear();
			if (SOCKET_ERROR == ::WSARecv(_socket, buffer, count, byte, flag, &overlap.data(), nullptr)) {
				switch (::GetLastError()) {
				case WSA_IO_PENDING:
					return result::pending;
				case WSAECONNRESET:
				case WSAECONNABORTED:
					return result::close;
				case WSAENOTSOCK:
				default:
					::__debugbreak();
				}
			}
			return result::complet;
		}
		inline auto receive_from(WSABUF* buffer, unsigned long count, unsigned long* byte, unsigned long* flag, socket_address& address, overlap& overlap) noexcept -> result {
			overlap.clear();
			int length = address.size();
			if (SOCKET_ERROR == ::WSARecvFrom(_socket, buffer, count, byte, flag, &address.data(), &length, &overlap.data(), nullptr)) {
				switch (::GetLastError()) {
				case WSA_IO_PENDING:
					return result::pending;
				case WSAECONNRESET:
				case WSAECONNABORTED:
					return result::close;
				case WSAENOTSOCK:
				default:
					::__debugbreak();
				}
			}
			return result::complet;
		}
		inline auto send(char const* const buffer, int const length, int const flag) noexcept -> int {
			int result = ::send(_socket, buffer, length, flag);
			if (SOCKET_ERROR == result) {
				switch (::GetLastError()) {
				case WSAEWOULDBLOCK:
					break;
				case WSAECONNRESET:
				case WSAECONNABORTED:
					close();
					break;
				case WSAENOTCONN:
				default:
					::__debugbreak();
				}
			}
			return result;
		}
		inline auto send_to(char const* const buffer, int const length, int const flag, socket_address const& socket_address) const noexcept -> int {
			auto result = ::sendto(_socket, buffer, length, flag, &socket_address.data(), socket_address.size());
			if (SOCKET_ERROR == result) {
				switch (::WSAGetLastError()) {
				default:
					::__debugbreak();
#pragma warning(suppress: 4065)
				}
			}
			return result;
		}
		inline auto send(WSABUF* buffer, unsigned long count, unsigned long* byte, unsigned long flag) noexcept -> int {
			int result = ::WSASend(_socket, buffer, count, byte, flag, nullptr, nullptr);
			if (SOCKET_ERROR == result) {
				switch (::GetLastError()) {
				case WSAECONNRESET:
				case WSAECONNABORTED:
					close();
					break;
				case WSAENOTSOCK:
				default:
					::__debugbreak();
				}
			}
			return result;
		}
		inline auto send(WSABUF* buffer, unsigned long count, unsigned long* byte, unsigned long flag, overlap& overlap) noexcept -> result {
			overlap.clear();
			if (SOCKET_ERROR == ::WSASend(_socket, buffer, count, byte, flag, &overlap.data(), nullptr)) {
				switch (::GetLastError()) {
				case WSA_IO_PENDING:
					return result::pending;
				case WSAECONNRESET:
				case WSAECONNABORTED:
				case WSAEINTR:
					return result::close;
				case WSAEINVAL:
				case WSAENOTSOCK:
				default:
					::__debugbreak();
				}
			}
			return result::complet;
		}
		inline auto send_to(WSABUF* buffer, unsigned long count, unsigned long* byte, unsigned long flag, socket_address const& address, overlap& overlap) noexcept {
			overlap.clear();
			if (SOCKET_ERROR == ::WSASendTo(_socket, buffer, count, byte, flag, &address.data(), address.size(), &overlap.data(), nullptr)) {
				switch (::GetLastError()) {
				case WSA_IO_PENDING:
					return result::pending;
				case WSAECONNRESET:
				case WSAECONNABORTED:
				case WSAEINTR:
					return result::close;
				case WSAEINVAL:
				case WSAENOTSOCK:
				default:
					::__debugbreak();
				}
			}
			return result::complet;
		}
		inline void cancel_io(void) const noexcept {
			if (FALSE == ::CancelIo(reinterpret_cast<HANDLE>(_socket))) {
				switch (::WSAGetLastError()) {
				default:
					::__debugbreak();
#pragma warning(suppress: 4065)
				}
			}
		}
		inline void cancel_io_ex(void) const noexcept {
			if (FALSE == ::CancelIoEx(reinterpret_cast<HANDLE>(_socket), nullptr)) {
				switch (::WSAGetLastError()) {
				case ERROR_NOT_FOUND:
					break;
				default:
					::__debugbreak();
#pragma warning(suppress: 4065)
				}
			}
		}
		inline void cancel_io_ex(overlap& overlap) const noexcept {
			if (FALSE == ::CancelIoEx(reinterpret_cast<HANDLE>(_socket), &overlap.data())) {
				switch (::WSAGetLastError()) {
				default:
					::__debugbreak();
#pragma warning(suppress: 4065)
				}
			}
		}
		inline auto get_local_socket_address(void) const noexcept -> std::optional<socket_address_storage> {
			socket_address_storage address;
			int length = address.size();
			if (SOCKET_ERROR == ::getsockname(_socket, &address.data(), &length)) {
				switch (::GetLastError()) {
				default:
					__debugbreak(); //임시로 둠 아마 넘기는 조건이 있을듯
					break;
#pragma warning(suppress: 4065)
				}
				return std::nullopt;
			}
			return address;
		}
		inline auto get_remote_socket_address(void) const noexcept -> std::optional<socket_address_storage> {
			socket_address_storage address;
			int length = address.size();
			if (SOCKET_ERROR == ::getpeername(_socket, &address.data(), &length)) {
				switch (::GetLastError()) {
				default:
					__debugbreak();
					break;
#pragma warning(suppress: 4065)
				}
				return std::nullopt;
			}
			return address;
		}
		inline bool wsa_get_overlapped_result(overlap& overlap, unsigned long* transfer, bool const wait, unsigned long* flag) noexcept {
			return ::WSAGetOverlappedResult(_socket, &overlap.data(), transfer, wait, flag);
		}
		inline void set_option_tcp_nodelay(int const enable) const noexcept {
			set_option(IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char const*>(&enable), sizeof(int));
		}
		inline void set_option_dont_fragment(unsigned long enable) const noexcept {
			set_option(IPPROTO_IP, IP_DONTFRAGMENT, reinterpret_cast<char*>(&enable), sizeof(unsigned long));
		}
		inline void set_option_linger(unsigned short const onoff, unsigned short const time) const noexcept {
			LINGER linger{ onoff , time };
			set_option(SOL_SOCKET, SO_LINGER, reinterpret_cast<char const*>(&linger), sizeof(LINGER));
		}
		inline void set_option_broadcast(unsigned long const enable) const noexcept {
			set_option(SOL_SOCKET, SO_BROADCAST, reinterpret_cast<char const*>(&enable), sizeof(unsigned long));
		}
		inline void set_option_send_buffer(int const size) const noexcept {
			set_option(SOL_SOCKET, SO_SNDBUF, reinterpret_cast<char const*>(&size), sizeof(int));
		}
		inline void set_option_receive_buffer(int const size) const noexcept {
			set_option(SOL_SOCKET, SO_RCVBUF, reinterpret_cast<char const*>(&size), sizeof(int));
		}
		inline void set_option_update_accept_context(socket& socket_) const noexcept {
			set_option(SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, reinterpret_cast<char*>(&socket_.data()), sizeof(SOCKET));
		}
		inline void set_option_update_connect_context(void) const noexcept {
			set_option(SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT, nullptr, sizeof(SOCKET));
		}

		inline void set_option(int const level, int const name, char const* value, int const length) const noexcept {
			if (SOCKET_ERROR == ::setsockopt(_socket, level, name, value, length))
				::__debugbreak();
		}
		//inline void get_option_connect_time(unsigned short const onoff, unsigned short const time) const noexcept {
		//}
		//inline void get_option(int const level, int const name, char* value, int* length) const noexcept {
		//	if (SOCKET_ERROR == getsockopt(_socket, level, name, value, length))
		//		::__debugbreak(); /*SOL_SOCKET*/
		//}
		inline void io_control_udp_connect_reset(int behavior) noexcept {
			unsigned long result;
			io_control(SIO_UDP_CONNRESET, &behavior, sizeof(int), nullptr, 0, result);
		}
		inline void io_control_nonblocking(unsigned long const enable) const noexcept {
			io_control(FIONBIO, enable);
		}
		inline void io_control(long const cmd, unsigned long arg) const noexcept {
			if (SOCKET_ERROR == ::ioctlsocket(_socket, cmd, &arg))
				::__debugbreak();
		}
		inline void io_control(unsigned long control_code, void* in_buffer, unsigned long in_buffer_size, void* out_buffer, unsigned long out_buffer_size, unsigned long& byte_return) noexcept {
			if (SOCKET_ERROR == ::WSAIoctl(_socket, control_code, in_buffer, in_buffer_size, out_buffer, out_buffer_size, &byte_return, nullptr, nullptr))
				::__debugbreak();
		}
		inline void set_file_complet_notify_mode(unsigned char flag) noexcept {
			if (0 == ::SetFileCompletionNotificationModes(reinterpret_cast<HANDLE>(_socket), flag)) {
				switch (::GetLastError()) {
				default:
					__debugbreak();
					break;
#pragma warning(suppress: 4065)
				}
			}
		}
		inline auto data(void) const noexcept -> SOCKET const& {
			return _socket;
		}
		inline auto data(void) noexcept -> SOCKET& {
			return library::cast<SOCKET&>(library::cast<socket const&>(*this).data());
		}
	};
	class socket_extend final {
		using result = socket::result;
		LPFN_ACCEPTEX _accept_ex;
		LPFN_CONNECTEX _connect_ex;
		LPFN_GETACCEPTEXSOCKADDRS _get_accept_ex_sockaddr;
		LPFN_DISCONNECTEX _disconnect_ex;
	public:
		inline auto accept(socket& listen_socket, socket& accept_socket, void* output_buffer, unsigned long address_length, unsigned long remote_address_length, overlap& overlap_) const noexcept -> result {
			assert(address_length == remote_address_length);
			assert(address_length == sizeof(sockaddr_in) + 16 || address_length == sizeof(sockaddr_in6) + 16);
			assert(remote_address_length == sizeof(sockaddr_in) + 16 || remote_address_length == sizeof(sockaddr_in6) + 16);

			overlap_.clear();
#pragma warning(suppress: 6387)
			if (FALSE == _accept_ex(listen_socket.data(), accept_socket.data(), output_buffer, 0, address_length, remote_address_length, nullptr, &overlap_.data())) {
				switch (::WSAGetLastError()) {
				case WSA_IO_PENDING:
					return result::pending;
				case WSAENOTSOCK:
					return result::close;
				case WSAECONNRESET:
				case WSAECONNABORTED:
				default:
					::__debugbreak();
				}
			}
			return result::complet;
		}
		inline auto connect(socket& connect_socket, socket_address const& socket_address, overlap& overlap) const noexcept -> result {
			overlap.clear();
#pragma warning(suppress: 6387)
			if (FALSE == _connect_ex(connect_socket.data(), &socket_address.data(), socket_address.size(), nullptr, 0, nullptr, &overlap.data())) {
				switch (::WSAGetLastError()) {
				case WSA_IO_PENDING:
					return result::pending;
				case WSAENOTSOCK:
					return result::close;
				case WSAECONNRESET:
				case WSAECONNABORTED:
				default:
					::__debugbreak();
				}
			}
			return result::complet;
		}
		inline auto get_accept_ex_socket_address(void* buffer) const noexcept -> library::pair<socket_address_storage, socket_address_storage> {
			sockaddr* local_sockaddr = nullptr;
			int local_sockaddr_length = 0;
			sockaddr* remote_sockaddr = nullptr;
			int remote_sockaddr_length = 0;
			_get_accept_ex_sockaddr(buffer, 0, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16, &local_sockaddr, &local_sockaddr_length, &remote_sockaddr, &remote_sockaddr_length);
			return library::pair<socket_address_storage, socket_address_storage>(
				library::_piecewise_construct, library::forward_as_tuple(*local_sockaddr, local_sockaddr_length),
				library::forward_as_tuple(*remote_sockaddr, remote_sockaddr_length));
		}

		inline void wsa_io_control_acccept_ex(socket& socket) noexcept {
			GUID guid = WSAID_ACCEPTEX;
			unsigned long byte_return;
			socket.io_control(SIO_GET_EXTENSION_FUNCTION_POINTER, reinterpret_cast<void*>(&guid), sizeof(GUID), reinterpret_cast<void*>(&_accept_ex), sizeof(LPFN_ACCEPTEX), byte_return);
		}
		inline void wsa_io_control_connect_ex(socket& socket) noexcept {
			GUID guid = WSAID_CONNECTEX;
			unsigned long byte_return;
			socket.io_control(SIO_GET_EXTENSION_FUNCTION_POINTER, reinterpret_cast<void*>(&guid), sizeof(GUID), reinterpret_cast<void*>(&_connect_ex), sizeof(LPFN_CONNECTEX), byte_return);
		}
		inline void wsa_io_control_disconnect_ex(socket& socket) noexcept {
			GUID guid = WSAID_DISCONNECTEX;
			unsigned long byte_return;
			socket.io_control(SIO_GET_EXTENSION_FUNCTION_POINTER, reinterpret_cast<void*>(&guid), sizeof(GUID), reinterpret_cast<void*>(&_disconnect_ex), sizeof(LPFN_DISCONNECTEX), byte_return);
		}
		inline void wsa_io_control_get_accept_ex_sockaddr(socket& socket) noexcept {
			GUID guid = WSAID_GETACCEPTEXSOCKADDRS;
			unsigned long byte_returned;
			socket.io_control(SIO_GET_EXTENSION_FUNCTION_POINTER, reinterpret_cast<void*>(&guid), sizeof(GUID), reinterpret_cast<void*>(&_get_accept_ex_sockaddr), sizeof(LPFN_GETACCEPTEXSOCKADDRS), byte_returned);
		}
	};

	class file_descript final {
		using size_type = unsigned int;
		fd_set _fd_set;
	public:
		inline file_descript(void) noexcept = default;
		inline file_descript(file_descript const&) noexcept = delete;
		inline file_descript(file_descript&&) noexcept = delete;
		inline auto operator=(file_descript const&) noexcept -> file_descript & = delete;
		inline auto operator=(file_descript&&) noexcept -> file_descript & = delete;
		inline ~file_descript(void) noexcept = default;

		inline void zero(void) noexcept {
			FD_ZERO(&_fd_set);
		}
		inline void set(socket& socket) noexcept {
			FD_SET(socket.data(), &_fd_set);
		}
		inline auto is_fd_set(socket& socket) const noexcept -> int {
			return FD_ISSET(socket.data(), &_fd_set);
		}
		inline auto data(void) noexcept -> fd_set& {
			return _fd_set;
		}
		inline auto size(void) const noexcept -> size_type {
			return _fd_set.fd_count;
		}
	};
}