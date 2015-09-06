#include <QDebug>
#include "gamestorage.h"

GameStorage::GameStorage(QObject *parent) : QObject(parent) {
	srand(time(0));
	
	QSettings settings;
	this->m_uniqueId = settings.value("uniqueId", rand()).toInt();
	settings.setValue("uniqueId", uniqueId());
	setProfileName(settings.value("profileName", QString("Player %1").arg(uniqueId())).toString());
	setAvatarId(settings.value("avatarId", QString("gomoku.jpg")).toString());
}

int GameStorage::rounds(int uniqueId) {
	QSettings settings;
	return settings.value(QString("%1.rounds").arg(uniqueId), 0).toInt();
}

void GameStorage::setRounds(int uniqueId, int rounds) {
	QSettings settings;
	settings.setValue(QString("%1.rounds").arg(uniqueId), rounds);
}

void GameStorage::increaseRounds() {
	setRounds(opponentUniqueId(), rounds(opponentUniqueId()) + 1);
}

void GameStorage::decreaseRounds() {
	setRounds(opponentUniqueId(), rounds(opponentUniqueId()) - 1);
}

int GameStorage::wins(int uniqueId) {
	QSettings settings;
	return settings.value(QString("%1.wins").arg(uniqueId), 0).toInt();
}

double GameStorage::winningRate(int uniqueId) {
	QSettings settings;
	int rounds = this->rounds(uniqueId), wins = this->wins(uniqueId);
	if (rounds == 0) return 0.0;
	return (double)wins * 100.0 / rounds;
}

void GameStorage::setWins(int uniqueId, int wins) {
	QSettings settings;
	settings.setValue(QString("%1.wins").arg(uniqueId), wins);
}

void GameStorage::increaseWins() {
	setWins(opponentUniqueId(), wins(opponentUniqueId()) + 1);
}

QString GameStorage::getFileName() const {
	QString timeString = QDateTime::currentDateTime().toString(Qt::ISODate).replace(':', "_");
	QString path = "save_file_" + timeString + ".txt";
	qDebug() << path;
	return path;
}

QUrl GameStorage::getFileDirectory() const {
	QUrl dir = QUrl(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
	qDebug() << dir;
	return dir;
}

QString GameStorage::getNameFromDir(QString dir) const {
	QString name = QFileInfo(dir).fileName();
	qDebug() << name;
	return name;
}
