#ifndef SAUCE_INTERNAL_IMPLICIT_BINDINGS_H_
#define SAUCE_INTERNAL_IMPLICIT_BINDINGS_H_

#include <sauce/exceptions.h>
#include <sauce/injector.h>
#include <sauce/memory.h>
#include <sauce/provider.h>
#include <sauce/internal/resolved_binding.h>
#include <sauce/internal/bindings.h>
#include <sauce/internal/injections/all.h>
#include <sauce/internal/key.h>

namespace sauce {
namespace internal {

struct ImplicitBindings;

typedef Bindings<ImplicitBindings> ConcreteBindings;

/**
 * Attempts to supply a Binding when the given Dependency is not found.
 */
template<typename Dependency>
struct ImplicitBinding {
  typedef sauce::shared_ptr<ResolvedBinding<Dependency> > BindingPtr;

  /**
   * Attempt to supply a (unknown) Binding at provision time.
   */
  static BindingPtr get(ConcreteBindings const &) {
    throw UnboundExceptionFor<Dependency>();
  }

};

/**
 * Attempts to supply a Binding when none is found for a dependency.
 */
struct ImplicitBindings {

  /**
   * Attempt to supply a (unknown) Binding at provision time.
   */
  template<typename Dependency>
  sauce::shared_ptr<ResolvedBinding<Dependency> > get(ConcreteBindings const & bindings) const {
    return ImplicitBinding<Dependency>::get(bindings);
  }

};

/**
 * The implicit Injector binding.
 */
template<>
struct ImplicitBinding<Named<Injector, Unnamed> > {
  typedef sauce::shared_ptr<ResolvedBinding<Named<Injector, Unnamed> > > BindingPtr;
  typedef sauce::internal::injections::InjectorInjection BoundInjection;
  typedef BoundInjection::InjectionPtr InjectionPtr;
  static BindingPtr get(ConcreteBindings const &) {
    InjectionPtr injection(new BoundInjection());
    BindingPtr binding = injection;
    return binding;
  }
};

/**
 * The implicit Provider binding for bound dependencies.
 */
template<typename Dependency, typename Name>
struct ImplicitBinding<Named<Provider<Dependency>, Name> > {
  typedef typename Key<Dependency>::Normalized Normalized;
  typedef Named<Provider<Dependency>, Name> ProviderDependency;
  typedef typename ResolvedBinding<ProviderDependency>::BindingPtr BindingPtr;
  typedef typename ResolvedBinding<Normalized>::BindingPtr ProvidedBindingPtr;
  typedef sauce::internal::injections::ImplicitProviderInjection<Dependency, Name> BoundInjection;
  typedef typename BoundInjection::InjectionPtr InjectionPtr;

  static BindingPtr get(ConcreteBindings const & bindings) {
    ProvidedBindingPtr providedBinding(bindings.getBinding<Normalized>());
    InjectionPtr injection(new BoundInjection(providedBinding));
    BindingPtr binding = injection;
    return binding;
  }
};

}

namespace i = ::sauce::internal;

}

#endif // SAUCE_INTERNAL_IMPLICIT_BINDINGS_H_
