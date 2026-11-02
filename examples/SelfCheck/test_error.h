// Two groups for SelfCheck, with codes that overlap on purpose.

#ifndef ERRORGROUP_EXAMPLES_SELFCHECK_TEST_ERROR_H_
#define ERRORGROUP_EXAMPLES_SELFCHECK_TEST_ERROR_H_

#include <error_group.h>

namespace self_check {

enum class AlphaError : uint8_t {
  kFirst,
  kSecond,
};

enum class BetaError : uint8_t {
  kFirst,
};

class AlphaErrorGroup : public error_group::ErrorGroup {
 public:
  constexpr AlphaErrorGroup() = default;

  const __FlashStringHelper* Name() const override;
  const __FlashStringHelper* Message(uint8_t code) const override;
};

class BetaErrorGroup : public error_group::ErrorGroup {
 public:
  constexpr BetaErrorGroup() = default;

  const __FlashStringHelper* Name() const override;
  const __FlashStringHelper* Message(uint8_t code) const override;
};

extern const AlphaErrorGroup kAlphaErrorGroup;
extern const BetaErrorGroup kBetaErrorGroup;

inline const error_group::ErrorGroup& GetErrorGroup(AlphaError) {
  return kAlphaErrorGroup;
}

inline const error_group::ErrorGroup& GetErrorGroup(BetaError) {
  return kBetaErrorGroup;
}

// Made in test_error.cpp, so that SelfCheck compares an error from another
// translation unit: equal only if both see the group at the same address.
error_group::Error MakeAlphaSecondElsewhere();

}  // namespace self_check

#endif  // ERRORGROUP_EXAMPLES_SELFCHECK_TEST_ERROR_H_
