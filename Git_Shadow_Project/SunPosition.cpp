#include <opencv2\opencv.hpp>
#include <opencv\cv.h>
#include <vector>
#include<opencv2\features2d\features2d.hpp>
#include<opencv2\imgproc\imgproc.hpp>
#include"SunPosition.h"

using namespace std;
using namespace cv;

#define M_PI 3.1415



VSPSSunPosition::VSPSSunPosition()
{

}


VSPSSunPosition::~VSPSSunPosition()
{

}

void  VSPSSunPosition::sunPos(int year, int month, int day, double hour, double min, double sec, double lat, double longitude)
{
	//sunposition sp;

	float twopi = 2.0f * M_PI;
	float deg2rad = M_PI / 180;
	int m_month[13] = { 0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30 };
	int leapdays;
	for (int i = 1; i < month; i++)
	{
		day = day + m_month[i];
	}

	if ((year % 4 == 0) && (year % 400 == 0 || year % 100 != 0) && ((day >= 60)&!(month == 2 && day == 60)))
	{
		day = day + 1;
	}

	int julian_date;

	hour = hour + min / 60 + sec / 3600;
	int delta = year - 1949;
	int leap = (int(delta) / 4);
	//double leap = trunc(double(delta) / 4);

	float jd = 32916.5 + delta * 365 + leap + day + hour / 24;
	float m_time = jd - 51545.0;

	//Mean longitude
	double mnlong = 0;
	double rad_calc = 360;
	mnlong = 280.460 + 0.9856474 * m_time;
	mnlong = fmod(mnlong, rad_calc);

	if (mnlong < 0)
		mnlong = mnlong + 360;

	//Mean anomaly
	double mnanom = 357.528 + 0.9856003 * m_time;
	mnanom = fmod(mnanom, rad_calc);

	if (mnanom < 0)
	{
		mnanom = mnanom + 360;
	}
	mnanom = mnanom * deg2rad;

	//Ecliptic longitude and obliquity of ecliptic
	double eclong;
	eclong = mnlong + 1.915 * sin(mnanom) + 0.020 * sin(2 * mnanom);
	eclong = fmod(eclong, rad_calc);
	if (eclong < 0)
		eclong += 360;

	double oblqec = 23.429 - 0.0000004 * m_time;
	eclong = eclong * deg2rad;
	oblqec = oblqec * deg2rad;

	//Celestial coordinates
	double m_num = cos(oblqec) * sin(eclong);
	double m_den = cos(eclong);
	double m_ra = atan(m_num / m_den);
	if (m_den < 0)
		m_ra = m_ra + M_PI;

	if (m_den >= 0 && m_num < 0)
		m_ra = m_ra + 2 * M_PI;
	double m_dec = asin(sin(oblqec)*sin(eclong));

	//Local coordinates greendwich mean sidereal time
	double gmst = 6.697375 + 0.0657098242 * m_time + hour;

	gmst = fmod(gmst, 24);
	if (gmst < 0)
		gmst += 24;

	//local mean side realtime
	double lmst = gmst + longitude / 15.0;
	lmst = fmod(lmst, 24);
	if (lmst < 0)
		lmst += 24.0;
	lmst = lmst * 15.0 * deg2rad;

	//Hour angle
	double m_ha = lmst - m_ra;
	if (m_ha < -M_PI)
		m_ha = m_ha + 2 * M_PI;
	if (m_ha > M_PI)
		m_ha = m_ha - 2 * M_PI;

	//Latitude to radians
	lat = lat * deg2rad;


	//Azimuth and elevation
	double m_el = asin(sin(m_dec) * sin(lat) + cos(m_dec) * cos(lat) * cos(m_ha));
	double m_az = asin(-cos(m_dec) * sin(m_ha) / cos(m_el));

	double cosAzPos = (0 <= (sin(m_dec) - sin(m_el) * sin(lat)));
	double sinAzNeg = (sin(m_az) < 0);
	if (cosAzPos && sinAzNeg)
		m_az = m_az + 2 * M_PI;
	if (!cosAzPos)
		m_az = M_PI - m_az;

	m_el = m_el / deg2rad;
	m_az = m_az / deg2rad;
	double m_lat = lat / deg2rad;

	azimuth = m_az;
	elevation = m_el;
	//return 0;
}