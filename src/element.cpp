/** \file element.cpp
 *  \brief Element implementation file.
 */

#include <sstream>
#include <stdexcept>
#include "element.hpp"
#include "catch.hpp"

std::ostream& operator<<(std::ostream& os, const Element& e)
{
    return os << e.toString();
}

template<>
int IntElement::evaluate(const Valuation& valuation) const
{
    return t;
}

template<>
int VariableElement::evaluate(const Valuation& valuation) const
{
    auto it = valuation.find(t);
    if(it == valuation.end()) throw std::domain_error{"Undefined variable."};
    return it->second;
}

template<>
std::string IntElement::toString() const
{
    return std::to_string(t);
}

template<>
std::string VariableElement::toString() const
{
    return {t};
}

template<>
IntElement& IntElement::operator+=(const IntElement& tel)
{
    t += tel.t;
    return *this;
};

template<>
IntElement& IntElement::operator-=(const IntElement& tel)
{
    t -= tel.t;
    return *this;
};

template<>
IntElement& IntElement::operator*=(const IntElement& tel)
{
    t *= tel.t;
    return *this;
};

template<>
IntElement& IntElement::operator/=(const IntElement& tel)
{
    if(tel.t == 0)
    {
        throw std::invalid_argument("Divisor cannot be zero.");
    }
    t /= tel.t;
    return *this;
};

IntElement operator*(const IntElement& i1, const IntElement& i2)
{
    return IntElement{i1.t * i2.t};
}

IntElement operator+(const IntElement& i1, const IntElement& i2)
{
    return IntElement{i1.t + i2.t};
}

IntElement operator-(const IntElement& i1, const IntElement& i2)
{
    return IntElement{i1.t - i2.t};
}

IntElement operator/(const IntElement& i1, const IntElement& i2)
{
    if(i2.t == 0) throw std::invalid_argument{"Divisor cannot be zero."};
    return IntElement{i1.t / i2.t};
}

TEST_CASE("IntElement construction, mutators and accessors.",
          "[IntElement][constructor][get][set][mutator][accessor]")
{
    IntElement el9{9};
    IntElement def{ };
    IntElement elx{1};

    CHECK(def.getVal() == 0);
    CHECK(elx.getVal() == 1);

    def.setVal(5);
    CHECK(def.getVal() == 5);
    elx.setVal(-1);
    CHECK(elx.getVal() == -1);

    CHECK(elx.evaluate(Valuation{}) == -1);

    Valuation valA{{'A', 66}};
    CHECK(elx.evaluate(valA) == -1);

    CHECK(IntElement{el9}.getVal() == 9);

    auto el9copy1 = el9.clone();
    CHECK(el9copy1->toString() == el9.toString());
    CHECK(el9copy1->toString() == "9");

    IntElement* el9copy2 = static_cast<IntElement*>(el9copy1->clone());
    CHECK(el9copy2->getVal() == 9);

    delete el9copy1;
    delete el9copy2;
}

TEST_CASE("IntElement operators.", "[IntElement][operator][math]")
{
    IntElement elX{ };
    IntElement el1{1};
    IntElement el2{2};
    IntElement el3{3};
    IntElement el9{9};

    CHECK(el1 == IntElement{1});
    CHECK(el1 != el2);

    CHECK((el1 + el2).getVal() ==  3);
    CHECK((el1 - el3).getVal() == -2);
    CHECK((el9 * el2).getVal() == 18);
    CHECK((el1 / el2).getVal() ==  0);
    CHECK((el3 / el1).getVal() ==  3);
    CHECK_THROWS(el1 / 0);

    CHECK((elX += el2).getVal() ==  2);
    CHECK((elX -= el3).getVal() == -1);
    CHECK((elX *= el2).getVal() == -2);
    CHECK((elX /= el2).getVal() == -1);
    CHECK((elX /= el1).getVal() == -1);
    CHECK_THROWS(elX /= 0);

    std::stringstream ss;
    ss << el9;
    CHECK(ss.str() == "9");
}

TEST_CASE("VariableElement construction, mutators and accessors.",
          "[VariableElement][constructor][get][set][mutator][accessor]")
{
    VariableElement elA{'A'};
    VariableElement elG{'G'};
    VariableElement def{ };
    VariableElement elx{'x'};

    CHECK(def.getVal() == '\0');
    CHECK(elx.getVal() == 'x');

    def.setVal('h');
    CHECK(def.getVal() == 'h');

    CHECK_THROWS(elx.evaluate(Valuation{}));

    Valuation valA{{'A', 66}};
    CHECK(elA.evaluate(valA) == 66);

    CHECK(VariableElement{elA}.getVal() == 'A');

    auto elAcopy1 = elA.clone();
    CHECK(elAcopy1->toString() == elA.toString());
    CHECK(elAcopy1->toString() == "A");

    VariableElement* elAcopy2 = static_cast<VariableElement*>(elAcopy1->clone());
    CHECK(elAcopy2->getVal() == 'A');

    delete elAcopy1;
    delete elAcopy2;
}

TEST_CASE("VariableElement operators.", "[VariableElement][operator][math]")
{
    VariableElement elA{'A'};
    VariableElement elB{'B'};

    CHECK(elA == VariableElement{'A'});
    CHECK(elA != elB);

    std::stringstream ss;
    ss << elA;
    CHECK(ss.str() == "A");
}
