#ifndef SAUCE_INTERNAL_INJECTOR_INJECTION_H_
#define SAUCE_INTERNAL_INJECTOR_INJECTION_H_

#include <sauce/injector.h>
#include <sauce/memory.h>
#include <sauce/internal/providing_injection.h>
#include <sauce/internal/key.h>

namespace sauce {
namespace internal {

/**
 * An injection that provides the injector itself.
 */
class InjectorInjection: public ProvidingInjection<Key<Injector>::Normalized, NoScope> {

  void validateAcyclic(InjectorPtr, TypeIds &) const {}

public:

  /**
   * Provide the injector.
   *
   * Just return the passed injector.
   */
  InjectorPtr provide(BindingPtr, InjectorPtr injector) const {
    return injector;
  }

};

}

namespace i = ::sauce::internal;

}

#endif // SAUCE_INTERNAL_INJECTOR_INJECTION_H_