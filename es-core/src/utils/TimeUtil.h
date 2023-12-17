#pragma once
#ifndef ES_CORE_UTILS_TIME_UTIL_H
#define ES_CORE_UTILS_TIME_UTIL_H

#include <string>
#include <time.h>

namespace Utils
{
	namespace Time
	{
		static inline time_t blankDate() {
			// 1970-01-02
			tm timeStruct = { 0, 0, 0, 2, 0, 70, 0, 0, -1 };
			return mktime(&timeStruct);
		}
		static int NOT_A_DATE_TIME = 0;
		static time_t BLANK_DATE = blankDate();

		class DateTime
		{
		public:

			 DateTime();
			 DateTime(const time_t& _time);
			 DateTime(const tm& _timeStruct);
			 DateTime(const std::string& _isoString);
			~DateTime();

			const bool operator<           (const DateTime& _other) const { return (mTime <  _other.mTime); }
			const bool operator<=          (const DateTime& _other) const { return (mTime <= _other.mTime); }
			const bool operator>           (const DateTime& _other) const { return (mTime >  _other.mTime); }
			const bool operator>=          (const DateTime& _other) const { return (mTime >= _other.mTime); }
			           operator time_t     ()                       const { return mTime; }
			           operator tm         ()                       const { return mTimeStruct; }
			           operator std::string()                       const { return mIsoString; }

			void               setTime      (const time_t& _time);
			const time_t&      getTime      () const { return mTime; }
			void               setTimeStruct(const tm& _timeStruct);
			const tm&          getTimeStruct() const { return mTimeStruct; }
			void               setIsoString (const std::string& _isoString);
			const std::string& getIsoString () const { return mIsoString; }

		private:

			time_t      mTime;
			tm          mTimeStruct;
			std::string mIsoString;

		}; // DateTime

		class Duration
		{
		public:

			 Duration(const time_t& _time);
			~Duration();

			unsigned int getDays   () const { return mDays; }
			unsigned int getHours  () const { return mHours; }
			unsigned int getMinutes() const { return mMinutes; }
			unsigned int getSeconds() const { return mSeconds; }

		private:

			unsigned int mTotalSeconds;
			unsigned int mDays;
			unsigned int mHours;
			unsigned int mMinutes;
			unsigned int mSeconds;

		}; // Duration

		time_t      now         ();
		time_t      stringToTime(const std::string& _string, const std::string& _format = "%Y%m%dT%H%M%S");
		std::string timeToString(const time_t& _time, const std::string& _format = "%Y%m%dT%H%M%S");
		int         daysInMonth (const int _year, const int _month);
		int         daysInYear  (const int _year);

	} // Time::

} // Utils::

#endif // ES_CORE_UTILS_TIME_UTIL_H
