#include "gamecontroller.h"

GameController::GameController(QObject *parent) : QObject(parent) {
	this->m_network = NULL;
}

void GameController::initNetwork() {
	this->m_network = new NetworkManager();
}

void GameController::destroyNetwork() {
	delete this->m_network;
	this->m_network = NULL;
}
