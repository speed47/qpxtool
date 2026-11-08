#include "device_policy.h"
#include <iostream>
#include <cstring>
#include <sys/stat.h>

// Model optical devices and aliases without requiring root or real hardware.
extern "C" char* __wrap_realpath(const char* path, char*) {
	if (!strcmp(path, "/dev/cdrom") || !strcmp(path, "/dev/scd0")) return strdup("/dev/sr0");
	if (!strcmp(path, "/dev/dvd")) return strdup("/dev/sda");
	if (!strcmp(path, "/dev/sr1")) return strdup("/etc/passwd");
	if (!strcmp(path, "/dev/sr0") || !strcmp(path, "/dev/sr2")) return strdup(path);
	return nullptr;
}

extern "C" int __wrap_stat(const char* path, struct stat* info) {
	memset(info, 0, sizeof(*info));
	info->st_mode = !strcmp(path, "/dev/sr0") ? S_IFBLK : S_IFREG;
	return 0;
}

int main() {
	int failures = 0;
	auto check = [&](bool condition, const char* label) {
		if (!condition) {
			std::cerr << label << '\n';
			++failures;
		}
	};
	for (const auto* path : {"/dev/sr0", "/dev/sr123", "/dev/scd0"})
		check(device_policy::default_name_allowed(path), path);
	for (const auto* path : {"/dev/sr", "/dev/sr-1", "/dev/sr0/../sda", "/dev/sr0/", "/dev/sr0x", "/dev/sda",
	                         "/dev/sg0", "/dev/null", "/dev/disk0", "/etc/passwd", "sr0"})
		check(!device_policy::default_name_allowed(path), path);
	device_policy policy;
	std::string authorized = "stale";
	check(!policy.authorize("/dev/null", authorized) && authorized.empty(), "default denies non-optical device");
	check(policy.authorize("/dev/cdrom", authorized) && authorized == "/dev/sr0",
	      "optical alias resolves to canonical path");
	check(policy.authorize("/dev/scd0", authorized) && authorized == "/dev/sr0", "legacy optical alias");
	check(policy.authorize("/dev/sr0", authorized), "canonical optical device");
	check(!policy.authorize("/dev/dvd", authorized), "alias to non-optical device rejected");
	check(!policy.authorize("/dev/sr1", authorized), "optical-looking alias to regular file rejected");
	check(!policy.authorize("/dev/sr2", authorized), "regular file with optical name rejected");
	check(!policy.authorize("/dev/sr3", authorized), "missing optical device rejected");
	policy.devices = {"/dev/null", "/custom/optical device"};
	check(policy.authorize("/custom/optical device", authorized) && authorized == "/custom/optical device",
	      "exact list");
	check(!policy.authorize("/dev/sr0", authorized), "list replaces defaults");
	check(!policy.authorize("/custom/optical device/../other", authorized), "no prefix matching");
	policy.allow_all = true;
	check(policy.authorize("/legacy/path", authorized), "unrestricted override");
	for (const std::string& path :
	     {std::string(), std::string("/dev/sr0\n"), std::string("/dev/sr0\r"), std::string("/dev/sr0\0ignored", 16)})
		check(!policy.authorize(path, authorized), "invalid protocol path");
	return failures ? 1 : 0;
}
