/** \file squarematrix.cpp
 *  \brief SquareMatrix implementation file.
 */

#include <future>
#include <functional>
#include <cmath>
#include "squarematrix.hpp"
#include "catch.hpp"

template<>
void ElementarySquareMatrix<IntElement>::t_oper(const ConcreteSquareMatrix& rhs,
    std::function<IntElement(IntElement&, IntElement&)> func)
{
    std::vector<std::thread> v_threads;
    const unsigned int m_size = pow(this->getRowSize(), 2.0);
    unsigned int blocksz =
        static_cast<unsigned int>(
            ceil(static_cast<double>(m_size) /
                 static_cast<double>(n_threads)));
    unsigned int blockct = 0;
    if(blocksz == 0)
    {
        blockct = 0;
    }
    else
    {
        blockct = ceil(m_size / blocksz);
    }
    std::atomic_int turn{0};
    std::atomic_int ends{static_cast<int>(blockct)};
    std::vector<std::vector<std::shared_ptr<IntElement>>> temp;

    for(unsigned int i = 0; i < blockct; i++)
    {
        v_threads.push_back(std::thread{[&turn, this, &rhs, &blocksz, &func, &ends]()
        {
            const unsigned int myturn = turn++;

            auto lhs_fut = std::async(
                std::launch::async, &ElementarySquareMatrix<IntElement>::block,
                    this, (blocksz * myturn), blocksz);
            auto rhs_fut = std::async(
                std::launch::async, &ElementarySquareMatrix<IntElement>::block,
                    rhs, (blocksz * myturn), blocksz);

            auto block_lhs = lhs_fut.get();
            auto block_rhs = rhs_fut.get();

            std::vector<std::shared_ptr<IntElement>> results;
            auto iter_rhs = block_rhs.cbegin();
            int k = 0;
            for(auto& el_lhs : block_lhs)
            {
                std::shared_ptr<IntElement> result{
                    new IntElement{func(*el_lhs, **iter_rhs++)}};

                results.push_back(result);
                k++;
            }

            ends--;

            // Wait until all other threads
            // are done before writing to original matrix.
            while(true)
            {
                mtx.lock();
                if(ends == 0)
                {
                    mtx.unlock();
                    break;
                }
                mtx.unlock();
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }

            // Write result back to original matrix.
            mtx.lock();
            auto iter = block_lhs.begin();
            for(auto& res : results)
            {
                std::swap(**iter++, *res);
            }
            mtx.unlock();
        }});
    }

    for(auto&& t : v_threads)
    {
        t.join();
    }
}

std::ostream& operator<<(std::ostream& os, const SquareMatrix& e)
{
    return os << e.toString();
}

template<>
ElementarySquareMatrix<IntElement>::ElementarySquareMatrix(int m) : n(m)
{
    std::vector<std::vector<std::shared_ptr<IntElement>>> temp;

    auto seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    std::uniform_int_distribution<int> distribution(-99, 99);

    for(int n = 0; n < m; n++)
    {
        std::vector<std::shared_ptr<IntElement>> row;

        for(int j = 0; j < m; j++)
        {
            row.push_back(std::shared_ptr<IntElement>{
                new IntElement{distribution(generator)}});
        }

        temp.push_back(std::move(row));
    }

    std::swap(elements, temp);
}

