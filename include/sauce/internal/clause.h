#ifndef SAUCE_SAUCE_CLAUSE_H_
#define SAUCE_SAUCE_CLAUSE_H_

#include <sauce/internal/binding.h>

namespace sauce {
namespace internal {

typedef SAUCE_SHARED_PTR<Binding> BindingPointer;

/**
 * Base class for parts of the fluent binding API.
 */
template<typename Derived>
class Clause {
  bool act;

protected:

  // TODO: this wants to be private
  BindingMap & bindingMap;

  Clause(BindingMap & bindingMap):
    act(true),
    bindingMap(bindingMap) {}

  BindingMap & pass() {
    act = false;
    return bindingMap;
  }

public:

  virtual ~Clause() {
    if (act) {
      Derived::activate(bindingMap);
    }
  }
};

}

namespace i = ::sauce::internal;

}

#endif // ifndef SAUCE_SAUCE_CLAUSE_H_