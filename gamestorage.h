#ifndef GameStorage_H
#define GameStorage_H

#include <QObject>
#include <QString>
#include <QSettings>
#include <QDateTime>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QUrl>
#include "udpbroadcastmanager.h"
#include "defineproperty.h"

class GameStorage : public QObject {
	Q_OBJECT
public:
	explicit GameStorage(QObject *parent = 0);
	
signals: DEFINE_PROPERTY(QString, P, p, rofileName)
signals: DEFINE_PROPERTY(QString, A, a, vatarId)
signals: DEFINE_PROPERTY(int, U, u, niqueId)
signals: DEFINE_PROPERTY(bool, I, i, sHost)
signals: DEFINE_PROPERTY(QString, O, o, pponentIP)
signals: DEFINE_PROPERTY(int, O, o, pponentUniqueId)
signals: DEFINE_PROPERTY(int, M, m, yself)
signals: DEFINE_PROPERTY(QString, P, p, layer1Name)
signals: DEFINE_PROPERTY(QString, P, p, layer2Name)
signals: DEFINE_PROPERTY(QString, P, p, layer1AvatarId)
signals: DEFINE_PROPERTY(QString, P, p, layer2AvatarId)
	  
	Q_PROPERTY(QUrl fileDirectory READ fileDirectory)
	QUrl fileDirectory() const {
		return getFileDirectory();
	}

public:
	Q_INVOKABLE int rounds(int uniqueId);
	Q_INVOKABLE void setRounds(int uniqueId, int rounds);
	Q_INVOKABLE void increaseRounds();
	Q_INVOKABLE void decreaseRounds();
	Q_INVOKABLE int wins(int uniqueId);
	Q_INVOKABLE double winningRate(int uniqueId);
	Q_INVOKABLE void setWins(int uniqueId, int wins);
	Q_INVOKABLE void increaseWins();
	Q_INVOKABLE QString getFileName() const;
	Q_INVOKABLE QUrl getFileDirectory() const;
	Q_INVOKABLE QString getNameFromDir(QString dir) const;
};

#endif // GameStorage_H
