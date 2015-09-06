#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQml>
#include <QtQuick>
#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>
#include "gamestorage.h"
#include "udpbroadcastmanager.h"
#include "gamelogic.h"
#include "tcpnetworkmanager.h"

static QObject *gameStorageProvider(QQmlEngine *engine, QJSEngine *scriptEngine) {
	Q_UNUSED(engine)
	Q_UNUSED(scriptEngine)
	
	GameStorage *controller = new GameStorage();
	return controller;
}

int main(int argc, char *argv[]) {
	QGuiApplication app(argc, argv);
	
	QCoreApplication::setOrganizationName("Kanari");
	QCoreApplication::setApplicationName("GomokuDuel");
	
	QDir::root().mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
	
	QQmlApplicationEngine engine;
	
	qmlRegisterSingletonType<GameStorage>("com.Kanari.GomokuDuel", 1, 0,
	                                         "GameStorage", gameStorageProvider);
	qmlRegisterType<UdpBroadcastManager>("com.Kanari.GomokuDuel", 1, 0, "BroadcastManager");
	qmlRegisterType<UdpBroadcastHostList>("com.Kanari.GomokuDuel", 1, 0, "BroadcastHostList");
	qmlRegisterType<GameLogic>("com.Kanari.GomokuDuel", 1, 0, "GameLogic");
	qmlRegisterType<TcpNetworkManager>("com.Kanari.GomokuDuel", 1, 0, "NetworkManager");
	
	engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
	
	return app.exec();
}

