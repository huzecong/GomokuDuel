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

#include <ctime>

class GameStorage : public QObject {
	Q_OBJECT
public:
	explicit GameStorage(QObject *parent = 0);
	
    Q_PROPERTY(QString profileName READ profileName WRITE setProfileName NOTIFY profileNameChanged)
    const QString &profileName() const {
        return this->m_profileName;
    }
    void setProfileName(const QString &profileName) {
        if (this->m_profileName != profileName) {
            this->m_profileName = profileName;
            QSettings settings;
            settings.setValue("profileName", profileName);
            emit profileNameChanged(profileName);
        }
    }
    Q_PROPERTY(QString avatarId READ avatarId WRITE setAvatarId NOTIFY avatarIdChanged)
    const QString &avatarId() const {
        return this->m_avatarId;
    }
    void setAvatarId(const QString &avatarId) {
        if (this->m_avatarId != avatarId) {
            this->m_avatarId = avatarId;
            QSettings settings;
            settings.setValue("avatarId", avatarId);
            emit avatarIdChanged(avatarId);
        }
    }
    Q_PROPERTY(int uniqueId READ uniqueId)
    int uniqueId() const {
        return this->m_uniqueId;
    }
    
signals:
    void profileNameChanged(const QString &profileName);
    void avatarIdChanged(const QString &avatarId);
    
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

private:
    QString m_profileName, m_avatarId;
    int m_uniqueId;
};

#endif // GameStorage_H
