#include "gamestorage.h"

GameStorage::GameStorage(QObject *parent) : QObject(parent) {
	srand(time(0));
	
	QSettings settings("Kanari", "GomokuDuel");
	this->m_profileName = settings.value("profileName", QString("Player %1").arg(rand())).toString();
	this->m_avatarId = settings.value("avatarId", QString("gomoku.jpg")).toString();
}
