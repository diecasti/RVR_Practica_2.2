#include "Chat.h"

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void ChatMessage::to_bin()
{
	alloc_data(MESSAGE_SIZE);

	memset(_data, 0, MESSAGE_SIZE);

	//Serializar los campos type, nick y message en el buffer _data
	char* tmp = _data;
	memcpy(tmp, &type, sizeof(uint8_t));
	tmp += sizeof(uint8_t);
	nick[nick.size() + 1] = '\0';
	memcpy(tmp, nick.c_str(), sizeof(char) * 8);
	tmp += sizeof(char) * 8;
	message[message.size() + 1] = '\0';
	memcpy(tmp, message.c_str(), sizeof(char) * _MAX_FNAME);
}

int ChatMessage::from_bin(char* bobj)
{
	alloc_data(MESSAGE_SIZE);
	memcpy(static_cast<void*>(_data), bobj, MESSAGE_SIZE);

	//Reconstruir la clase usando el buffer _data
	char* tmp = _data;

	memcpy(&type, tmp, sizeof(int8_t));
	tmp += sizeof(int8_t);
	nick.resize(sizeof(char) * 8);
	memcpy(&nick[0], tmp, sizeof(char) * 8);
	tmp += sizeof(char) * 8;
	message.resize(sizeof(char) * _MAX_FNAME);
	memcpy(&message[0], tmp, sizeof(char) * _MAX_FNAME);

	return 0;
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void ChatServer::do_messages()
{
	while (true)
	{
		/*
		 * NOTA: los clientes están definidos con "smart pointers", es necesario
		 * crear un unique_ptr con el objeto socket recibido y usar std::move
		 * para añadirlo al vector
		 */

		 //Recibir Mensajes en y en función del tipo de mensaje
		 // - LOGIN: Añadir al vector clients
		 // - LOGOUT: Eliminar del vector clients
		 // - MESSAGE: Reenviar el mensaje a todos los clientes (menos el emisor)
		Socket* client;
		ChatMessage msg;
		socket.recv(msg, client);
		while (true)
		{

			int count = 0;

			switch (msg.type)		// LOGIN = 0, MESSAGE = 1, LOGOUT = 2
			{
			case ChatMessage::LOGIN:
				clients.push_back(client);
				std::cout << msg.nick.c_str() << " logeado " << std::endl;
				break;
			case  ChatMessage::LOGOUT:
				for (Socket* sock : clients)
				{
					if ((*sock == *client))
					{
						clients.erase(clients.begin() + count);
						break;
					}
					count++;
				}
				std::cout << msg.nick.c_str() << " desconectado" << std::endl;

				break;
			case ChatMessage::MESSAGE:
				for (Socket* sock : clients)
				{
					if (!(*sock == *client))
						socket.send(msg, *sock);
				}
				std::cout << msg.nick.c_str() << " mensaje enviado" << std::endl;
				break;
			default:
				break;
			}
			std::cout << "Conectado: " << clients.size() << std::endl;
		}
	}
}

// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------

void ChatClient::login()
{
	std::string msg;

	ChatMessage em(nick, msg);
	em.type = ChatMessage::LOGIN;

	socket.send(em, socket);
}

void ChatClient::logout()
{
	std::string msg;

	ChatMessage em(nick, msg);
	em.type = ChatMessage::LOGOUT;

	socket.send(em, socket);
}

void ChatClient::input_thread()
{
	while (true)
	{
		// Leer stdin con std::getline
		std::string msg;
		std::getline(std::cin, msg);
		
		//mensaje para desconectarse del servidor
		if (msg == "LOGOUT")
		{
			logout();
			break;
		}
		else
		{
			// Enviar al servidor usando socket
			ChatMessage em(nick, msg);
			em.type = ChatMessage::MESSAGE;
			socket.send(em, socket);
		}
	}
}

void ChatClient::net_thread()
{
	while (true)
	{
		//Recibir Mensajes de red
		//Mostrar en pantalla el mensaje de la forma "nick: mensaje"
		ChatMessage em;

		socket.recv(em);

		if (em.nick != nick)
			std::cout << em.nick << " : " << em.message << std::endl;
	}
}

