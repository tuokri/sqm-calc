/** \file compositesquarematrix.cpp
 *  \brief CompositeSquareMatrix implementation file.
 */

#include <sstream>
#include "compositesquarematrix.hpp"
#include "catch.hpp"

CompositeSquareMatrix::CompositeSquareMatrix() :
    oprnd1(std::move(std::unique_ptr<SquareMatrix>{new ConcreteSquareMatrix{}})),
    oprnd2(std::move(std::unique_ptr<SquareMatrix>{new ConcreteSquareMatrix{}})),
    oprtor([](const ConcreteSquareMatrix& m1, const ConcreteSquareMatrix& m2){return m1 + m2;}), op_char('+') {}

CompositeSquareMatrix::CompositeSquareMatrix(
    const SquareMatrix& op1,
    const SquareMatrix& op2,
    const std::function<ConcreteSquareMatrix(
        const ConcreteSquareMatrix&,
        const ConcreteSquareMatrix&)>& opr,
    char opc) :
    oprnd1(std::move(std::unique_ptr<SquareMatrix>{op1.clone()})),
    oprnd2(std::move(std::unique_ptr<SquareMatrix>{op2.clone()})),
    oprtor(opr),
    op_char(opc) {}

CompositeSquareMatrix::CompositeSquareMatrix(const CompositeSquareMatrix& m) :
    oprnd1(std::move(std::unique_ptr<SquareMatrix>{m.oprnd1->clone()})),
    oprnd2(std::move(std::unique_ptr<SquareMatrix>{m.oprnd2->clone()})),
    oprtor(m.oprtor),
    op_char(m.op_char) {}

CompositeSquareMatrix::CompositeSquareMatrix(CompositeSquareMatrix&& m) :
    oprnd1(std::move(m.oprnd1)),
    oprnd2(std::move(m.oprnd2)),
    oprtor(std::move(m.oprtor)),
    op_char(std::move(m.op_char))
{
    m.oprnd1 = 0;
    m.oprnd1 = 0;
    m.oprtor = 0;
    m.op_char = 0;
}

unsigned int CompositeSquareMatrix::getRowSize() const
{
    return this->oprnd1->getRowSize();
}

CompositeSquareMatrix&
    CompositeSquareMatrix::operator=(const CompositeSquareMatrix& m)
{
    oprnd1 = std::move(std::unique_ptr<SquareMatrix>{m.oprnd1->clone()});
    oprnd2 = std::move(std::unique_ptr<SquareMatrix>{m.oprnd2->clone()});
    oprtor = m.oprtor;
    op_char = m.op_char;
    return *this;
}

CompositeSquareMatrix&
    CompositeSquareMatrix::operator=(CompositeSquareMatrix&& m)
{
    oprnd1 = std::move(m.oprnd1);
    oprnd2 = std::move(m.oprnd2);
    oprtor = std::move(m.oprtor);
    op_char = std::move(m.op_char);
    return *this;
}

SquareMatrix* CompositeSquareMatrix::clone() const
{
    return new CompositeSquareMatrix{*this};
}

void CompositeSquareMatrix::print(std::ostream& os) const
{
    os << this->toString();
}

std::string CompositeSquareMatrix::toString() const
{
    std::string str;

    str += "( ";
    str += oprnd1->toString();
    str += " ) ";
    str += op_char;
    str += " ( ";
    str += oprnd2->toString();
    str += " )";

    return str;
}

ConcreteSquareMatrix CompositeSquareMatrix::evaluate(const Valuation& val) const
{
    return oprtor(oprnd1->evaluate(val), oprnd2->evaluate(val));
}

TEST_CASE("CompositeSquareMatrix construction.",
          "[CompositeSquareMatrix][constructor]")
{
    std::stringstream ss;

    CompositeSquareMatrix csm{};
    CHECK(csm.toString() == "( [] ) + ( [] )");

    CompositeSquareMatrix csm2{};
    csm2 = csm;
    CHECK(csm2.toString() == csm.toString());

    Valuation val{};
    csm.evaluate(val);

    SymbolicSquareMatrix symbm{"[[a,b][4,5]]"};
    ConcreteSquareMatrix concm{"[[1,2][4,5]]"};
    CompositeSquareMatrix csm3{
        symbm,
        concm,
        [](const ConcreteSquareMatrix csm1, const ConcreteSquareMatrix csm2)
        {
            ConcreteSquareMatrix sum{csm1 + csm2};
            return sum;
        },
        '+'};

    CHECK(csm3.toString() == "( [[a,b][4,5]] ) + ( [[1,2][4,5]] )");
    csm3.print(ss);
    CHECK(ss.str() == "( [[a,b][4,5]] ) + ( [[1,2][4,5]] )");

    Valuation val1{{'a', 1}, {'b', 4}};

    ConcreteSquareMatrix valuated = csm3.evaluate(val1);
    ConcreteSquareMatrix valuated2{csm3.evaluate(val1)};
    CHECK(valuated.toString() == "[[2,6][8,10]]");
    CHECK(valuated == valuated2);

    CompositeSquareMatrix clone{
        *static_cast<CompositeSquareMatrix*>(csm3.clone())};

    CHECK(clone.toString() == csm3.toString());

    CompositeSquareMatrix clone2{};
    clone2 = std::move(clone);
    CHECK(clone2.toString() == csm3.toString());

    CompositeSquareMatrix clone3{std::move(csm3)};
    CHECK(clone2.toString() == clone3.toString());
}
