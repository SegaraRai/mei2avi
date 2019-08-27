#ifndef ML_FRACTION_HPP
#define ML_FRACTION_HPP

#include <cassert>
#include <numeric>


template<typename T>
struct Fraction {
private:
  static constexpr T DenominatorForZero = 1;

  void NormalizeZero() {
    if (numerator == 0) {
      denominator = DenominatorForZero;
    }
  }

public:
  using value_type = T;

  T numerator;
  T denominator;


  constexpr Fraction<T>(T numerator, T denominator) :
    numerator(numerator),
    denominator(denominator)
  {
    assert(denominator >= 1);
    Reduce();
  }


  constexpr Fraction<T>(T numerator = 0) :
    Fraction<T>(numerator, 1)
  {}


  constexpr Fraction<T>& Reduce() {
    if (numerator == 0) {
      denominator = DenominatorForZero;
      return *this;
    }

    const auto gcd = std::gcd(numerator, denominator);
    numerator /= gcd;
    denominator /= gcd;
    return *this;
  }

  static constexpr Fraction<T> Reduce(Fraction<T> fraction) {
    return fraction.Reduce();
  }


  constexpr Fraction<T> Inverse() const {
    return Fraction<T>(denominator, numerator);
  }

  static constexpr Fraction<T> Inverse(Fraction<T> fraction) {
    return fraction.Inverse();
  }


  constexpr Fraction<T>& operator+=(const Fraction<T>& rhs) {
    const auto gcd = std::gcd(denominator, rhs.denominator);
    numerator = numerator * (rhs.denominator / gcd) + rhs.numerator * (denominator / gcd);
    denominator *= rhs.denominator / gcd;
    NormalizeZero();
    return *this;
  }

  constexpr Fraction<T>& operator-=(const Fraction<T>& rhs) {
    const auto gcd = std::gcd(denominator, rhs.denominator);
    numerator = numerator * (rhs.denominator / gcd) - rhs.numerator * (denominator / gcd);
    denominator *= rhs.denominator / gcd;
    NormalizeZero();
    return *this;
  }

  constexpr Fraction<T>& operator*=(const Fraction<T>& rhs) {
    if (numerator == 0 || rhs.numerator == 0) {
      numerator = 0;
      denominator = DenominatorForZero;
      return *this;
    }
    const auto gcd1 = std::gcd(numerator, rhs.denominator);
    const auto gcd2 = std::gcd(denominator, rhs.numerator);
    numerator = (numerator / gcd1) * (rhs.numerator / gcd2);
    denominator = (denominator / gcd2) * (rhs.denominator / gcd1);
    return *this;
  }

  constexpr Fraction<T>& operator/=(const Fraction<T>& rhs) {
    return *this *= rhs.Inverse();;
  }


  friend constexpr Fraction<T> operator+(Fraction<T> lhs, const Fraction<T>& rhs) {
    lhs += rhs;
    return lhs;
  }

  friend constexpr Fraction<T> operator-(Fraction<T> lhs, const Fraction<T>& rhs) {
    lhs -= rhs;
    return lhs;
  }

  friend constexpr Fraction<T> operator*(Fraction<T> lhs, const Fraction<T>& rhs) {
    lhs *= rhs;
    return lhs;
  }

  friend constexpr Fraction<T> operator/(Fraction<T> lhs, const Fraction<T>& rhs) {
    lhs /= rhs;
    return lhs;
  }


  friend constexpr bool operator==(const Fraction<T>& lhs, const Fraction<T>& rhs) {
    const auto gcd = std::gcd(lhs.denominator, rhs.denominator);
    const auto ln = lhs.numerator * (rhs.denominator / gcd);
    const auto rn = rhs.numerator * (lhs.denominator / gcd);
    return ln == rn;
  }

  friend constexpr bool operator<(const Fraction<T>& lhs, const Fraction<T>& rhs) {
    const auto gcd = std::gcd(lhs.denominator, rhs.denominator);
    const auto ln = lhs.numerator * (rhs.denominator / gcd);
    const auto rn = rhs.numerator * (lhs.denominator / gcd);
    return ln < rn;
  }

  friend constexpr bool operator!=(const Fraction<T>& lhs, const Fraction<T>& rhs) {
    return !operator==(lhs, rhs);
  }

  friend constexpr bool operator>(const Fraction<T>& lhs, const Fraction<T>& rhs) {
    return operator<(rhs, lhs);
  }

  friend constexpr bool operator<=(const Fraction<T>& lhs, const Fraction<T>& rhs) {
    return !operator>(lhs, rhs);
  }

  friend constexpr bool operator>=(const Fraction<T>& lhs, const Fraction<T>& rhs) {
    return !operator<(lhs, rhs);
  }
};

#endif
