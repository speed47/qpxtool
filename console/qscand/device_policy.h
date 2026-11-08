#ifndef QSCAND_DEVICE_POLICY_H
#define QSCAND_DEVICE_POLICY_H

#include <string>
#include <vector>

class device_policy {
public:
	bool allow_all = false;
	std::vector<std::string> devices;

	// Return the authorized path to pass to qscan. Default Unix paths are resolved
	// so aliases cannot redirect qscan to a different kind of device.
	bool authorize(const std::string& requested, std::string& authorized) const;
	static bool default_name_allowed(const std::string& path);
};

extern device_policy device_access;

#endif
