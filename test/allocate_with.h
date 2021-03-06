#ifndef SAUCE_TEST_ALLOCATE_WITH_H_
#define SAUCE_TEST_ALLOCATE_WITH_H_

namespace sauce {
namespace test {

/**
 * A template of tag types for mock allocators interacting with AllocateWith.
 */
template<typename C>
class A {
public:
  bool operator==(A<C>) const {
    return true;
  }
};

/**
 * A helper to use mock allocators with bindings.
 *
 * Usually an allocator is specified by type alone: an instance is not passed.
 * This is explicitly protected in the standard (requiring allocator instances
 * to be exchangable and so essentially stateless.)
 *
 * It also prevents us from using per-instance state, which is the way gmock
 * likes to do things.  As a work-around, the mocked allocator state is kept
 * in a common, static field (given by the template parameter.)  One can swap
 * it out with AllocateWith<Backing>::Base::setBacking, and get an allocator
 * delegating to it with AllocateWith<Backing>::Allocator.
 */
template<typename Backing>
class AllocateWith {
public:

  typedef Backing Backing_;

  /**
   * The untemplated allocator base class.
   *
   * It holds the shared, static pointer to a backing instance.
   */
  class Base {
  protected:
    static Backing_ * backing;
  public:
    static void setBacking(Backing_ & b) {
      backing = &b;
    }
  };

  template<class C>
  class Allocator: public Base {
  public:

    typedef size_t    size_type;
    typedef ptrdiff_t difference_type;
    typedef C *       pointer;
    typedef C const * const_pointer;
    typedef C &       reference;
    typedef C const & const_reference;
    typedef C         value_type;

    template<typename D>
    class rebind {
    public:
      typedef Allocator<D> other;
    };

    Allocator() {}

    Allocator(Allocator const &) {}

    template<typename D> Allocator(Allocator<D> const &) {}

    C * allocate(size_t size) {
      return Base::backing->allocate(A<C>(), size);
    }

    void deallocate(C * c, size_t size) {
      Base::backing->deallocate(c, size);
    }

  };

};

}
}

#endif // SAUCE_TEST_ALLOCATE_WITH_H_
