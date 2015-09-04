#ifndef GAMECONTROLLER_H
#define GAMECONTROLLER_H

#include <QObject>
#include "networkmanager.h"

class GameController : public QObject {
	Q_OBJECT
public:
	explicit GameController(QObject *parent = 0);
	
	Q_PROPERTY(NetworkManager *network READ network)
	NetworkManager *network() const {
		return this->m_network;
	}
	
signals:
	
public slots:
	void initNetwork();
	void destroyNetwork();
	
private:
	NetworkManager *m_network;
};

#endif // GAMECONTROLLER_H
