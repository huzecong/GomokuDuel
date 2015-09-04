#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQml>
#include <QtQuick>
#include "gamecontroller.h"
#include "networkmanager.h"

static QObject *gameControllerProvider(QQmlEngine *engine, QJSEngine *scriptEngine) {
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)

    GameController *controller = new GameController();
    return controller;
}

int main(int argc, char *argv[])
{
	QGuiApplication app(argc, argv);
	
	QQmlApplicationEngine engine;
	
	qmlRegisterSingletonType<GameController>("com.Kanari.GomokuDuel", 1, 0,
	                                         "GameController", gameControllerProvider);
	qmlRegisterType<NetworkHostData>("com.Kanari.GomokuDuel", 1, 0, "NetworkHostData");
	qmlRegisterUncreatableType<NetworkManager>("com.Kanari.GomokuDuel", 1, 0,
	                                           "NetworkManager", "use manager in GameController");
	qmlRegisterType<NetworkHostList>("com.Kanari.GomokuDuel", 1, 0, "NetworkHostList");
	
	engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
	
	return app.exec();
}

