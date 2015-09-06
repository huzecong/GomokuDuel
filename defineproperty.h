#ifndef DEFINEPROPERTY_H
#define DEFINEPROPERTY_H

/*
 * Usage:
 *   // Property isEnabled
 *   signals: DEFINE_PROPERTY(bool, I, i, sEnabled)
 * 
 * The "signals: " is necessary because moc searches for special keywords
 * before expanding macros
 */

#define __DEFINE_PROPERTY(type, readName, setName, notifyName, varName) \
	void notifyName(const type &readName); \
public: \
	Q_PROPERTY(type readName READ readName WRITE setName NOTIFY notifyName) \
	const type &readName() const { \
		return this->varName; \
	} \
	Q_INVOKABLE void setName(const type &readName) { \
		if (this->varName != readName) { \
			this->varName = readName; \
			emit notifyName(readName); \
		} \
	} \
private: \
	type varName; \


#define _DEFINE_PROPERTY(type, lowercase, uppercase) \
	__DEFINE_PROPERTY(type, lowercase, set##uppercase, lowercase##Changed, m_##lowercase)

#define DEFINE_PROPERTY(type, upperfirst, lowerfirst, rest) \
	_DEFINE_PROPERTY(type, lowerfirst##rest, upperfirst##rest)

#endif // DEFINEPROPERTY_H
