#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <time.h>

#include "CFileUtil.h"

// 日付管理クラス
class CTime
{
public:
	int		iYear;		// 年
	int		iMonth;		// 月
	int		iDay;		// 日
	int		iHour;		// 時
	int		iMinute;	// 分
	int		iSecond;	// 秒

	int		iYDayCnt;	// 年内の通し日数

	CTime()
	{
		iYear = 0; iMonth = 0; iDay = 0;
		iHour = 0; iMinute = 0; iSecond = 0;
		iYDayCnt = 0;
	};

	CTime(const int& year, const int& month, const int& day, const int& hour, const int& minute, const int& sec)
	{
		iYear = year; iMonth = month; iDay = day;
		iHour = hour; iMinute = minute; iSecond = sec;
		iYDayCnt = GetYDateCount(month, day);
	};

	CTime& operator ++ ()
	{
		struct tm tm = { 0, 0, 0, iDay + 1, iMonth - 1, iYear - 1900};
		time_t t = mktime(&tm);
		struct tm result;
		localtime_s(&result, &t);
		if ((result.tm_mon + 1 == 2) && (result.tm_mday == 29))	// うるう年は考慮しない
		{
			tm.tm_mday++;
			t = mktime(&tm);
			localtime_s(&result, &t);
		}
		iDay = result.tm_mday; iMonth = result.tm_mon + 1; iYear = result.tm_year + 1900;
		iYDayCnt = GetYDateCount(iMonth, iDay);
		return *this;
	}

	bool operator <= (CTime a)
	{
		struct tm tm1, tm2;
		tm1.tm_year = iYear - 1900; tm1.tm_mon = iMonth - 1; tm1.tm_mday = iDay;
		tm1.tm_hour = iHour; tm1.tm_min = iMinute; tm1.tm_sec = iSecond;
		tm1.tm_yday = iYDayCnt;
		tm2.tm_year = a.iYear - 1900; tm2.tm_mon = a.iMonth - 1; tm2.tm_mday = a.iDay;
		tm2.tm_hour = a.iHour; tm2.tm_min = a.iMinute; tm2.tm_sec = a.iSecond;
		tm2.tm_yday = a.iYDayCnt;
		return (mktime(&tm1) <= mktime(&tm2));
	}

public:
	// フォーマットはyyyy/mm/dd, HH:MM:SS に対応
	void SetDateTime(const std::string& date, const std::string& time)
	{
		std::vector<std::string> strSplit;

		if (!date.empty())
		{
			GetFUtil()->SplitCSVData(date, &strSplit, '/');
			if (strSplit.size() == 3)
			{
				this->iYear = stoi(strSplit[0]);
				this->iMonth = stoi(strSplit[1]);
				this->iDay = stoi(strSplit[2]);
			}
		}

		if (!time.empty())
		{
			GetFUtil()->SplitCSVData(time, &strSplit, ':');
			if (strSplit.size() == 2)
			{
				this->iHour = stoi(strSplit[0]);
				this->iMinute = stoi(strSplit[1]);
			}
			else if (strSplit.size() == 3)
			{
				this->iHour = stoi(strSplit[0]);
				this->iMinute = stoi(strSplit[1]);
				this->iSecond = stoi(strSplit[2]);
			}
		}

	};

	void SetYear(const int& y) { this->iYear = y; };
	void SetMonth(const int& m) { this->iMonth = m; };
	void SetDay(const int& d) { this->iDay = d; };
	void SetHour(const int& h) { this->iHour = h; };
	void SetMinute(const int& m) { this->iMinute = m; };
	void SetSecond(const int& s) { this->iSecond = s; };

	int GetYear() { return this->iYear; };
	int GetMonth() { return this->iMonth; };
	int GetDay() { return this->iDay; };
	int GetHour() { return this->iHour; };
	int GetMinute() { return this->iMinute; };
	int GetYDateCount() { return this->iYDayCnt; };

	/* 年内の通し日数(1/1からの経過日数)を取得 */
	static int GetYDateCount(const int& month, const int& day)
	{
		int nYDayCnt = 0;
		const int MD[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };	// 毎月の日数、閏年は考慮しない
		for (int i = 0; i < month - 1; i++)
		{
			nYDayCnt += MD[i];
		}
		nYDayCnt += (day - 1);
		return nYDayCnt;
	};

	/* 時間差分(秒) time2 - time1*/
	static double DiffTime(const CTime& time1, const CTime& time2)
	{
		struct tm tm1, tm2;
		tm1.tm_year = time1.iYear - 1900; tm1.tm_mon = time1.iMonth - 1; tm1.tm_mday = time1.iDay;
		tm1.tm_hour = time1.iHour; tm1.tm_min = time1.iMinute; tm1.tm_sec = time1.iSecond;
		tm1.tm_yday = time1.iYDayCnt;
		tm2.tm_year = time2.iYear - 1900; tm2.tm_mon = time2.iMonth - 1; tm2.tm_mday = time2.iDay;
		tm2.tm_hour = time2.iHour; tm2.tm_min = time2.iMinute; tm2.tm_sec = time2.iSecond;
		tm2.tm_yday = time2.iYDayCnt;
		time_t t1 = mktime(&tm1);
		time_t t2 = mktime(&tm2);
		double diff = difftime(t2, t1);

		return diff;
	};

	/* 現在時刻を取得 */
	static CTime GetCurrentTime()
	{
		time_t t = time(NULL);
		struct tm local;
		localtime_s(&local, &t);
		CTime time(local.tm_year + 1900, local.tm_mon + 1, local.tm_mday, local.tm_hour, local.tm_min, local.tm_sec);
		return time;
	};

	std::string Format(std::string s)
	{
		struct tm time;
		time.tm_year = this->iYear - 1900; time.tm_mon = this->iMonth - 1; time.tm_mday = this->iDay;
		time.tm_hour = this->iHour; time.tm_min = this->iMinute; time.tm_sec = this->iSecond;
		time.tm_yday = this->iYDayCnt;
		char buf[128];
		strftime(buf, sizeof(buf), s.c_str(), &time);
		return buf;
	};

	/* 夏至、春分、冬至の日にちを取得 */
	// 2099年までは以下の計算式が使える
	static CTime GetSummerSolstice(const int& year)		// 夏至
	{
		CTime time;
		time.iYear = year; time.iMonth = 6;
		time.iDay = int(22.2747 + 0.24162603 * ((int64_t)year - 1900) - int(((int64_t)year - 1900) / 4));
		time.iYDayCnt = GetYDateCount(time.iMonth, time.iDay);
		return time;
	};

	static CTime GetVernalEquinox(const int& year)		// 春分の日
	{
		CTime time;
		time.iYear = year; time.iMonth = 3;
		time.iDay = int(20.8431 + 0.242194 * (((int64_t)year - 1980) - int(((int64_t)year - 1980) / 4)));
		time.iYDayCnt = GetYDateCount(time.iMonth, time.iDay);
		return time;
	};

	static CTime GetWinterSolstice(const int& year)		// 冬至
	{
		CTime time;
		time.iYear = year; time.iMonth = 12;
		time.iDay = int(22.6587 + 0.24274049 * ((int64_t)year - 1900) - int(((int64_t)year - 1900) / 4));
		time.iYDayCnt = GetYDateCount(time.iMonth, time.iDay);
		return time;
	};

	// 月を指定して日数を取得
	static int GetDayNum(const int& month)
	{
		const int MD[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };	// 毎月の日数、閏年は考慮しない
		return MD[month - 1];
	};

};