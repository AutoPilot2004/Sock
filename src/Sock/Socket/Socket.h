#pragma once

#include <string>
#include <memory>

namespace Sock
{
	enum class AF
	{
		IPv4
	};

	enum class Type
	{
		STREAM,
		DGRAM
	};

	enum class OptLevel
	{
		SOCKET
	};

	enum class OptName
	{
		BROADCAST
	};

	struct Addr
	{
		AF          af;
		std::string ip;
		uint16_t    port;
	};

	struct MacAddr
	{
		static constexpr uint8_t BYTE_SIZE = 6;

		uint8_t byte[BYTE_SIZE];
	};

	class Socket
	{
	public:
		virtual ~Socket();

	public:
		void bind(const Addr& addr) const;

		template<typename T>
		void set_opt(OptLevel lvl, OptName name, const T& val) const
		{
			set_opt_impl(lvl, name, &val, sizeof(val));
		}

	protected:
		struct SocketHandle;

		Socket(AF af, Type type);
		Socket(const SocketHandle& sh);

		Socket(Socket&&)            noexcept;
		Socket& operator=(Socket&&) noexcept;
		
	protected:
		std::unique_ptr<SocketHandle> m_pimpl;

	private:
		void set_opt_impl(OptLevel lvl, OptName name, const void* val, size_t valLen) const;
	};

	class ClientSocket : public Socket
	{
		using Socket::Socket;

	public:
		ClientSocket(AF af, Type type) : Socket(af, type) {}

	public:
		void connect(const Addr& addr) const;

		size_t send(const void* buffer, size_t size) const;
		size_t recv(void* buffer, size_t size)       const;

	private:
		friend class ServerSocket;
	};

	class ServerSocket : public Socket
	{
	public:
		ServerSocket(AF af, Type type) : Socket(af, type) {}

	public:
		void listen(int backlog) const;

		[[nodiscard]] ClientSocket accept() const;
	};

	class StreamIPv4ClientSocket final : public ClientSocket
	{
	public:
		StreamIPv4ClientSocket() : ClientSocket(AF::IPv4, Type::STREAM) {}
	};

	class DatagramIPv4Socket final : public ClientSocket
	{
	public:
		DatagramIPv4Socket() : ClientSocket(AF::IPv4, Type::DGRAM) {}

		size_t send_to(const void* buffer, size_t size, const Addr& to) const;
		size_t recv_from(void* buffer, size_t size, Addr& from)         const;
	};

	class StreamIPv4ServerSocket final : public ServerSocket
	{
	public:
		StreamIPv4ServerSocket() : ServerSocket(AF::IPv4, Type::STREAM) {}
	};
}