template<>
ElementarySquareMatrix<IntElement>::ElementarySquareMatrix(
    const std::string& str_m) : n(0)
{
    std::stringstream ss;
    char c = 0;
    int a;
    bool looped = false;

    ss << str_m;
    ss >> std::skipws >> c;
    if(c != '[' || !ss.good())
    {
        throw std::invalid_argument("Invalid string (first bracket).");
    }

    ss >> std::skipws >> c;
    while(c != ']' && ss.good())
    {
        if(!looped)
        {
            looped = true;
        }

        if(c != '[' || !ss.good())
        {
            throw std::invalid_argument("Invalid string.");
        }

        std::vector<std::shared_ptr<IntElement>> row;
        do
        {
            ss >> std::skipws >> a >> c;
            if(!ss.good())
            {
                throw std::invalid_argument("Invalid string.");
            }
            row.push_back(std::shared_ptr<IntElement>{new IntElement{a}});
        }
        while(c == ',');

        if(c != ']') throw std::invalid_argument("Invalid string.");

        if(elements.size() == 0)
        {
            n = row.size();
        }
        else if(row.size() != n)
        {
            throw std::invalid_argument("Invalid string (row size).");
        }
        elements.push_back(std::move(row));
        ss >> std::skipws >> c;
    }

    if(!looped)
    {
        throw std::invalid_argument("Invalid string.");
    }
    if(elements.size() != n)
    {
        throw std::invalid_argument("Invalid string (elements size).");
    }
    if(!ss.good())
    {
        throw std::invalid_argument("Invalid string (bad stream).");
    }
    if(str_m.back() != ']')
    {
        throw std::invalid_argument("Invalid string (last bracket).");
    }

    /* Check if there is still stuff in the stream. */
    ss >> std::skipws >> c;
    if(ss.good())
    {
        throw std::invalid_argument("Invalid string.");
    }
}

template<>
ElementarySquareMatrix<Element>::ElementarySquareMatrix(
    const std::string& str_m) : n(0)
{
    std::stringstream ss;
    char c = 0;
    int a = 0;
    bool looped = false;

    ss << str_m;
    ss >> std::skipws >> c;
    if(c != '[' || !ss.good())
    {
        throw std::invalid_argument("Invalid string (first bracket).");
    }

    ss >> std::skipws >> c;
    while(c != ']' && ss.good())
    {
        if(!looped)
        {
            looped = true;
        }

        if(c != '[' || !ss.good())
        {
            throw std::invalid_argument("Invalid string.");
        }

        std::vector<std::shared_ptr<Element>> row;
        do
        {
            ss >> std::skipws >> a;
            if(!ss.good())
            {
                ss.clear();
                ss >> c;
                if(!ss.good() || !isalpha(c))
                {
                    throw std::invalid_argument("Invalid string.");
                }
                row.push_back(std::shared_ptr<Element>{new VariableElement{c}});
            }
            else
            {
                row.push_back(std::shared_ptr<Element>{new IntElement{a}});
            }
            ss >> std::skipws >> c;
        }
        while(c == ',');

        if(c != ']') throw std::invalid_argument("Invalid string.");

        if(elements.size() == 0)
        {
            n = row.size();
        }
        else if(row.size() != n)
        {
            throw std::invalid_argument("Invalid string.");
        }
        elements.push_back(std::move(row));
        ss >> std::skipws >> c;
    }

    if(!looped)
    {
        throw std::invalid_argument("Invalid string.");
    }
    if(elements.size() != n)
    {
        throw std::invalid_argument("Invalid string (elements size).");
    }
    if(!ss.good())
    {
        throw std::invalid_argument("Invalid string (bad stream).");
    }

    /* Check if there is still stuff in the stream. */
    ss >> std::skipws >> c;
    if(ss.good())
    {
        throw std::invalid_argument("Invalid string.");
    }
}

template<>
ConcreteSquareMatrix
    SymbolicSquareMatrix::evaluate(const Valuation& val) const
{
    std::vector<std::vector<std::shared_ptr<IntElement>>> temp;

    for(const auto& row : elements)
    {
        std::vector<std::shared_ptr<IntElement>> newrow;

        for(const auto& e : row)
        {
            newrow.push_back(std::shared_ptr<IntElement>{
                new IntElement{e->evaluate(val)}});
        }

        temp.push_back(std::move(newrow));
    }

    return ConcreteSquareMatrix{n, std::move(temp)};
}

