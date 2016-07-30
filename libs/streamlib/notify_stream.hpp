/**
 * \brief A stream class to write desktop notifications
 *
 * This class provides an output stream that are translated to desktop notifications using notify-send.
 * There are three streams provided, notify_low, notify_normal and notify_critical, corresponding to the three notifications urgency levels.
 * To write a notification, use eg: notify_normal << "This is shown as the headline" << "This is shown as the notification body" << std::endl;
 * The streams up to the specified urgency level are enabled if USE_NOTIFY_LOW, USE_NOTIFY_NORMAL or USE_NOTIFY_CRITICAL.
 */


#pragma once


#ifdef USE_NOTIFICATIONS

#ifndef NOTIFY_PROGRAM_NAME
	#define NOTIFY_PROGRAM_NAME "My fancy pantsy program"
#endif

#ifndef NOTIFY_TIMEOUT
	#define NOTIFY_TIMEOUT 3000
#endif


#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include "doodle_config.hpp"
#include <libnotify/notify.h>
class Notify_stream: public std::ostream
{
	private:
		static unsigned int instance_count;
		class stream_buf: public std::stringbuf
		{
			private:
				bool is_first;
				std::string first;
				std::string second;
				NotifyUrgency urgency;

			public:
				stream_buf(NotifyUrgency urgency) : is_first(true), first(), second(), urgency(urgency) {}

				virtual int sync()
				{
					NotifyNotification* notification = notify_notification_new(first.c_str(), second.c_str(), "dialog-information");
					notify_notification_set_timeout(notification, NOTIFY_TIMEOUT);
					notify_notification_set_urgency(notification, urgency);
					notify_notification_show(notification, NULL);
					g_object_unref(G_OBJECT(notification));
					second.clear();
					is_first = true;
					return 0;
				}

				std::streamsize xsputn(const char_type* string, std::streamsize length)
				{
					if(is_first) // The first string is the headline...
					{
						first.assign(string, length);
						is_first = false;
					}
					else // ...all following strings make up the body.
					{
						second.append(string, length);
					}
					return length;
				}
		};

		stream_buf buffer;
	public:
		Notify_stream(NotifyUrgency urgency = NOTIFY_URGENCY_NORMAL) : std::ostream(&buffer), buffer(urgency)
		{
			if(!instance_count)
			{
				notify_init(NOTIFY_PROGRAM_NAME);
			}
			instance_count++;
		}
		~Notify_stream(void)
		{
			instance_count--;
			if(!instance_count)
			{
				notify_uninit();
			}
		}
};

// Do not actually use these
extern Notify_stream notify_low_hidden;
extern Notify_stream notify_normal_hidden;
extern Notify_stream notify_critical_hidden;


/**
 * This little stund will only enable the output streams that are actually needed.
 * Source: Randall C. Chang, February 01, 2003
 * Website: http://www.drdobbs.com/more-on-the-cc-comment-macro-for-debug-s/184401612
 */
#if defined(USE_NOTIFY_LOW)
	#define notify_low notify_low_hidden
#else
	#define notify_low if(0)std::cout
#endif


#if defined(USE_NOTIFY_LOW) || defined(USE_NOTIFY_NORMAL)
	#define notify_normal notify_normal_hidden
#else
	#define notify_normal if(0)std::cout
#endif


#if defined(USE_NOTIFY_LOW) || defined(USE_NOTIFY_NORMAL) || defined(USE_NOTIFY_CRITICAL)
	#define notify_critical notify_critical_hidden
#else
	#define notify_critical if(0)std::cout
#endif


#endif
