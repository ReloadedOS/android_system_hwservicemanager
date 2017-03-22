#define LOG_TAG "hwservicemanager"
#include "HidlService.h"

#include <android-base/logging.h>
#include <sstream>

namespace android {
namespace hidl {
namespace manager {
namespace V1_0 {
namespace implementation {

HidlService::HidlService(
    const std::string &interfaceName,
    const std::string &instanceName,
    const sp<IBase> &service,
    pid_t pid)
: mInterfaceName(interfaceName),
  mInstanceName(instanceName),
  mService(service),
  mPid(pid)
{}

sp<IBase> HidlService::getService() const {
    return mService;
}
void HidlService::setService(sp<IBase> service, pid_t pid) {
    mService = service;
    mPid = pid;

    sendRegistrationNotifications();
}

pid_t HidlService::getPid() const {
    return mPid;
}
const std::string &HidlService::getInterfaceName() const {
    return mInterfaceName;
}
const std::string &HidlService::getInstanceName() const {
    return mInstanceName;
}

void HidlService::addListener(const sp<IServiceNotification> &listener) {
    mListeners.push_back(listener);

    if (mService != nullptr) {
        auto ret = listener->onRegistration(
            mInterfaceName, mInstanceName, true /* preexisting */);
        ret.isOk(); // ignore
    }
}

void HidlService::registerPassthroughClient(pid_t pid) {
    mPassthroughClients.insert(pid);
}

const std::set<pid_t> &HidlService::getPassthroughClients() const {
    return mPassthroughClients;
}

std::string HidlService::string() const {
    std::stringstream ss;
    ss << mInterfaceName << "/" << mInstanceName;
    return ss.str();
}

void HidlService::sendRegistrationNotifications() {
    if (mListeners.size() == 0 || mService == nullptr) {
        return;
    }

    hidl_string iface = mInterfaceName;
    hidl_string name = mInstanceName;

    for (auto it = mListeners.begin(); it != mListeners.end();) {
        auto ret = (*it)->onRegistration(iface, name, false /* preexisting */);
        if (ret.isOk()) {
            ++it;
        } else {
            it = mListeners.erase(it);
        }
    }
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace manager
}  // namespace hidl
}  // namespace android
