#include "platform.h"
#include <stdlib.h>
#include <boost/filesystem.hpp>
#include <iostream>

#ifdef WIN32
#include <codecvt>
#endif

#ifdef USE_DBUS
#include "Log.h"
#include <dbus/dbus.h>
#define LOGIND_DEST  "org.freedesktop.login1"
#define LOGIND_PATH  "/org/freedesktop/login1"
#define LOGIND_IFACE "org.freedesktop.login1.Manager"
#endif

std::string getHomePath()
{
	std::string homePath;

	// this should give you something like "/home/YOUR_USERNAME" on Linux and "C:\Users\YOUR_USERNAME\" on Windows
	const char * envHome = getenv("HOME");
	if(envHome != nullptr)
	{
		homePath = envHome;
	}

#ifdef WIN32
	// but does not seem to work for Windows XP or Vista, so try something else
	if (homePath.empty()) {
		const char * envDir = getenv("HOMEDRIVE");
		const char * envPath = getenv("HOMEPATH");
		if (envDir != nullptr && envPath != nullptr) {
			homePath = envDir;
			homePath += envPath;

			for(unsigned int i = 0; i < homePath.length(); i++)
				if(homePath[i] == '\\')
					homePath[i] = '/';
		}
	}
#endif

	// convert path to generic directory seperators
	boost::filesystem::path genericPath(homePath);
	return genericPath.generic_string();
}

#ifdef USE_DBUS
bool HasLogind()
{
	// recommended method by systemd devs. The seats directory
	// doesn't exist unless logind created it and therefore is running.
	// see also https://mail.gnome.org/archives/desktop-devel-list/2013-March/msg00092.html
	return (access("/run/systemd/seats/", F_OK) >= 0);
}

DBusMessage *DBusSystemSend(DBusMessage *message)
{
	DBusError error;
	dbus_error_init (&error);
	DBusConnection *con = dbus_bus_get(DBUS_BUS_SYSTEM, &error);

	DBusMessage *returnMessage = dbus_connection_send_with_reply_and_block(con, message, -1, &error);

	if (dbus_error_is_set(&error))
		LOG(LogError) << "DBus: Error " << error.name << " - " << error.message;

	dbus_error_free (&error);
	dbus_connection_unref(con);

	return returnMessage;
}

bool LogindCheckCapability(const char *capability)
{
	DBusMessage *message = dbus_message_new_method_call(LOGIND_DEST, LOGIND_PATH, LOGIND_IFACE, capability);
	DBusMessage *reply = DBusSystemSend(message);
	char *arg;

	if(reply && dbus_message_get_args(reply, NULL, DBUS_TYPE_STRING, &arg, DBUS_TYPE_INVALID))
	{
		// Returns one of "yes", "no" or "challenge". If "challenge" is
		// returned the operation is available, but only after authorization.
		return (strcmp(arg, "yes") == 0);
	}
	return false;
}

bool LogindSetPowerState(const char *state)
{
	DBusMessage *message = dbus_message_new_method_call(LOGIND_DEST, LOGIND_PATH, LOGIND_IFACE, state);

	// The user_interaction boolean parameters can be used to control
	// wether PolicyKit should interactively ask the user for authentication
	// credentials if it needs to.
	DBusMessageIter args;
	dbus_bool_t arg = false;
	dbus_message_iter_init_append(message, &args);
	dbus_message_iter_append_basic(&args, DBUS_TYPE_BOOLEAN, &arg);

	return DBusSystemSend(message) != NULL;
}
#endif

int runShutdownCommand()
{
#ifdef WIN32 // windows
	return system("shutdown -s -t 0");
#else // osx / linux
#ifdef USE_DBUS
	if(HasLogind() && LogindCheckCapability("CanPowerOff")) {
		LOG(LogInfo) << "LogindSetPowerState('PowerOff')";
		Log::flush();
		return LogindSetPowerState("PowerOff");
	} else
#endif
	return system("sudo shutdown -h now");
#endif
}

int runRestartCommand()
{
#ifdef WIN32 // windows
	return system("shutdown -r -t 0");
#else // osx / linux
#ifdef USE_DBUS
	if(HasLogind() && LogindCheckCapability("CanReboot")) {
		LOG(LogInfo) << "LogindSetPowerState('Reboot')";
		Log::flush();
		return LogindSetPowerState("Reboot");
	} else
#endif
	return system("sudo shutdown -r now");
#endif
}

int runSystemCommand(const std::string& cmd_utf8)
{
#ifdef WIN32
	// on Windows we use _wsystem to support non-ASCII paths
	// which requires converting from utf8 to a wstring
	typedef std::codecvt_utf8<wchar_t> convert_type;
	std::wstring_convert<convert_type, wchar_t> converter;
	std::wstring wchar_str = converter.from_bytes(cmd_utf8);
	return _wsystem(wchar_str.c_str());
#else
	return system(cmd_utf8.c_str());
#endif
}