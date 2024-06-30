#include <numeric>
#include <cmath>

using namespace std;

class Rational {
 public:
  Rational() {
    numerator_ = 0;
    denominator_ = 1;
  }

  Rational(int numerator) {
    numerator_ = numerator;
    denominator_ = 1;
  }

  Rational(int numerator, int denumenator) {
    if (numerator * denumenator < 0) {
      numerator_ = abs(numerator / gcd(numerator, denumenator))*-1;
      denominator_ = abs(denumenator / gcd(numerator, denumenator));
    } else {
      numerator_ = abs(numerator / gcd(numerator, denumenator));
      denominator_ = abs(denumenator / gcd(numerator, denumenator));
    }
  }

  int Numerator() const {
    return numerator_;
  }

  int Denominator() const {
    return denominator_;
  }

  void SetNumerator(int numerator) {
    numerator_ = numerator;
  }

  void SetDenominator(int denominator) {
    if (denominator != 0) {
      denominator_ = denominator;
    }
  }

 private:
  int numerator_;
  int denominator_;
};

int main ()
{
  Rational(1, 5);
  Rational(6, -8);
  Rational(-6, 8);
  Rational(6, 8);
  Rational(-6, -8);
}