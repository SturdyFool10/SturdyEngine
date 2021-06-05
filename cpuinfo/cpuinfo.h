#include <string>
bool initialized = false;
#ifdef _M_AMD64
#define arch "x86"
#elif defined _M_IX86
#define arch "x86"
#elif defined _M_ARM
#define arch "ARM"
#elif defined _M_ARM64
#define arch "ARM"
#else
#error "SturdyEngine does not support your archetechure, please compile for either x86-64 or for ARM"
#endif
class ProcessorDescriptor {
public:
	std::string ar;
	int32_t flags0 = 0x0000;
	int32_t flags1 = 0x0000;
	std::string brandname;
	std::string vendor;
	bool getFlag0(int shift) {
		return (flags0 & shift) != 0;
	}
	bool getFlag1(int shift) {
		return (flags1 & shift) != 0;
	}
private:
	void setArch(std::string ar) {
		this->ar = ar;
	}
};
#ifdef _M_AMD64
#include "cpuinfox86.h"
#define cpuLibIncluded true
#elif defined _M_IX86
#ifndef cpuLibIncluded
#include "cpuinfox86.h"
#define cpuLibIncluded true
#endif
#elif defined _M_ARM
#ifndef cpuLibIncluded
#include "cpuinfoARM.h"	
#define cpuLibIncluded true
#endif
#elif defined _M_ARM64
#ifndef cpuLibIncluded
#include "cpuinfoARM.h"	
#define cpuLibIncluded true
#endif
#else
#error "SturdyEngine does not support your archetechure, please compile for either x86-64 or for ARM"
#endif
namespace CPUInfo {
	class CPU {
	public:
		CPU() {
			if (!initialized) {
				initFlags(proc);
			}
		}
		ProcessorDescriptor& getDescriptor() {
			return proc;
		}
	private:
		ProcessorDescriptor proc;
	};
}
