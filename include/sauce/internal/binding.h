#ifndef SAUCE_SAUCE_INTERNAL_BINDING_H_
#define SAUCE_SAUCE_INTERNAL_BINDING_H_

#include <map>
#include <set>
#include <vector>
#include <utility>

#include <cassert>

#include <sauce/exceptions.h>
#include <sauce/memory.h>
#include <sauce/named.h>
#include <sauce/internal/key.h>
#include <sauce/internal/type_id.h>

namespace sauce {

class Injector;

namespace internal {

/**
 * Detects circular dependencies on behalf of injectors.
 */
template<typename Dependency>
class CircularDependencyGuard {
  friend class ::sauce::Injector;

  TypeIds & ids;
  TypeId id;

  CircularDependencyGuard(TypeIds & ids):
    ids(ids),
    id(typeIdOf<Dependency>()) {
    TypeIds::iterator i = ids.find(id);
    if (i == ids.end()) {
      ids.insert(i, id);
    } else {
      throw CircularDependencyExceptionFor<Dependency>();
    }
  }

  ~CircularDependencyGuard() {
    ids.erase(id);
  }
};

template<typename Dependency>
class Binding;

/**
 * An opaque binding.
 *
 * Bindings associate an interface with an implementation.  How that provision
 * comes about is determine by derived types.  Binding itself is a pure
 * interface.
 *
 * To be type-homogenous, Binding is not a template, and particularly not a
 * template of any specific interface or implementation types.  It however has
 * a TypeId, which indirectly identifies the interface it is bound to.
 *
 * The interface is raised to the type system in Binding, a templated class
 * deriving from OpaqueBinding.  The implementation is in turn raised in
 * TransparentBinding, which derives from Binding.
 *
 * Other binding implementations (those that implement provision strategies)
 * extend TransparentBinding.  Their behavior is exposed to a Binding client
 * by (runtime) polymorphism.
 */
struct OpaqueBinding {

  /**
   * The TypeId of the (hidden) dependency this Binding provides.
   *
   * The dependency id finger prints which provision requests this Binding may satisfy in an
   * Injector.
   */
  virtual TypeId getDependencyId() = 0;

  /**
   * The TypeId of this Binding's (hidden) scope.
   */
  virtual TypeId getScopeId() = 0;

  /**
   * Resolve the interface actually bound.
   *
   * Note the caller is expected to understand which interface this is.  As
   * this is an internal type and Sauce's own callers obey this as an
   * invariant, requests for other types will immediately fail an assert.
   */
  template<typename Dependency>
  Binding<Dependency> & resolve() {
    assert((typeIdOf<Dependency>()) == getDependencyId());
    return *static_cast<Binding<Dependency> *>(this);
  }

  /**
   * Provide, but do not return an instance of the hidden interface.
   *
   * Instead, cache the instance in its appropriate scope, if any.  If the binding is not scoped,
   * do nothing.  The typeIds indicate which keys are already currently being provided: this is
   * used for circular dependency detection.
   */
  virtual void eagerlyProvide(Injector &, TypeIds &) {}

};

/**
 * A binding for a specific interface.
 */
template<typename Dependency>
struct Binding:
  public OpaqueBinding {

  /**
   * Provide an instance of Iface, using the given injector to resolve dependencies.
   *
   * The typeIds indicate which keys are already currently being provided: this is used for
   * circular dependency detection.
   */
  virtual typename Key<Dependency>::Ptr get(Injector & injector, TypeIds & typeIds) = 0;

};

typedef void (*PendingThrow)();

/**
 * Template function used to create typed, deferred exceptions.
 *
 * The exception must have an accessible nullary constructor.
 * Instances of this template will construct and throw an instance.
 */
template<typename Exception>
void pendingThrowFactory() {
  throw Exception();
}

typedef SAUCE_SHARED_PTR<OpaqueBinding> BindingPointer;

class Bindings {
  typedef std::map<TypeId, BindingPointer> BindingMap;
  typedef std::vector<BindingPointer> ScopedBindings;
  typedef std::map<TypeId, ScopedBindings> ScopeMap;

  BindingMap bindingMap;
  ScopeMap scopeMap;
  PendingThrow pending;

  ScopedBindings & bindingsInScope(TypeId scopeId) {
    // TODO: thread safety..
    ScopeMap::iterator i = scopeMap.find(scopeId);
    if (i == scopeMap.end()) {
      scopeMap.insert(i, std::make_pair(scopeId, ScopedBindings()));
      i = scopeMap.find(scopeId);
    }

    return i->second;
  }

public:

  Bindings():
    bindingMap(),
    scopeMap(),
    pending(NULL) {}

  /**
   * Insert the given binding.
   */
  template<typename Binding_>
  void put() {
    BindingPointer binding(new Binding_());
    bindingMap.insert(std::make_pair(binding->getDependencyId(), binding));
    bindingsInScope(binding->getScopeId()).push_back(binding);
  }

  /**
   * Get and resolve the binding for the named Dependency.
   *
   * If no binding is found, throw UnboundException.
   */
  template<typename Dependency>
  Binding<Dependency> & get() {
    std::map<TypeId, BindingPointer>::iterator i = bindingMap.find(typeIdOf<Dependency>());
    if (i == bindingMap.end()) {
      throw UnboundExceptionFor<Dependency>();
    }

    OpaqueBinding & binding = *(i->second.get());
    return binding.resolve<Dependency>();
  }

  template<typename Scope>
  void eagerlyProvide(Injector & injector, TypeIds & typeIds) {
    ScopedBindings & scopedBindings = bindingsInScope(typeIdOf<Scope>());
    for (ScopedBindings::iterator i = scopedBindings.begin(); i != scopedBindings.end(); ++i) {
      OpaqueBinding & binding = *(i->get());
      binding.eagerlyProvide(injector, typeIds);
    }
  }

  /**
   * Save an exception of the given type to throw when it is safe.
   *
   * The exception must have an accessible nullary constructor.
   *
   * Any previously saved exception is dropped.
   */
  template<typename Exception>
  void throwLater() {
    pending = &pendingThrowFactory<Exception>;
  }

  /**
   * Throw and clear any saved exception.
   */
  void throwAnyPending() {
    if (pending) {
      PendingThrow toThrow = pending;
      pending = NULL;
      toThrow();
    }
  }
};

}

namespace i = ::sauce::internal;

}

#endif // SAUCE_SAUCE_INTERNAL_BINDING_H_