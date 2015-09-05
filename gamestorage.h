#ifndef GameStorage_H
#define GameStorage_H

#include <QObject>
#include <QString>
#include <QSettings>
#include "udpbroadcastmanager.h"
#include "defineproperty.h"

class GameStorage : public QObject {
	Q_OBJECT
public:
	explicit GameStorage(QObject *parent = 0);
	
signals: DEFINE_PROPERTY(QString, P, p, rofileName)
signals: DEFINE_PROPERTY(QString, A, a, vatarId)
signals: DEFINE_PROPERTY(bool, I, i, sHost)
signals: DEFINE_PROPERTY(QString, O, o, pponentIP)
signals: DEFINE_PROPERTY(int, M, m, yself)
signals: DEFINE_PROPERTY(QString, P, p, layer1Name)
signals: DEFINE_PROPERTY(QString, P, p, layer2Name)
signals: DEFINE_PROPERTY(QString, P, p, layer1AvatarId)
signals: DEFINE_PROPERTY(QString, P, p, layer2AvatarId)
};

#endif // GameStorage_H
