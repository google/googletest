#include "cotest/cotest.h"

namespace testing {

EventHandle internal::Coroutine::NextEvent(const char *file, int line) {
    return EventHandle(crf->NextEvent(file, line));
}

bool internal::Coroutine::gmock_mutex_held = false;

EventHandle::EventHandle(std::shared_ptr<crf::InteriorEventSession> crf_es_) : crf_es(crf_es_) {}

EventHandle EventHandle::IsLaunchResult() const { return crf_es->IsLaunchResult() ? *this : EventHandle(); }

EventHandle EventHandle::IsObject(crf::UntypedMockObjectPointer object) {
    COTEST_ASSERT(crf_es);

    if (!crf_es->IsMockCall()) return EventHandle();
    auto crf_mcs = std::static_pointer_cast<crf::InteriorMockCallSession>(crf_es);

    // Check the mock object
    if (object != crf_mcs->GetMockObject()) return EventHandle();

    return *this;
}

EventHandle EventHandle::IsMockCall() {
    COTEST_ASSERT(crf_es);
    return crf_es->IsMockCall() ? *this : EventHandle();
}

EventHandle::operator bool() const { return !!crf_es; }

EventHandle EventHandle::Drop() {
    COTEST_ASSERT(crf_es);
    crf_es->Drop();
    return *this;
}

EventHandle EventHandle::Accept() {
    COTEST_ASSERT(crf_es);
    crf_es->Accept();
    return *this;
}

EventHandle EventHandle::Return() {
    COTEST_ASSERT(crf_es);
    crf_es->Return();
    return *this;
}

EventHandle EventHandle::FromMain() {
    COTEST_ASSERT(crf_es);
    bool ok = crf_es->IsFrom(nullptr);
    return ok ? *this : EventHandle();
}

std::string EventHandle::GetName() const {
    COTEST_ASSERT(crf_es && "call session is NULL, check for failed test");
    return static_cast<crf::InteriorMockCallSession *>(crf_es.get())->GetName();
}

}  // namespace testing
