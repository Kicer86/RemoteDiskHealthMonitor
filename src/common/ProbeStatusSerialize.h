
#pragma once

#include <QDataStream>


inline QDataStream& operator<<(QDataStream& _out, const ProbeStatus& _probeStatus)
{
    _out << static_cast<qint8>(_probeStatus.health);
    _out << _probeStatus.jsonData;

	return _out;
}

inline QDataStream& operator>>(QDataStream& _in, ProbeStatus& _probeStatus)
{
    qint8 health;
    QString jsonData;

	_in >> health >> jsonData;

    _probeStatus.health = static_cast<GeneralHealth::Health>(health);
    _probeStatus.jsonData = jsonData;

	return _in;
}
