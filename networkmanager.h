#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

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

struct NetworkHostData {
	QString name, ip;
	QString avatarId;	// used with custom image provider
	int rounds, win;
	QDateTime dateTime;
	
	NetworkHostData(const QString &_name, const QString &_ip,
	                const QString &_avatarId, int _rounds, int _win,
	                const QDateTime &_dateTime)
	    : name(_name), ip(_ip), avatarId(_avatarId), rounds(_rounds),
	      win(_win), dateTime(_dateTime) {}
	
	inline bool operator == (const NetworkHostData &data) const {
		if (name != data.name) return false;
		if (ip != data.ip) return false;
		return true;
	}
};

class NetworkHostList : public QAbstractListModel {
	Q_OBJECT
public:
	enum RoleNames {
		NameRole = Qt::UserRole,
		AvatarRole = Qt::UserRole + 2,
		IPRole = Qt::UserRole + 3,
		RoundsRole = Qt::UserRole + 4,
		WinningRateRole = Qt::UserRole + 5
	};
	explicit NetworkHostList(QObject *parent = 0);
	~NetworkHostList() {}
	
	friend class NetworkManager;
	
	Q_PROPERTY(int count READ count NOTIFY countChanged)
	
signals:
	void countChanged(int count);
	
public:
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
	int count() const {
		return this->m_data.count();
	}
	void insert(int index, const NetworkHostData &data);
	void append(const NetworkHostData &data);
	void remove(int index);
	const NetworkHostData &get(int index) {
		return this->m_data.at(index);
	}
	
protected:
    virtual QHash<int, QByteArray> roleNames() const override;
	
private:
    QList<NetworkHostData> m_data;
    QHash<int, QByteArray> m_roleNames;
	
	void add(const NetworkHostData &data);
	void remove(const NetworkHostData &data);
};

class NetworkManager : public QObject {
	Q_OBJECT
public:
	explicit NetworkManager(QObject *parent = 0);
	
	Q_PROPERTY(NetworkHostList *hostList READ hostList)
    Q_PROPERTY(bool isHost READ isHost WRITE setIsHost NOTIFY isHostChanged)
	
	NetworkHostList *hostList() const {
		return this->m_hostList;
	}
    bool isHost() const {
        return this->isServer;
    }
    void setIsHost(bool host) {
        if (host != this->isServer) {
            this->isServer = host;
            emit isHostChanged(host);
        }
    }
	
signals:
    void isHostChanged(bool isHost);
	
public slots:
	void sendInfo(const QString &qualifier);
	
	void receiveMatchInfo();
	
private:
	static const int UDP_PORT = 8263;
	static const int TCP_PORT = 41013;
	const int __udpSendInterval = 1000;
	const int __udpTimeoutInterval = 5000;
	
	QHostAddress ip;
	NetworkHostList *m_hostList;
	QTimer udpSendTimer, udpRefreshTimer;
	QUdpSocket udpSocket;
	bool isServer;
	QTcpServer *tcpServer;
	QTcpSocket *tcpSocket;
};

#endif // NETWORKMANAGER_H
