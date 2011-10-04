#ifndef SAUCE_SAUCE_INJECTOR_H_
#define SAUCE_SAUCE_INJECTOR_H_

#include <sauce/exceptions.h>
#include <sauce/memory.h>
#include <sauce/internal/binding.h>

namespace sauce {

class Bindings;

class Injector {
  i::BindingMap bindingMap;

  friend class Bindings;

  Injector():
    bindingMap() {}

  Injector(i::BindingMap & bindingMap):
    bindingMap(bindingMap) {}

public:

  template<typename Iface>
  SAUCE_SHARED_PTR<Iface> get() {
    i::BindingMap::iterator i = bindingMap.find(i::BindKeyOf<Iface>());
    if (i == bindingMap.end()) {
      throw UnboundException();
    }

    i::Binding & binding = *(i->second.get());
    return binding.resolve<Iface>().get(*this);
  }

};

}

#endif // ifndef SAUCE_SAUCE_INJECTOR_H_