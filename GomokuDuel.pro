TEMPLATE = app

QT += qml quick network gui
CONFIG += c++11

SOURCES += main.cpp \
	udpbroadcastmanager.cpp \
	tcpnetworkmanager.cpp \
	gamelogic.cpp \
	gamestorage.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

DISTFILES += \
	main.qml \
	MainPage.qml \
	TextButton.qml \
	ConnectPage.qml \
	CustomPage.qml \
	CustomSubtitled.qml \
	CustomBaseListItem.qml \
	CreateMatchDialog.qml \
	ManualIPDialog.qml \
	ClickableSquare.qml \
	ConnectToHostDialog.qml \
	CustomDialog.qml \
	GamePage.qml \
	GameBoard.qml \
	ProfilePage.qml \
	GameBoardToggle.js \
	BoardCircle.qml \
	font/ZhuanTi.ttc

HEADERS += \
	udpbroadcastmanager.h \
	tcpnetworkmanager.h \
	gamelogic.h \
	defineproperty.h \
	gamestorage.h
