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

#include <cassert>
#include <cmath>

class NetworkHostData : public QObject {
	Q_OBJECT

public:
	QDateTime dateTime;
	
	NetworkHostData(const QString &_name = "", const QString &_ip = "",
	                const QString &_avatarId = "", int _rounds = 1, int _win = 0,
	                const QDateTime &_dateTime = QDateTime())
	    : m_name(_name), m_ip(_ip), m_avatarId(_avatarId), m_rounds(_rounds),
	      m_win(_win), dateTime(_dateTime) {}
	NetworkHostData(const NetworkHostData &data) {
		this->m_name = data.m_name;
		this->m_ip = data.m_ip;
		this->m_avatarId = data.m_avatarId;
		this->m_win = data.m_win;
		this->m_rounds = data.m_rounds;
		this->dateTime = data.dateTime;
	}
	NetworkHostData &operator = (const NetworkHostData &data) {
		this->m_name = data.m_name;
		this->m_ip = data.m_ip;
		this->m_avatarId = data.m_avatarId;
		this->m_win = data.m_win;
		this->m_rounds = data.m_rounds;
		this->dateTime = data.dateTime;
		return *this;
	}
	
	inline bool operator == (const NetworkHostData &data) const {
		if (this->m_name != data.m_name) return false;
		if (this->m_ip != data.m_ip) return false;
		return true;
	}
	
	Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
	const QString &name() const {
		return this->m_name;
	}
	void setName(const QString &name) {
		if (this->m_name != name) {
			this->m_name = name;
			emit nameChanged(name);
		}
	}
	Q_PROPERTY(QString ip READ ip WRITE setIP NOTIFY ipChanged)
	const QString &ip() const {
		return this->m_ip;
	}
	void setIP(const QString &ip) {
		if (this->m_ip != ip) {
			this->m_ip= ip;
			emit ipChanged(ip);
		}
	}
	Q_PROPERTY(QString avatarId READ avatarId WRITE setAvatarId NOTIFY avatarIdChanged)
	const QString &avatarId() const {
		return this->m_avatarId;
	}
	void setAvatarId(const QString &avatarId) {
		if (this->m_avatarId != avatarId) {
			this->m_avatarId = avatarId;
			emit avatarIdChanged(avatarId);
		}
	}
	Q_PROPERTY(int rounds READ rounds WRITE setRounds NOTIFY roundsChanged)
	int rounds() const {
		return this->m_rounds;
	}
	void setRounds(int rounds) {
		if (this->m_rounds!= rounds) {
			this->m_rounds = rounds;
			emit roundsChanged(rounds);
		}
	}
	Q_PROPERTY(double winningRate READ winningRate WRITE setWinningRate NOTIFY winningRateChanged)
	double winningRate() const {
		return (double)this->m_win * 100.0 / this->m_rounds;
	}
	void setWinningRate(double rate) {
		int win = round(rate * this->m_rounds / 100.0);
		if (this->m_win!= win) {
			this->m_win = win;
			emit winningRateChanged(winningRate());
		}
	}
	
signals:
	void nameChanged(const QString &name);
	void ipChanged(const QString &ip);
	void avatarIdChanged(const QString &avatarId);
	void roundsChanged(int rounds);
	void winningRateChanged(double winningRate);
	
private:
	QString m_name, m_ip;
	QString m_avatarId;	// used with custom image provider
	int m_rounds, m_win;
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
	
	friend class NetworkManager;
	
	Q_PROPERTY(int count READ count NOTIFY countChanged)
	int count() const {
		return this->m_data.count();
	}
	
signals:
	void countChanged(int count);
	
public:
    virtual int rowCount(const QModelIndex &parent) const override;
    virtual QVariant data(const QModelIndex &index, int role) const override;
	
	void insert(int index, const NetworkHostData &data);
	void append(const NetworkHostData &data);
	void remove(int index);
	NetworkHostData &get(int index) {
		return const_cast<NetworkHostData &>(this->m_data.at(index));
	}
	const NetworkHostData &get(int index) const {
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

	Q_PROPERTY(NetworkHostList *hostList READ hostList)
	NetworkHostList *hostList() const {
		return this->m_hostList;
	}
    Q_PROPERTY(bool isHost READ isHost WRITE setIsHost NOTIFY isHostChanged)
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
	void hostAddressChanged(const QString &hostAddress);
	
	void newOpponent(const NetworkHostData &data);
	
public slots:
	void sendInfo(const QString &qualifier);
	void startHost();
	void abortHost();
	void receiveMatchInfo();
	bool updateHostList(const NetworkHostData &data);
	void respondJoinRequest(const NetworkHostData &data, bool accepted,
	                        bool triggerNewSignal = true);
	void confirmJoinRequest(const NetworkHostData &data);
	void connectToHost(const QString &ip);
	
private:
	static const int UDP_PORT = 8263;
	static const int TCP_PORT = 41013;
	const int __udpSendInterval = 1000;
	const int __udpTimeoutInterval = 5000;
	
	QList<NetworkHostData> pendingOpponent;
	QHostAddress ip;
	NetworkHostList *m_hostList;
	QTimer udpSendTimer, udpRefreshTimer;
	QUdpSocket udpSocket;
	bool isServer;
	QTcpServer *tcpServer;
	QTcpSocket *tcpSocket;
};

#endif // NETWORKMANAGER_H