template<>
ConcreteSquareMatrix
    ConcreteSquareMatrix::evaluate(const Valuation& val) const
{
    return ConcreteSquareMatrix{*this};
}

template<>
bool ConcreteSquareMatrix::operator==(const ConcreteSquareMatrix& m) const
{
    if(m.n != n) return false;

    auto rowit = m.elements.cbegin();

    for(const auto& row : elements)
    {
        auto eit = rowit++->cbegin();

        bool equals = std::all_of(row.cbegin(), row.cend(),
            [&eit](const std::shared_ptr<IntElement>& ep1)
            {return *ep1 == **eit++;});

        if(!equals) return false;
    }

    return true;
}

template<>
bool SymbolicSquareMatrix::operator==(const SymbolicSquareMatrix& m) const
{
    if(m.n != n) return false;

    auto rowit = m.elements.cbegin();

    for(const auto& row : elements)
    {
        auto eit = rowit++->cbegin();

        bool equals = std::all_of(row.cbegin(), row.cend(),
            [&eit](const std::shared_ptr<Element>& ep1)
            {return ep1->toString() == (**eit++).toString();});

        if(!equals) return false;
    }

    return true;
}

template<>
ConcreteSquareMatrix&
    ConcreteSquareMatrix::operator+=(const ConcreteSquareMatrix& m)
{
    if(n != m.n) throw std::invalid_argument("Dimension mismatch.");

    this->t_oper(m, [](IntElement& e1, IntElement& e2)
    {
        return IntElement{e1 + e2};
    });

    return *this;
}

template<>
ConcreteSquareMatrix&
    ConcreteSquareMatrix::operator-=(const ConcreteSquareMatrix& m)
{
    if(n != m.n) throw std::invalid_argument("Dimension mismatch.");

    this->t_oper(m, [](IntElement& e1, IntElement& e2)
    {
        return IntElement{e1 - e2};
    });

    return *this;
}

template<>
ConcreteSquareMatrix&
    ConcreteSquareMatrix::operator*=(const ConcreteSquareMatrix& m)
{
    if(n != m.n) throw std::invalid_argument("Dimension mismatch");

    std::vector<std::vector<std::shared_ptr<IntElement>>> prodv;

	ConcreteSquareMatrix tm = m.transpose();

	for(auto& row : elements)
	{
        prodv.push_back(std::vector<std::shared_ptr<IntElement>>{});
        for(auto& col : tm.elements)
        {
            std::vector<IntElement> iev(n);

            std::transform(row.begin(), row.end(), col.begin(), iev.begin(),
                [](std::shared_ptr<IntElement>& ep1,
                   std::shared_ptr<IntElement>& ep2)
                    {return IntElement{*ep1 * *ep2};});

            IntElement sum;
            for(auto& e: iev) sum += e;

            prodv.back().push_back(std::shared_ptr<IntElement>{
                new IntElement{sum}});
        }
	}

	std::swap(elements, prodv);
	return *this;
}

template<>
ConcreteSquareMatrix&
    ConcreteSquareMatrix::operator/=(const ConcreteSquareMatrix& m)
{
    *this *= m.transpose();
    return *this;
}

ConcreteSquareMatrix operator+(const ConcreteSquareMatrix& m1,
                               const ConcreteSquareMatrix& m2)
{
    ConcreteSquareMatrix ret{m1};
    ret += m2;
    return ret;
}

ConcreteSquareMatrix operator-(const ConcreteSquareMatrix& m1,
                               const ConcreteSquareMatrix& m2)
{
    ConcreteSquareMatrix ret{m1};
    ret -= m2;
    return ret;
}
ConcreteSquareMatrix operator*(const ConcreteSquareMatrix& m1,
                               const ConcreteSquareMatrix& m2)
{
    ConcreteSquareMatrix ret{m1};
    ret *= m2;
    return ret;
}
ConcreteSquareMatrix operator/(const ConcreteSquareMatrix& m1,
                               const ConcreteSquareMatrix& m2)
{
    ConcreteSquareMatrix ret{m1};
    ret /= m2;
    return ret;
}

