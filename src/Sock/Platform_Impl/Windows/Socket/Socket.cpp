#include "Sock/Socket/Socket.h"

#include <WinSock2.h>
#include <stdexcept>

#define THROW_IF(expr, str) do { if (expr) throw std::runtime_error(std::string(str) + " Reason: " + getLastWSAErrorDesc()); } while(0)

namespace
{
	int afSockToWin(Sock::AF af)
	{
		switch (af) {
		case Sock::AF::IPv4: return AF_INET;
		}

		throw std::logic_error("Invalid Sock::AF value!");
	}

	Sock::AF afWinToSock(int af)
	{
		switch (af) {
		case AF_INET: return Sock::AF::IPv4;
		}

		throw std::logic_error("Invalid Win AF value!");
	}

	int typeSockToWin(Sock::Type type)
	{
		switch (type) {
		case Sock::Type::STREAM: return SOCK_STREAM;
		case Sock::Type::DGRAM:  return SOCK_DGRAM;
		}

		throw std::logic_error("Invalid Sock::Type value!");
	}

	Sock::Type typeWinToSock(int type)
	{
		switch (type) {
		case SOCK_STREAM: return Sock::Type::STREAM;
		case SOCK_DGRAM:  return Sock::Type::DGRAM;
		}

		throw std::logic_error("Invalid Win Type value!");
	}

	int optLvlSockToWin(Sock::OptLevel lvl)
	{
		switch (lvl) {
		case Sock::OptLevel::SOCKET: return SOL_SOCKET;
		}

		throw std::logic_error("Invalid Sock::OptLevel value!");
	}

	int optNameSockToWin(Sock::OptName lvl)
	{
		switch (lvl) {
		case Sock::OptName::BROADCAST: return SO_BROADCAST;
		}

		throw std::logic_error("Invalid Sock::OptName value!");
	}

	const char* getLastWSAErrorDesc()
	{
		switch (WSAGetLastError()) {
		case WSANOTINITIALISED: return "Uninitialized Socket Context.";
		case WSAENETDOWN:       return "The network subsystem or the associated service provider has failed.";
		case WSAEAFNOSUPPORT:   return "The specified address family is not supported.";
		case WSAEINVAL:         return "Invalid argument supplied.";
		}

		return "!No description yet!";
	}

	sockaddr_in sockAddrSockToWin(const Sock::Addr& addr)
	{
		sockaddr_in sockaddr;
		sockaddr.sin_family = afSockToWin(addr.af);
		sockaddr.sin_addr.S_un.S_addr = inet_addr(addr.ip.c_str());
		sockaddr.sin_port = htons(addr.port);

		return sockaddr;
	}

	Sock::Addr sockAddrWinToSock(const sockaddr_in& addr)
	{
		return {
			.af = afWinToSock(addr.sin_family),
			.ip = inet_ntoa(addr.sin_addr),
			.port = ntohs(addr.sin_port)
		};
	}

	class WSA
	{
	public:
		WSA()  { (void)WSAStartup(MAKEWORD(2, 2), &m_wsa); }
		~WSA() { (void)WSACleanup(); }

	private:
		WSADATA m_wsa;
	};

	std::unique_ptr<WSA> sp_wsa;
}

namespace Sock
{
	struct Socket::SocketHandle
	{
		SOCKET handle = INVALID_SOCKET;
	};

	Socket::Socket(Sock::AF af, Sock::Type type)
		: Socket({})
	{
		THROW_IF((m_pimpl->handle = socket(afSockToWin(af), typeSockToWin(type), 0)) == INVALID_SOCKET, "Failed to create socket!");
	}

	Socket::Socket(const SocketHandle& sh)
		: m_pimpl(std::make_unique<SocketHandle>(sh))
	{
		if (!sp_wsa) sp_wsa = std::make_unique<WSA>();
	}

	Socket::~Socket()
	{
		if (m_pimpl) closesocket(m_pimpl->handle);
	}

	Socket::Socket(Socket&&)            noexcept = default;
	Socket& Socket::operator=(Socket&&) noexcept = default;

	void Socket::bind(const Addr& addr) const
	{
		sockaddr_in sockaddr = sockAddrSockToWin(addr);

		int bind = ::bind(m_pimpl->handle, reinterpret_cast<SOCKADDR*>(&sockaddr), sizeof(sockaddr));

		THROW_IF(bind == SOCKET_ERROR, "Failed to bind socket!");
	}

	void Socket::set_opt_impl(OptLevel lvl, OptName name, const void* val, size_t valLen) const
	{
		int setopt = setsockopt(m_pimpl->handle, optLvlSockToWin(lvl), optNameSockToWin(name), reinterpret_cast<const char*>(val), valLen);

		THROW_IF(setopt == SOCKET_ERROR, "Failed to set option!");
	}

	void ClientSocket::connect(const Addr& addr) const
	{
		sockaddr_in sockaddr = sockAddrSockToWin(addr);

		int connect = ::connect(m_pimpl->handle, reinterpret_cast<SOCKADDR*>(&sockaddr), sizeof(sockaddr));

		THROW_IF(connect == SOCKET_ERROR, "Couldn't connect!");
	}

	size_t ClientSocket::send(const void* buffer, size_t size) const
	{
		int send = ::send(m_pimpl->handle, reinterpret_cast<const char*>(buffer), size, 0);

		THROW_IF(send == SOCKET_ERROR, "Failed to send!");

		return send;
	}

	size_t ClientSocket::recv(void* buffer, size_t size) const
	{
		int recv = ::recv(m_pimpl->handle, reinterpret_cast<char*>(buffer), size, 0);

		THROW_IF(recv == SOCKET_ERROR, "Failed to receive!");

		return recv;
	}

	size_t DatagramIPv4Socket::send_to(const void* buffer, size_t size, const Addr& to) const
	{
		sockaddr_in sockaddr = sockAddrSockToWin(to);

		int sendto = ::sendto(m_pimpl->handle, reinterpret_cast<const char*>(buffer), size, 0, reinterpret_cast<SOCKADDR*>(&sockaddr), sizeof(sockaddr));

		THROW_IF(sendto == SOCKET_ERROR, "Failed to send!");

		return sendto;
	}

	size_t DatagramIPv4Socket::recv_from(void* buffer, size_t size, Addr& from) const
	{
		sockaddr_in sockaddr;
		int sizeSock = sizeof(sockaddr);

		int recvfrom = ::recvfrom(m_pimpl->handle, reinterpret_cast<char*>(buffer), size, 0, reinterpret_cast<SOCKADDR*>(&sockaddr), &sizeSock);

		THROW_IF(recvfrom == SOCKET_ERROR, "Failed to receive!");

		from = sockAddrWinToSock(sockaddr);

		return recvfrom;
	}

	void ServerSocket::listen(int backlog) const
	{
		int listen = ::listen(m_pimpl->handle, backlog);

		THROW_IF(listen == SOCKET_ERROR, "Failed to listen!");
	}

	ClientSocket ServerSocket::accept() const
	{
		SOCKET accept = ::accept(m_pimpl->handle, nullptr, nullptr);

		THROW_IF(accept == INVALID_SOCKET, "Failed to accept socket!");

		return ClientSocket({ accept });
	}
}