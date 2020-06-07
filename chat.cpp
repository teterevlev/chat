#include <iostream>
#include <thread>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <unistd.h>
#include <sys/un.h>
#include <fcntl.h>
#include <algorithm>
#include <set>
#include <string>
#include <arpa/inet.h>
using namespace std;//188.165.150.133:3426

class ChatServer{
    int listener;
    struct sockaddr_in addr;
    char buf[1024];
    int bytes_read;
    set<int> clients;
    timeval timeout;
    fd_set readset;
  public:
    ChatServer(int port, int timeout=15){
        setup(port);
        
        
        this->timeout.tv_sec = timeout;
        clients.clear();
        FD_ZERO(&readset);
        FD_SET(listener, &readset);

        loop();
    }
    inline void setup(int port){
        listener = socket(AF_INET, SOCK_STREAM, 0);
        if(listener < 0){
            perror("socket");
            exit(1);
        }
        
        fcntl(listener, F_SETFL, O_NONBLOCK);
        
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port);
        addr.sin_addr.s_addr = INADDR_ANY;
        
        if(bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0){
            perror("bind");
            exit(2);
        }
        listen(listener, 2);
    }
    inline void loop(){
        while(1){

            for(set<int>::iterator it = clients.begin(); it != clients.end(); it++){
                FD_SET(*it, &readset);
            }
            
            //waitForEvent();
            int mx = max(listener, *max_element(clients.begin(), clients.end()));
            if(select(mx+1, &readset, NULL, NULL, &timeout) < 0){
                perror("select err");
                exit(3);
            }
            if(FD_ISSET(listener, &readset)){
                // Поступил новый запрос на соединение, используем accept
                int sock = accept(listener, NULL, NULL);
                if(sock < 0){
                    perror("accept");
                    exit(3);
                }
                
                fcntl(sock, F_SETFL, O_NONBLOCK);

                clients.insert(sock);
            }
            processEvents();
            
        }
    }
    inline void waitForEvent(){
        /*
        int mx = max(listener, *max_element(clients.begin(), clients.end()));
        if(select(mx+1, &readset, NULL, NULL, &timeout) < 0){
            perror("select err");
            exit(3);
        }
        */
    }
    inline void acceptEvent(){
        /*
        if(FD_ISSET(listener, &readset)){
            // Поступил новый запрос на соединение, используем accept
            int sock = accept(listener, NULL, NULL);
            if(sock < 0){
                perror("accept");
                exit(3);
            }
            
            fcntl(sock, F_SETFL, O_NONBLOCK);

            clients.insert(sock);
        }
        */
    }
    inline void processEvents(){
        for(set<int>::iterator it = clients.begin(); it != clients.end(); it++){
            if(FD_ISSET(*it, &readset)){
                // Поступили данные от клиента, читаем их
                bytes_read = recv(*it, buf, 1024, 0);

                if(bytes_read <= 0)
                {
                    // Соединение разорвано, удаляем сокет из множества
                    close(*it);
                    clients.erase(*it);
                    continue;
                }

                cout << buf << endl;
                // Отправляем данные обратно клиенту
                send(*it, buf, bytes_read, 0);
            }
        }
    }
};  

void server(){
    int listener;
    struct sockaddr_in addr;
    char buf[1024];
    int bytes_read;

    listener = socket(AF_INET, SOCK_STREAM, 0);
    if(listener < 0)
    {
        perror("socket");
        exit(1);
    }
    
    fcntl(listener, F_SETFL, O_NONBLOCK);
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(3427);
    addr.sin_addr.s_addr = INADDR_ANY;
    cout << INADDR_ANY << endl;
    //inet_pton(AF_INET, "188.165.150.133", &(addr.sin_addr));
    if(bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("bind");
        exit(2);
    }

    listen(listener, 2);
    
    set<int> clients;
    clients.clear();

    while(1)
    {
        // Заполняем множество сокетов
        fd_set readset;
        FD_ZERO(&readset);
        FD_SET(listener, &readset);

        for(set<int>::iterator it = clients.begin(); it != clients.end(); it++)
            FD_SET(*it, &readset);

        // Задаём таймаут
        timeval timeout;
        timeout.tv_sec = 15;
        timeout.tv_usec = 0;

        // Ждём события в одном из сокетов
        int mx = max(listener, *max_element(clients.begin(), clients.end()));
        if(select(mx+1, &readset, NULL, NULL, &timeout) < 0)
        {
            perror("select err");
            exit(3);
        }
        
        // Определяем тип события и выполняем соответствующие действия
        if(FD_ISSET(listener, &readset))
        {
            // Поступил новый запрос на соединение, используем accept
            int sock = accept(listener, NULL, NULL);
            if(sock < 0)
            {
                perror("accept");
                exit(3);
            }
            
            fcntl(sock, F_SETFL, O_NONBLOCK);

            clients.insert(sock);
        }

        for(set<int>::iterator it = clients.begin(); it != clients.end(); it++)
        {
            if(FD_ISSET(*it, &readset))
            {
                // Поступили данные от клиента, читаем их
                bytes_read = recv(*it, buf, 1024, 0);

                if(bytes_read <= 0)
                {
                    // Соединение разорвано, удаляем сокет из множества
                    close(*it);
                    clients.erase(*it);
                    continue;
                }

                cout << buf << endl;
                // Отправляем данные обратно клиенту
                send(*it, buf, bytes_read, 0);
            }
        }
    }


}
void schedule(){
    while(1);
}
int getPort(int argc, char* argv[]){
    int port = 3427;
    if (argc > 1){
        try{
            port = stoi(argv[1]);
        }catch(...){}
    }
    return port;
}
int main (int argc, char* argv[]){
    /*
    thread scheduleTh(schedule);
    thread serverTh(server);
    scheduleTh.join();
    serverTh.join();
    */
    auto port = getPort(argc, argv);
    cout << port <<endl;
    //ChatServer chat(port);
    server();
    return 0;
}