TEST_CASE("Matrix blocks.", "[block][matrix][exception]")
{
    ConcreteSquareMatrix conc{"[[1,2][3,4]]"};
    SymbolicSquareMatrix symb{"[[a,b][c,d]]"};
    SymbolicSquareMatrix sym2{"[[a,1][2,d]]"};

    auto conc_block1 = conc.block(0, 3);
    auto conc_block2 = conc.block(2, 0);
    auto symb_block1 = symb.block(0, 2);
    auto symb_block2 = symb.block(3, 1);
    auto symb_block3 = sym2.block(3, 1);
    auto symb_block4 = sym2.block(0, 4);

    CHECK(*conc_block1.at(0) == IntElement{1});
    CHECK(*conc_block1.at(1) == IntElement{2});
    CHECK(*conc_block1.at(2) == IntElement{3});

    CHECK(symb_block1.at(0)->toString() == VariableElement{'a'}.toString());
    CHECK(symb_block1.at(1)->toString() == VariableElement{'b'}.toString());

    CHECK(symb_block2.at(0)->toString() == VariableElement{'d'}.toString());

    CHECK(symb_block3.at(0)->toString() == VariableElement{'d'}.toString());

    CHECK(symb_block4.at(0)->toString() == VariableElement{'a'}.toString());
    CHECK(symb_block4.at(1)->toString() == IntElement{1}.toString());
    CHECK(symb_block4.at(2)->toString() == IntElement{2}.toString());
    CHECK(symb_block4.at(3)->toString() == VariableElement{'d'}.toString());

    CHECK(conc_block1.size() == 3);
    CHECK(conc_block2.size() == 0);
    CHECK(symb_block1.size() == 2);
    CHECK(symb_block2.size() == 1);
    CHECK(symb_block3.size() == 1);
    CHECK(symb_block4.size() == 4);

    ConcreteSquareMatrix m{"[[1,2,3][4,5,6][7,8,9]]"};
    auto mblock = m.block(7, 2);
    CHECK(*mblock.at(0) == IntElement{8});
    CHECK(*mblock.at(1) == IntElement{9});

    ConcreteSquareMatrix m2{"[[1,2,3,4][5,6,7,8][9,10,11,12][13,14,15,16]]"};
    auto m2block1 = m2.block(0, 16);
    CHECK(*m2block1.at(0) == IntElement{1});
    CHECK(*m2block1.at(1) == IntElement{2});
    CHECK(*m2block1.at(2) == IntElement{3});
    CHECK(*m2block1.at(3) == IntElement{4});
    CHECK(*m2block1.at(4) == IntElement{5});
    CHECK(*m2block1.at(14) == IntElement{15});
    CHECK(*m2block1.at(15) == IntElement{16});

    CHECK_THROWS(conc.block(-1,  0));
    CHECK_THROWS(conc.block( 1,  5));
    CHECK_THROWS(conc.block(-1,  0));
    CHECK_THROWS(conc.block( 1,  5));
    CHECK_THROWS(conc.block( 6,  0));
    CHECK_THROWS(conc.block(-1, -1));
    CHECK_THROWS(conc.block( 0,  8));

    CHECK_THROWS(symb.block(-1,  0));
    CHECK_THROWS(symb.block( 1,  5));
    CHECK_THROWS(symb.block( 6,  0));
    CHECK_THROWS(symb.block(-1, -1));
    CHECK_THROWS(symb.block( 0,  8));

    CHECK_THROWS(conc.block( 6,  0));
    CHECK_THROWS(conc.block(-1, -1));
    CHECK_THROWS(conc.block( 0,  8));

    CHECK_THROWS(symb.block(-1,  0));
    CHECK_THROWS(symb.block( 1,  5));
    CHECK_THROWS(symb.block( 6,  0));
    CHECK_THROWS(symb.block(-1, -1));
    CHECK_THROWS(symb.block( 0,  8));
};

