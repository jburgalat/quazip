#include "times.hpp"
#include <QMap>

double dayToHours(double days, double &remaining) {
    double v = days * 24.0;
    remaining = v - (int)v;
    return v;
}

double dayToMins(double days, double &remaining) {
    double v = days * 24.0 * 60.0;
    remaining = v - (int)v;
    return v;
}

double dayToSecs(double days, double &remaining) {
    double v = days * 24.0 * 60.0 * 60.0;
    remaining = v - (int)v;
    return v;
}

double dayToMSecs(double days, double &remaining) {
    double v = days * 24.0 * 60.0 * 60.0 * 1000.0;
    remaining = v - (int)v;
    return v;
}

double hourToMins(double hours, double &remaining) {
    double v = hours * 60.0;
    remaining = v - (int)v;
    return v;
}

double hourToSecs(double hours, double &remaining) {
    double v = hours * 60.0 * 60.0;
    remaining = v - (int)v;
    return v;
}

double hourToMSecs(double hours, double &remaining) {
    double v = hours * 60.0 * 60.0 * 1000.0;
    remaining = v - (int)v;
    return v;
}

double minToSecs(double mins, double &remaining) {
    double v = mins * 60.0;
    remaining = v - (int)v;
    return v;
}

double minToMSecs(double mins, double &remaining) {
    double v = mins * 60.0 * 1000.0;
    remaining = v - (int)v;
    return v;
}

double secToMSecs(double secs, double &remaining) {
    double v = secs * 1000.0;
    remaining = v - (int)v;
    return v;
}

double msecsToDays(qint64 ms, double &remaining) {
    double v = ms / (24.0 * 60.0 * 60.0 * 1000.0);
    remaining = v - (int)v;
    return v;
}

double msecsToHours(qint64 ms, double &remaining) {
    double v = ms / (60.0 * 60.0 * 1000.0);
    remaining = v - (int)v;
    return v;
}

double msecsToMins(qint64 ms, double &remaining) {
    double v = ms / (60.0 * 1000.0);
    remaining = v - (int)v;
    return v;
}

double msecsToSecs(qint64 ms, double &remaining) {
    double v = ms / (1000.0);
    remaining = v - (int)v;
    return v;
}

void fromMinutes(QString fmt, double min_fraction, double &s, double &z) {
    double r;
    if (fmt.contains("%s")) {
        s = minToSecs(min_fraction, r);
        z = fmt.contains("%z") ? secToMSecs(r, r) : 0.0;
    } else if (fmt.contains("%z")) {
        z = minToMSecs(min_fraction, r);
    }
}

void fromHours(QString fmt, double hour_fraction, double &m, double &s, double &z) {
    double r;
    if (fmt.contains("%m")) {
        m = hourToMins(hour_fraction, r);
        fromMinutes(fmt, r, s, z);
    } else if (fmt.contains("%s")) {
        s = hourToSecs(hour_fraction, r);
        z = fmt.contains("%z") ? secToMSecs(r, r) : 0.0;
    } else if (fmt.contains("%z")) {
        z = hourToMSecs(hour_fraction, r);
    }
}

void fromDays(QString fmt, double day_fraction, double &h, double &m, double &s, double &z) {
    double r;
    if (fmt.contains("%h")) {
        h = dayToHours(day_fraction, r);
        fromHours(fmt, r, m, s, z);
    } else if (fmt.contains("%m")) {
        m = dayToMins(day_fraction, r);
        fromMinutes(fmt, r, s, z);
    } else if (fmt.contains("%s")) {
        s = dayToSecs(day_fraction, r);
        z = fmt.contains("%z") ? secToMSecs(r, r) : 0.0;
    } else if (fmt.contains("%z")) {
        z = dayToMSecs(day_fraction, r);
    }
}

QString msecsToTimeFormat(qint64 msecs, QString fmt) {
    double d, h, m, s, z;
    double r;
    d = h = m = s = z = r = 0.0;

    if (fmt.contains("%d")) {
        d = msecsToDays(msecs, r);
        fromDays(fmt, r, h, m, s, z);
    } else if (fmt.contains("%h")) {
        h = msecsToHours(msecs, r);
        fromHours(fmt, r, m, s, z);
    } else if (fmt.contains("%m")) {
        m = msecsToMins(msecs, r);
        fromMinutes(fmt, r, s, z);
    } else if (fmt.contains("%s")) {
        s = msecsToSecs(msecs, r);
        z = fmt.contains("%z") ? secToMSecs(r, r) : 0.0;
    } else if (fmt.contains("%z"))
        z = msecs;

    QMap<QString, int> patterns = {{"%d", (int)d}, {"%h", (int)h}, {"%m", (int)m}, {"%s", (int)s}, {"%z", (int)z}};
    QString out = fmt;
    for (QMap<QString, int>::const_iterator it = patterns.begin(); it != patterns.end(); it++) {
        out.replace(it.key(), QString::number(it.value()));
    }
    return out;
}
