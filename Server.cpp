#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>

using namespace std;

    //Объявление обработчика сигнала
	volatile sig_atomic_t wasSigHup = 0;
	void sigHupHandler(int r)
	{
    	wasSigHup = 1;
	}

int main()
{	
	int sock=0;
	int listener;

	struct sockaddr_in addr;
	int addrLen = sizeof(addr);
	listener=socket(AF_INET, SOCK_STREAM, 0);
	if (listener<0)
	{
    	printf("socket error");
    	exit(1);
	}


	addr.sin_family = AF_INET;
	addr.sin_port = htons(8080);
	addr.sin_addr.s_addr = INADDR_ANY;


	 //явное связывание сокета с адресом
	if(bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0)
	{
    	printf("bind error");
    	exit(2);
	}

	listen(listener, 1);
	
	//блокировка сигнала
	sigset_t blockedMask, origMask;  //маски блокировки и исходная
	sigemptyset(&blockedMask);// пустое мн-во сигналов
	sigemptyset(&origMask);
	sigaddset(&blockedMask, SIGHUP);  // добавляем sighup в мн-во
	sigprocmask(SIG_BLOCK, &blockedMask, &origMask);  //рег. маску блока сигналов

	
	//Регистрация обработчика сигнала
	struct sigaction sa;
	sigaction(SIGHUP, NULL, &sa);// тип сигнала, новый метод обработки, старый метод
	sa.sa_handler = sigHupHandler;// обработка сигнала
	sa.sa_flags |= SA_RESTART;
	sigaction(SIGHUP, &sa, NULL);

	


	// работа основного цикла
	int maxFd;
	fd_set fds;
	while (1)
	{
    	 //мн-во файловых дескрипторов
    	FD_ZERO(&fds); //опустошаем
    	FD_SET(listener, &fds); //регистрируем фд
    	if (sock > 0)
        	FD_SET(sock, &fds); // регистрирум фд входящего
    	if (sock > listener)
        	maxFd = sock;
	else 
		maxFd = listener;
    	if (pselect(maxFd + 1, &fds, NULL, NULL, NULL, &origMask) == -1) //происходит разблокировка сигнала в функции pselect
    	{
        	if (errno == EINTR) //если прерван сигналом, принимаем его
        	{
            		if (wasSigHup == 1) //сигнал приходил
            		{
                		wasSigHup = 0;
                		printf("SIGHUP received.\n");
				continue;
            		}
        	}
        	else
        	{
            		printf("pselect error");
            		exit(3);
        	}
    	}

    	
    	if (sock > 0 && FD_ISSET(sock, &fds))
    	{
        	char buffer[1024] = { 0 };
        	int bytesRead = read(sock, buffer, 1024);
        	if (bytesRead > 0)
            		printf("Received data: %d bytes\n", bytesRead);// Обработка принятых данных (пример: вывод на экран)
        	else 
			if (bytesRead == 0)
        		{
            			// Соединение закрыто клиентом
            			printf("Client disconnected.\n");
            			close(sock);
				sock = 0;
        		}
        		else
        		{
            			// Ошибка при чтении данных
            			printf("read error");
            			//close(sock);
            			//exit(4);
        		}
		continue;
    	}
	
	if (FD_ISSET(listener, &fds))   // если не остался listener
    	{
        	sock = accept(listener, (struct sockaddr*)&addr, (socklen_t*)&addrLen);
        	if (sock < 0)
        	{
            		printf("accept error");
            		exit(5);
        	}
		printf("New connection.\n");
    	}
	}
	close(listener);
	return 0;
}