TEST_CASE("ConcreteSquareMatrix constructors, mutation and operators.",
          "[ConcreteSquareMatrix][constructor][assignment][mutator][math][op]")
{
    std::vector<std::vector<std::shared_ptr<IntElement>>> temp1{};
    int k = 1;
    for(int i = 0; i < 3; i++)
    {
        std::vector<std::shared_ptr<IntElement>> row;

        for(int j = 0; j < 3; j++)
        {
            row.push_back(std::shared_ptr<IntElement>{new IntElement{k++}});
        }

        temp1.push_back(std::move(row));
    }

    std::stringstream ss;
    ConcreteSquareMatrix empty_m{};
    ConcreteSquareMatrix matrix_123456789{3, std::move(temp1)};
    ConcreteSquareMatrix copym{matrix_123456789};
    ConcreteSquareMatrix copym2 = copym;
    ConcreteSquareMatrix copymX{};
    ConcreteSquareMatrix copym3{*static_cast<ConcreteSquareMatrix*>(copym.clone())};
    ConcreteSquareMatrix matrix_147258369{matrix_123456789.transpose()};

    copymX = copym;
    CHECK(copymX == copym);

    CHECK(copym == matrix_123456789);
    CHECK(copym2 == copym);
    CHECK(copym3 == copym);

    Valuation dummy = {{'x', 100}};
    CHECK(empty_m.evaluate(dummy) == ConcreteSquareMatrix{});
    CHECK(copym.evaluate(dummy) == copym3);

    CHECK(matrix_123456789.toString() == "[[1,2,3][4,5,6][7,8,9]]");
    CHECK(matrix_147258369.toString() == "[[1,4,7][2,5,8][3,6,9]]");

    CHECK(matrix_123456789 != matrix_147258369);

    CHECK((matrix_123456789 - matrix_123456789).toString() ==
        "[[0,0,0][0,0,0][0,0,0]]");
    CHECK((matrix_123456789 + matrix_123456789).toString() ==
        "[[2,4,6][8,10,12][14,16,18]]");
    CHECK((matrix_123456789 * matrix_123456789).toString() ==
        "[[30,36,42][66,81,96][102,126,150]]");
    CHECK((matrix_123456789 / matrix_123456789).toString() ==
        "[[14,32,50][32,77,122][50,122,194]]");

    CHECK(empty_m.toString() == "[]");
    empty_m.print(ss);
    CHECK(ss.str() == "[]");

    ConcreteSquareMatrix matrix1{"[[1,2][3,4]]"};
    ConcreteSquareMatrix matrix2{"[[2,2][2,2]]"};

    matrix1 += matrix2;
    CHECK(matrix1.toString() == "[[3,4][5,6]]");
    matrix2 -= matrix1;
    CHECK(matrix2.toString() == "[[-1,-2][-3,-4]]");
    matrix1 *= matrix2;
    CHECK(matrix1.toString() == "[[-15,-22][-23,-34]]");
    matrix1 /= matrix2;
    CHECK(matrix1.toString() == "[[59,133][91,205]]");

    empty_m = std::move(matrix1);
    CHECK(empty_m.toString() == "[[59,133][91,205]]");

    ConcreteSquareMatrix movem{std::move(copym)};
    CHECK(movem.toString() == "[[1,2,3][4,5,6][7,8,9]]");
    CHECK(copym == ConcreteSquareMatrix{});

    ConcreteSquareMatrix rand_m{4};
    ConcreteSquareMatrix mmm{rand_m.toString()};
    CHECK(rand_m.toString() == mmm.toString());
    CHECK(rand_m == mmm);

    CHECK(matrix2.getRowSize() == 2);

    ConcreteSquareMatrix mmm1{"[[1,2,3][4,5,6][7,8,9]]"};
    ConcreteSquareMatrix mmm2{"[[1,2,3][4,5,6][7,8,9]]"};
    ConcreteSquareMatrix mmm3 = mmm1 + mmm2;
    CHECK(mmm3.toString() == "[[2,4,6][8,10,12][14,16,18]]");

    ConcreteSquareMatrix mmmm1{"[[1,2,3,4][5,6,7,8][9,10,11,12][13,14,15,16]]"};
    ConcreteSquareMatrix mmmm2{"[[1,2,3,4][5,6,7,8][9,10,11,12][13,14,15,16]]"};
    ConcreteSquareMatrix mmmm3;
    mmmm3 = mmmm1 + mmmm2;
    CHECK(mmmm3.toString() == "[[2,4,6,8][10,12,14,16][18,20,22,24][26,28,30,32]]");

    CHECK_THROWS(matrix2 + matrix_123456789);
    CHECK_THROWS(matrix2 - matrix_123456789);
    CHECK_THROWS(matrix2 * matrix_123456789);
    CHECK_THROWS(matrix2 / matrix_123456789);
}

