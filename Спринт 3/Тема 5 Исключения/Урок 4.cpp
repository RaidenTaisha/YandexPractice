#include <iostream>
#include <numeric>

using namespace std;

class Rational {
 public:
  Rational() = default;

  Rational(int value)
      : numerator_(value)
      , denominator_(1) {
  }

  Rational(int numerator, int denominator)
      : numerator_(numerator)
      , denominator_(denominator)
  {
    Normalize();
  }

  int Numerator() const {
    return numerator_;
  }

  int Denominator() const {
    return denominator_;
  }

  Rational& operator+=(Rational right) {
    numerator_ = numerator_ * right.denominator_ + right.numerator_ * denominator_;
    denominator_ *= right.denominator_;
    Normalize();
    return *this;
  }

  Rational& operator-=(Rational right) {
    numerator_ = numerator_ * right.denominator_ - right.numerator_ * denominator_;
    denominator_ *= right.denominator_;
    Normalize();
    return *this;
  }

  Rational& operator*=(Rational right) {
    numerator_ *= right.numerator_;
    denominator_ *= right.denominator_;
    Normalize();
    return *this;
  }

  Rational& operator/=(Rational right) {
    if (right.Numerator() == 0) {
      throw invalid_argument("Division by 0"s);
    }
    numerator_ *= right.denominator_;
    denominator_ *= right.numerator_;
    Normalize();
    return *this;
  }

 private:
  void Normalize() {
    if (denominator_ < 0) {
      numerator_ = -numerator_;
      denominator_ = -denominator_;
    }
    int n = gcd(numerator_, denominator_);
    numerator_ /= n;
    denominator_ /= n;
  }

  int numerator_ = 0;
  int denominator_ = 1;
};

ostream& operator<<(ostream& output, Rational rational) {
  return output << rational.Numerator() << '/' << rational.Denominator();
}

istream& operator>>(istream& input, Rational& rational) {
  int numerator;
  int denominator;
  char slash;
  if ((input >> numerator) && (input >> slash) && (slash == '/') && (input >> denominator)) {
    rational = Rational{numerator, denominator};
  }
  return input;
}

// Unary plus and minus

Rational operator+(Rational value) {
  return value;
}

Rational operator-(Rational value) {
  return {-value.Numerator(), value.Denominator()};
}

// Binary arithmetic operations

Rational operator+(Rational left, Rational right) {
  return left += right;
}

Rational operator-(Rational left, Rational right) {
  return left -= right;
}

Rational operator*(Rational left, Rational right) {
  return left *= right;
}

Rational operator/(Rational left, Rational right) {
  if (right.Numerator() == 0) {
    throw invalid_argument("Division by 0"s);
  }
  return left /= right;
}

// Comparison operators

bool operator==(Rational left, Rational right) {
  return left.Numerator() == right.Numerator() &&
      left.Denominator() == right.Denominator();
}

bool operator!=(Rational left, Rational right) {
  return !(left == right);
}

bool operator<(Rational left, Rational right) {
  return left.Numerator() * right.Denominator() <
      left.Denominator() * right.Numerator();
}

bool operator>(Rational left, Rational right) {
  return left.Numerator() * right.Denominator() >
      left.Denominator() * right.Numerator();
}

bool operator>=(Rational left, Rational right) {
  return !(left < right);
}

bool operator<=(Rational left, Rational right) {
  return !(left > right);
}