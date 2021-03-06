#include "version.h"

void CheckVersionThread::run()
{
	auto v = checkForNewVersion();
	if (v.major > 0) {
		const auto vs = versionToString(v);
		emit notifyOnNewVersion(vs);
	}
}

Version getCurrentVersionRaw()
{
	QFile f(":/files/version");
	f.open(QFile::ReadOnly | QFile::Text);
	const auto version = f.readLine().trimmed();
	f.close();

	const auto parts = version.split('.');
	return Version(parts[0].toInt(), parts[1].toInt(), parts[2].toInt());
}

QString versionToString(const Version& v)
{
	return QString("%1.%2.%3")
		.arg(v.major).arg(v.minor).arg(v.patch);
}

QString removeTrailingVersionZero(const QString& vs)
{
	if (vs.endsWith(".0")) {
		QString tmp(vs);
		tmp.chop(2);
		return tmp;
	}
	return vs;
}

QString getCurrentVersion()
{
	return versionToString(getCurrentVersionRaw());
}

Version checkForNewVersion()
{
	QNetworkAccessManager mgr;
	QEventLoop loop;
	QNetworkRequest req(QUrl(
		"http://gortnar.com/vmt/version.txt"));

	QNetworkReply* reply = mgr.get(req);
	QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
	loop.exec();
 
	if (reply->error() != QNetworkReply::NoError) {
		delete reply;
		return Version(-1, 0, 0);
	}

	const auto parts = reply->readAll().trimmed().split('.');
	delete reply;
	const auto v = Version(
		parts[0].toInt(), parts[1].toInt(), parts[2].toInt());
	const auto cv = getCurrentVersionRaw();

	if (v.major * 10000 + v.minor * 100 + v.patch >
			cv.major * 10000 + cv.minor * 100 + cv.patch)
		return v;

	return Version(0, 0, 0);
}