TEST_CASE("ConcreteSquareMatrix errors.",
          "[ConcreteSquareMatrix][error][exception]")
{
    std::vector<std::shared_ptr<IntElement>> temp1;
    temp1.push_back(std::move(std::shared_ptr<IntElement>{new IntElement{0}}));
    std::vector<std::shared_ptr<IntElement>> temp2;
    std::vector<std::shared_ptr<IntElement>> temp3;

    std::vector<std::vector<std::shared_ptr<IntElement>>> v{3};
    std::vector<std::vector<std::shared_ptr<IntElement>>> v2;

    v2.push_back(std::move(temp1));
    v2.push_back(std::move(temp2));
    v2.push_back(std::move(temp3));

    CHECK_THROWS(ConcreteSquareMatrix(9, std::move(v)));
    CHECK_THROWS(ConcreteSquareMatrix(3, std::move(v)));
    CHECK_THROWS(ConcreteSquareMatrix(3, std::move(v2)));
    CHECK_THROWS(ConcreteSquareMatrix{""});
    CHECK_THROWS(ConcreteSquareMatrix{"]"});
    CHECK_THROWS(ConcreteSquareMatrix{"["});
    CHECK_THROWS(ConcreteSquareMatrix{"[]"});
    CHECK_THROWS(ConcreteSquareMatrix{"[][]"});
    CHECK_THROWS(ConcreteSquareMatrix{"[[1]][]"});
    CHECK_THROWS(ConcreteSquareMatrix{"[]1[]"});
    CHECK_THROWS(ConcreteSquareMatrix{"[1][]"});
    CHECK_THROWS(ConcreteSquareMatrix{"[["});
    CHECK_THROWS(ConcreteSquareMatrix{"[[1]"});
    CHECK_THROWS(ConcreteSquareMatrix{"[[4]["});
    CHECK_THROWS(ConcreteSquareMatrix{"[[a]["});
    CHECK_THROWS(ConcreteSquareMatrix{"[[1]]1"});
    CHECK_THROWS(ConcreteSquareMatrix{"[[1]1]"});
    CHECK_THROWS(ConcreteSquareMatrix{"[[1][1,1][1,1]"});
    CHECK_THROWS(ConcreteSquareMatrix{"[[1,2,3][4,5,6][7,8,9][10,11,12]]"});
    CHECK_THROWS(ConcreteSquareMatrix{"[[1,2,3],[4,5,6],[7,8,9]"});
    CHECK_THROWS(ConcreteSquareMatrix{"9"});
    CHECK_THROWS(ConcreteSquareMatrix{nullptr});
}

