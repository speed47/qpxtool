#include "device_policy.h"

#include <algorithm>
#include <cstdlib>
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <sys/stat.h>
#endif

device_policy device_access;

#if defined(__linux__) || defined(__FreeBSD__)
static bool numbered_device(const std::string& path, const std::string& prefix) {
	return path.size() > prefix.size() && path.compare(0, prefix.size(), prefix) == 0 &&
	       path.find_first_not_of("0123456789", prefix.size()) == std::string::npos;
}
#endif

bool device_policy::default_name_allowed(const std::string& path) {
#if defined(__linux__)
	return numbered_device(path, "/dev/sr") || numbered_device(path, "/dev/scd");
#elif defined(__FreeBSD__)
	return numbered_device(path, "/dev/cd") || numbered_device(path, "/dev/acd");
#elif defined(_WIN32) || defined(_WIN64)
	return path.size() == 2 && ((path[0] >= 'A' && path[0] <= 'Z') || (path[0] >= 'a' && path[0] <= 'z')) &&
	       path[1] == ':';
#else
	// macOS diskN/rdiskN names also identify non-optical disks. Require an
	// administrator's explicit list on platforms without optical-only names.
	return false;
#endif
}

bool device_policy::authorize(const std::string& requested, std::string& authorized) const {
	authorized.clear();
	// Protocol paths must be one argument, without embedded NUL or line endings.
	if (requested.empty() || requested.find_first_of(std::string("\0\r\n", 3)) != std::string::npos) return false;
	if (allow_all || !devices.empty()) {
		if (!allow_all && std::find(devices.begin(), devices.end(), requested) == devices.end()) return false;
		authorized = requested;
		return true;
	}
	bool known_name = default_name_allowed(requested);
#if defined(__linux__)
	known_name = known_name || requested == "/dev/cdrom" || requested == "/dev/dvd";
#endif
	if (!known_name) return false;
#if defined(_WIN32) || defined(_WIN64)
	if (GetDriveTypeA((requested + "\\").c_str()) != DRIVE_CDROM) return false;
	authorized = requested;
#else
	char* resolved = realpath(requested.c_str(), NULL);
	if (!resolved) return false;
	std::string canonical(resolved);
	free(resolved);
	struct stat info;
	if (!default_name_allowed(canonical) || stat(canonical.c_str(), &info) ||
	    !(S_ISBLK(info.st_mode) || S_ISCHR(info.st_mode)))
		return false;
	authorized = canonical;
#endif
	return true;
}
