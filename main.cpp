#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQml>
#include <QtQuick>
#include "networkmanager.h"

int main(int argc, char *argv[])
{
	QGuiApplication app(argc, argv);
	
	QQmlApplicationEngine engine;
	qmlRegisterType<NetworkManager>("com.Kanari.NetworkManager", 1, 0, "NetworkManager");
	qmlRegisterType<NetworkHostList>("com.Kanari.NetworkManager", 1, 0, "NetworkHostList");
	
	engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
	
	return app.exec();
}