TEST_CASE("SymbolicSquareMatrix tests", "[SymbolicSquareMatrix]")
{
    std::stringstream ss;
    SymbolicSquareMatrix def{};
    SymbolicSquareMatrix ssm{};
    CHECK(def.toString() == "[]");
    def.print(ss);
    CHECK(ss.str() == "[]");

    ssm = def;
    CHECK(ssm == def);

    SymbolicSquareMatrix m1{"[[1,2,3][4,5,6][7,8,9]]"};
    SymbolicSquareMatrix m2{"[[a,b,c][4,5,6][d,e,f]]"};

    CHECK(m1.toString() == "[[1,2,3][4,5,6][7,8,9]]");
    CHECK(m2.toString() == "[[a,b,c][4,5,6][d,e,f]]");

    SymbolicSquareMatrix copym{m1};
    CHECK(m1 == copym);

    SymbolicSquareMatrix movem{std::move(m2)};
    CHECK(movem.toString() == "[[a,b,c][4,5,6][d,e,f]]");
    CHECK(m2 != copym);
    CHECK(m2 == SymbolicSquareMatrix{});

    SymbolicSquareMatrix m3{"[[a,b][c,d]]"};
    SymbolicSquareMatrix m4{"[[1,b][3,h]]"};

    SymbolicSquareMatrix copym3 = m3;
    CHECK(copym3 == m3);
    SymbolicSquareMatrix movem4 = std::move(m4);
    CHECK(movem4 != m4);
    CHECK(m4 == SymbolicSquareMatrix{});
    CHECK(movem4.toString() == "[[1,b][3,h]]");

    SymbolicSquareMatrix t_m4{movem4.transpose()};
    CHECK(t_m4.toString() == "[[1,3][b,h]]");

    Valuation val = {{'b', 2}, {'h', 4}};
    ConcreteSquareMatrix evaluated = t_m4.evaluate(val);
    CHECK(evaluated.toString() == "[[1,3][2,4]]");

    SymbolicSquareMatrix clone_copym3{
        *static_cast<SymbolicSquareMatrix*>(copym3.clone())};
    CHECK(clone_copym3 == copym3);
}

TEST_CASE("SymbolicSquareMatrix errors.",
          "[SymbolicSquareMatrix][error][exception]")
{
    CHECK_THROWS(SymbolicSquareMatrix{""});
    CHECK_THROWS(SymbolicSquareMatrix{"[]"});
    CHECK_THROWS(SymbolicSquareMatrix{"[][]"});
    CHECK_THROWS(SymbolicSquareMatrix{"[[1]][]"});
    CHECK_THROWS(SymbolicSquareMatrix{"[]1[]"});
    CHECK_THROWS(SymbolicSquareMatrix{"[1][]"});
    CHECK_THROWS(SymbolicSquareMatrix{"]"});
    CHECK_THROWS(SymbolicSquareMatrix{"["});
    CHECK_THROWS(SymbolicSquareMatrix{"[["});
    CHECK_THROWS(SymbolicSquareMatrix{"[[1]"});
    CHECK_THROWS(SymbolicSquareMatrix{"[[4]["});
    CHECK_THROWS(SymbolicSquareMatrix{"[[1]]1"});
    CHECK_THROWS(SymbolicSquareMatrix{"[[1]1]"});
    CHECK_THROWS(SymbolicSquareMatrix{"[[1][1,1][1,1]"});
    CHECK_THROWS(SymbolicSquareMatrix{"[[1,2,3][4,5,6][7,8,9][10,11,12]]"});
    CHECK_THROWS(SymbolicSquareMatrix{"[[1,2,3],[4,5,6],[7,8,9]"});
    CHECK_THROWS(SymbolicSquareMatrix{"9"});
    CHECK_THROWS(SymbolicSquareMatrix{nullptr});
}

TEST_CASE("Large matrices.", "[matrix][large][stress][memory]")
{
    ConcreteSquareMatrix m1{150};
    ConcreteSquareMatrix m2{150};
    ConcreteSquareMatrix m3{150};
    ConcreteSquareMatrix m4{150};

    m1 += m2 += m3 += m4;
}
