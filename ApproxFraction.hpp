#ifndef ML_APPROX_FRACTION_HPP
#define ML_APPROX_FRACTION_HPP

#include <array>
#include <cstddef>
#include <cstdint>

#include "Fraction.hpp"


constexpr Fraction<std::uint_fast32_t> ApproxFraction(Fraction<std::uint_fast32_t> fraction) {
  constexpr std::array<Fraction<std::uint_fast32_t>, 2> PredefinedCandidates = {
    Fraction<std::uint_fast32_t>{1,     1},
    Fraction<std::uint_fast32_t>{30000, 1001},    // 29.97
  };

  struct Candidate {
    Fraction<std::uint_fast32_t> baseFraction;
    long multiply;
    double absError;
  };


  auto myAbs = [](double value) constexpr {
    return value >= 0. ? value : -value;
  };


  const double value = static_cast<double>(fraction.numerator) / fraction.denominator;

  // find the nearest one for each candidate
  std::array<Candidate, PredefinedCandidates.size()> candidates{};

  for (std::size_t i = 0; i < candidates.size(); i++) {
    candidates[i] = {
      PredefinedCandidates[i],
      0,
      1.e10,
    };
  }

  for (auto& candidate : candidates) {
    for (long multiply = 0; ; multiply++) {
      const auto currentValue = static_cast<double>(candidate.baseFraction.numerator) * multiply / candidate.baseFraction.denominator;
      const auto currentAbsError = myAbs(currentValue - value);
      if (currentAbsError > candidate.absError) {
        break;
      }
      candidate.absError = currentAbsError;
      candidate.multiply = multiply;
    }
  }

  const auto& approxCandidate = *std::min_element(candidates.cbegin(), candidates.cend(), [] (const Candidate& a, const Candidate& b) {
    return a.absError < b.absError;
  });

  return Fraction<std::uint_fast32_t>{
    approxCandidate.baseFraction.numerator * approxCandidate.multiply,
    approxCandidate.baseFraction.denominator,
  };
}


//static_assert(ApproxFraction(Fraction<std::uint_fast32_t>{211 * 1000, 8792}).numerator == 24);        // 24    (mov_sak.mei)
//static_assert(ApproxFraction(Fraction<std::uint_fast32_t>{577 * 1000, 19253}).numerator == 30000);    // 29.97 (mov_usi.mei)

#endif
