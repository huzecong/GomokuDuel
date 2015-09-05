#ifndef UDPBROADCASTMANAGER_H
#define UDPBROADCASTMANAGER_H

#include <QObject>
#include <QAbstractListModel>
#include <QByteArray>
#include <QVariant>
#include <QList>
#include <QHash>
#include <QString>
#include <QPixmap>
#include <QUdpSocket>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QHostAddress>
#include <QDataStream>
#include <QNetworkInterface>
#include <QList>
#include <QDateTime>
#include "defineproperty.h"

#include <cassert>
#include <cmath>

struct UdpBroadcastHostData {
	UdpBroadcastHostData() {}
	UdpBroadcastHostData(const QString &_name, const QString &_ip,
	                const QString &_avatarId, const QDateTime &_dateTime)
	    : name(_name), ip(_ip), avatarId(_avatarId), dateTime(_dateTime) {}
		
	inline bool operator == (const UdpBroadcastHostData &data) const {
		if (this->name != data.name) return false;
		if (this->ip != data.ip) return false;
		return true;
	}
	
	QString name, ip;
	QString avatarId;	// used with custom image provider
	QDateTime dateTime;
};



class UdpBroadcastHostList : public QAbstractListModel {
	Q_OBJECT
public:
	enum RoleNames {
		NameRole = Qt::UserRole,
		AvatarRole = Qt::UserRole + 2,
		IPRole = Qt::UserRole + 3
	};
	explicit UdpBroadcastHostList(QObject *parent = 0);
	
	friend class UdpBroadcastManager;
	
	Q_PROPERTY(int count READ count NOTIFY countChanged)
	int count() const {
		return this->m_data.count();
	}
	
signals:
	void countChanged(int count);
	
public:
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
	
	void insert(int index, const UdpBroadcastHostData &data);
	void append(const UdpBroadcastHostData &data);
	void remove(int index);
	void clear();
	UdpBroadcastHostData &get(int index) {
		return const_cast<UdpBroadcastHostData &>(this->m_data.at(index));
	}
	const UdpBroadcastHostData &get(int index) const {
		return this->m_data.at(index);
	}
	
protected:
    virtual QHash<int, QByteArray> roleNames() const override;
	
private:
    QList<UdpBroadcastHostData> m_data;
    QHash<int, QByteArray> m_roleNames;
	
	void add(const UdpBroadcastHostData &data);
	void remove(const UdpBroadcastHostData &data);
};



class UdpBroadcastManager : public QObject {
	Q_OBJECT
public:
	explicit UdpBroadcastManager(QObject *parent = 0);
	
	Q_PROPERTY(QString hostAddress READ hostAddress WRITE setHostAddress NOTIFY hostAddressChanged)
	QString hostAddress() const {
		return this->ip.toString();
	}
	void setHostAddress(const QString &addr) {
		QHostAddress ip(addr);
		if (ip != this->ip) {
			this->ip = ip;
			emit hostAddressChanged(ip.toString());
		}
	}

	Q_PROPERTY(UdpBroadcastHostList *hostList READ hostList)
	UdpBroadcastHostList *hostList() const {
		return this->m_hostList;
	}

signals: DEFINE_PROPERTY(bool, I, i, sHost)
signals: DEFINE_PROPERTY(QString, P, p, rofileName)
signals: DEFINE_PROPERTY(QString, A, a, vatarId)
	
signals:
	void hostAddressChanged(QString hostAddress);
	
	void hostTimeout(QString name, QString ip);
	void newOpponent(QString name, QString ip, QString avatarId);
	void invalidHostIP(QString ip);
	void requestRefused(QString ip);
	void requestAccepted(QString ip);
	void connectionConfirmed(QString ip);
	
public:
	Q_INVOKABLE void startHost();
	Q_INVOKABLE void abortHost();
	void receiveMatchInfo();
	void __newOpponent(const UdpBroadcastHostData &data);
	bool updateHostList(const UdpBroadcastHostData &data);
	Q_INVOKABLE void respondJoinRequest(QString ip, bool accepted,
	                                    bool triggerNewSignal = true);
	Q_INVOKABLE void confirmJoinRequest(QString ip);
	Q_INVOKABLE void connectToHost(QString ip);
	Q_INVOKABLE void abortConnectToHost();
	
	Q_INVOKABLE void reset();
	Q_INVOKABLE QString refreshIP();
	
public slots:
	void sendInfo(const QString &qualifier, const QHostAddress &ip = QHostAddress::Broadcast);
	void sendInfo(const QString &qualifier, const QString &ip);
	void updateHostList();
	
private:
	static const int UDP_PORT = 8263;
	const int __udpSendInterval = 1000;
	const int __udpTimeoutInterval = 4000;
	
	QList<UdpBroadcastHostData> pendingOpponent;
	QHostAddress ip, aimHostIP;
	UdpBroadcastHostList *m_hostList;
	QTimer udpSendTimer, udpRefreshTimer;
	QUdpSocket udpSocket;
};

#endif // UDPBROADCASTMANAGER_H
