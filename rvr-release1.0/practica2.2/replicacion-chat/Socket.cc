#include <string.h>

#include "Serializable.h"
#include "Socket.h"

Socket::Socket(const char* address, const char* port) :sd(-1)
{
	//Construir un socket de tipo AF_INET y SOCK_DGRAM usando getaddrinfo.
	struct addrinfo hints;
	struct addrinfo* res;

	memset((void*)&hints, 0, sizeof(hints));

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;

	int rc = getaddrinfo(address, port, &hints, &res);
	if (rc < 0)
		std::cout << "Error de conexion" << std::endl;

	//Con el resultado inicializar los miembros sd, sa y sa_len de la clase
	sa = res->ai_addr;
	sa_len = res->ai_addrlen;

	sd = socket(res->ai_family, res->ai_socktype, 0);
	if (sd < 0)
		std::cout << "Error en la creacion del socket" << std::endl;
}

int Socket::recv(Serializable& obj, Socket*& sock)
{
	struct sockaddr sa;
	socklen_t sa_len = sizeof(struct sockaddr);

	char buffer[MAX_MESSAGE_SIZE];

	ssize_t bytes = ::recvfrom(sd, buffer, MAX_MESSAGE_SIZE, 0, &sa, &sa_len);

	if (bytes <= 0)
	{
		return -1;
	}

	if (sock != 0)
	{
		sock = new Socket(&sa, sa_len);
	}

	obj.from_bin(buffer);

	return 0;
}

int Socket::send(Serializable& obj, const Socket& sock)
{
	//Serializar el objeto
	obj.to_bin();

	//Enviar el objeto binario a sock usando el socket sd
	int data = sendto(sock.sd, (void*)obj.data(), obj.size(), 0, (struct sockaddr*)&sock.sa, sock.sa_len);
	return data;
}

bool operator== (const Socket& s1, const Socket& s2)
{
	//Comparar los campos sin_family, sin_addr.s_addr y sin_port
	//de la estructura sockaddr_in de los Sockets s1 y s2
	//Retornar false si alguno difiere
	if (s1.sa.sa_family == AF_INET && s2.sa.sa_family == AF_INET)
	{
		struct sockaddr_in* sock1 = (struct sockaddr_in*)s1.sa.sa_data;
		struct sockaddr_in* sock2 = (struct sockaddr_in*)s2.sa.sa_data;

		return(sock1->sin_family == sock2->sin_family &&
			sock1->sin_port == sock2->sin_port &&
			sock1->sin_addr.s_addr == sock2->sin_addr.s_addr);
	}
};

std::ostream& operator<<(std::ostream& os, const Socket& s)
{
	char host[NI_MAXHOST];
	char serv[NI_MAXSERV];

	getnameinfo((struct sockaddr*)&(s.sa), s.sa_len, host, NI_MAXHOST, serv,
		NI_MAXSERV, NI_NUMERICHOST);

	os << host << ":" << serv;

	return os;
};

