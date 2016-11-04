#ifndef TB_FUNCTORS_H
#define TB_FUNCTORS_H 1

#include <functional>
#include <numeric>
#define USE_BRANCH_PREDICTION 1

#if USE_BRANCH_PREDICTION
#define likely(x) __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)
#else
#define likely(x) x
#define unlikely(x) x
#endif

namespace TbFunctors {

template <class TYPE1, class TYPE2 = TYPE1>
class LessByTime : public std::binary_function<TYPE1, TYPE2, bool> {
 public:
  /** Compare the time stamps of two objects
   *  @param o1   first object
   *  @param o2   second object
   *  @return  result of the comparison
   */
  inline bool operator()(TYPE1 o1, TYPE2 o2) const {
    return !o1 ? true : !o2 ? false : o1->time() < o2->time();
  }
};

template <class TYPE1, class TYPE2 = TYPE1>
class LessByTimeBP : public std::binary_function<TYPE1, TYPE2, bool> {
 public:
  /** Compare the time stamps of two objects
  *  @param o1   first object
  *  @param o2   second object
  *  @return  result of the comparison
  *  Uses branch prediction for faster sorts in partially sorted data
  */
  inline bool operator()(TYPE1 o1, TYPE2 o2) const {
    return unlikely(!o1) ? true : unlikely(!o2)
                                      ? false
                                      : likely(o1->time() < o2->time());
  }
};

template <class TYPE1, class TYPE2 = TYPE1>
class GreaterByToT : public std::binary_function<TYPE1, TYPE2, bool> {
 public:
  /** Compare the ADC values of two objects
   *  @param o1   first object
   *  @param o2   second object
   *  @return  result of the comparison
   */
  inline bool operator()(TYPE1 o1, TYPE2 o2) const {
    return (!o2) ? true : (!o1) ? false : o1->ToT() > o2->ToT();
  }
};

template <class TYPE1, class TYPE2 = TYPE1>
class LessByZ : public std::binary_function<TYPE1, TYPE2, bool> {
 public:
  /** Compare the z-coordinates of two objects
   *  @param o1   first object
   *  @param o2   second object
   *  @return  result of the comparison
   */
  inline bool operator()(TYPE1 o1, TYPE2 o2) const {
    return (!o1) ? true : (!o2) ? false : o1->z() < o2->z();
  }
};
}

template <class TYPE>
class lowerBound {
 public:
  bool operator()(TYPE lhs, const double t) const { return lhs->htime() < t; }
  bool operator()(const double t, TYPE rhs) const { return t < rhs->htime(); }
};

#endif
