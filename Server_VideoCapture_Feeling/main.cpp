#include <QCoreApplication>
#include <QDebug>
#include "itcpserver.h"

/**/
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

//    std::cout << "argc : " << argc << std::endl;
//    std::cout << "argv : " << argv[0] << std::endl;
//    std::cout << "argv : " << argv[1] << std::endl;
//    std::cout << "argv : " << argv[2] << std::endl;

    ITcpServer server;

    if(argc == 1)
    {
        server.listen();
    }
    else if(argc == 3)
    {
        std::cout << "ip : " << argv[1] << std::endl;
        std::cout << "port : " << argv[2] << std::endl;

        server.listen(QHostAddress(argv[1]),atoi(argv[2]));
    }

    return a.exec();
}
