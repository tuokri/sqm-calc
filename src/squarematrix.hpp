/** \file squarematrix.hpp
 *  \brief SquareMatrix header file.
 */

#ifndef SQUAREMATRIX_H
#define SQUAREMATRIX_H

#include <algorithm>
#include <vector>
#include <stdexcept>
#include <sstream>
#include <chrono>
#include <random>
#include <thread>
#include <mutex>
#include "element.hpp"
#include "valuation.hpp"

const unsigned int n_threads = std::thread::hardware_concurrency();
static std::mutex mtx;

/* Forward declaration. */
template <typename T>
class ElementarySquareMatrix;

using ConcreteSquareMatrix = ElementarySquareMatrix<IntElement>;
using SymbolicSquareMatrix = ElementarySquareMatrix<Element>;

/** \class SquareMatrix
 *  \brief Abstract base class for n*n square matrices.
 */
class SquareMatrix
{
    public:
        /** \brief Default destructor.
         */
        virtual ~SquareMatrix() = default;

        /** \brief Get row size of the matrix.
         *  \return Value of member n.
         */
        virtual unsigned int getRowSize() const = 0;

        /** \brief Returns pointer to a clone of this.
         *  \return SquareMatrix pointer.
         */
        virtual SquareMatrix* clone() const = 0;

        /** \brief Prints string representation to output stream.
         *  \param os std::ostream reference.
         */
        virtual void print(std::ostream& os) const = 0;

        /** \brief Returns string representation of the matrix.
         *  \return std::string.
         */
        virtual std::string toString() const = 0;

        /** \brief Evaluates SquareMatrix to a ConcreteSquareMatrix.
         *  \param val Valuation map.
         *  \return New instance of ConcreteSquareMatrix.
         */
        virtual ElementarySquareMatrix<IntElement>
            evaluate(const Valuation& val) const = 0;

        /** \brief Operator<< overload.
         *  \param os Reference to std::ostream.
         *  \param SquareMatrix reference.
         *  \return std::ostream reference.
         */
        friend std::ostream& operator<<(std::ostream& os, const SquareMatrix& e);
};

/** \class ElementarySquareMatrix<T>
 *  \brief Template class for specialized matrices.
 *  \tparam T Contained type.
 */
template <typename T>
class ElementarySquareMatrix : public SquareMatrix
{
    public:
        /** \brief Constructor with no parameters.
         */
        ElementarySquareMatrix() :
            n(0),
            elements(std::move(std::vector<std::vector<std::shared_ptr<T>>>{})) {};

        /** \brief Constructs a random matrix.
         *  \param m Row/column count for the random matrix.
         */
        ElementarySquareMatrix(int m);

        /** \brief Parametrized constructor.
         *         Constructs SquareMatrix from string
         *         of format "[[1,2][3,4]]".
         *
         *  \param str_m String to construct matrix from.
         */
        ElementarySquareMatrix(const std::string& str_m);

        /** \brief Parametrized constructor.
         *  \param new_n Value for n.
         *  \param new_elements Value for elements.
         */
        ElementarySquareMatrix(
            unsigned int new_n,
            std::vector<std::vector<std::shared_ptr<T>>> new_elements) :
                n(new_n),
                elements(std::move(new_elements))
        {
            if(n != elements.size())
            {
                throw std::invalid_argument("Not a squarematrix (invalid n).");
            }

            for(const auto& row : elements)
            {
                if(n != row.size())
                    throw std::invalid_argument("Not a squarematrix (invalid row).");
            }
        };

        /** \brief Copy constructor.
         *  \param Reference to ElementarySquareMatrix<T>.
         */
        ElementarySquareMatrix(const ElementarySquareMatrix<T>& m) :
            n(m.n), elements(std::vector<std::vector<std::shared_ptr<T>>>{})
        {
            for(const auto& row : m.elements)
            {
                std::vector<std::shared_ptr<T>> newrow;

                for(const auto& e : row)
                {
                    newrow.push_back(std::shared_ptr<T>{
                        static_cast<T*>(e->clone())});
                }

                elements.push_back(std::move(newrow));
            }
        };

        /** \brief Move constructor.
         *  \param Rvalue reference to ElementarySquareMatrix<T>.
         */
        ElementarySquareMatrix(ElementarySquareMatrix<T>&& m) :
            n(m.n),
            elements(std::move(m.elements))
        {
            m.n = 0;
            m.elements = std::vector<std::vector<std::shared_ptr<T>>>{};
        }

        /** \brief Default destructor.
         */
        virtual ~ElementarySquareMatrix() = default;

        /** \brief Get row size of the matrix.
         *  \return Value of member n.
         */
        unsigned int getRowSize() const override
        {
            return n;
        };

        /** \brief operator= overload (copy assignment).
         *  \param m Reference to ElementarySquareMatrix.
         *  \return Reference to ElementarySquareMatrix.
         */
        ElementarySquareMatrix<T>& operator=(const ElementarySquareMatrix<T>& m)
        {
            if(m == *this) return *this;

            n = m.n;
            elements.clear();

            for(const auto& row : m.elements)
            {
                std::vector<std::shared_ptr<T>> newrow;
                for(const auto& e : row)
                {
                    newrow.push_back(std::shared_ptr<T>{
                        static_cast<T*>(e->clone())});
                }

                elements.push_back(std::move(newrow));
            }

            return *this;
        };

        /** \brief operator= overload (move assignment).
         *  \param m Reference to ElementarySquareMatrix.
         *  \return Reference to ElementarySquareMatrix.
         */
        ElementarySquareMatrix<T>& operator=(ElementarySquareMatrix<T>&& m)
        {
            n = m.n;
            elements = std::move(m.elements);

            m.n = 0;
            m.elements = std::vector<std::vector<std::shared_ptr<T>>>{};

            return *this;
        };

        /** \brief Returns transpose of the matrix.
         *  \return Transposed ElementarySquareMatrix<T>.
         */
        ElementarySquareMatrix<T> transpose() const
        {
            ElementarySquareMatrix<T> temp{};
            temp.n = n;
            temp.elements = std::vector<std::vector<std::shared_ptr<T>>>{n};

            for(const auto& row : elements)
            {
                auto it = temp.elements.begin();
                for(auto& e : row)
                {
                    it++->push_back
                        (std::shared_ptr<T>{
                            static_cast<T*>(e->clone())});
                }
            }

            return temp;
        };

        /** \brief operator== overload.
         *  \return true if equal, else false.
         */
        bool operator==(const ElementarySquareMatrix<T>& m) const;

        /** \brief operator!= overload.
         *  \return true if not equal, else false.
         */
        bool operator!=(const ElementarySquareMatrix<T>& m) const
        {
            return !(*this == m);
        };

        /** \brief Returns pointer to a clone of this.
         *  \return SquareMatrix pointer.
         */
        SquareMatrix* clone() const override
        {
            return new ElementarySquareMatrix<T>{*this};
        };

        /** \brief Prints string representation to output stream.
         *  \param os std::ostream reference.
         */
        void print(std::ostream& os) const override
        {
            os << this->toString();
        }

        /** \brief Returns string representation of the matrix.
         *  \return std::string.
         */
        std::string toString() const override
        {
            std::string str("[");

            for(const auto& vec : elements)
            {
                str += "[";

                bool firstel = true;
                for(const auto& e : vec)
                {
                    if(!firstel) str += ",";
                    str += e->toString();
                    firstel = false;
                }

                str += "]";
            }

            str += "]";

            return str;
        }

        /** \brief Evaluates SquareMatrix to a ConcreteSquareMatrix.
         *  \param val Valuation map.
         *  \return Instance of ConcreteSquareMatrix.
         */
        ElementarySquareMatrix<IntElement>
            evaluate(const Valuation& val) const override;

        /** \brief Returns a copy of elements.
         *  \return std::vector.
         */
        std::vector<std::vector<std::shared_ptr<T>>>
            cloneElements() const;

        /** \brief operator+= overload.
         *  \param m Reference to ElementarySquareMatrix<T>.
         *  \return Reference to ElementarySquareMatrix<T>.
         */
        ElementarySquareMatrix<T>& operator+=(const ElementarySquareMatrix<T>& m);

        /** \brief operator-= overload.
         *  \param m Reference to ElementarySquareMatrix<T>.
         *  \return Reference to ElementarySquareMatrix<T>.
         */
        ElementarySquareMatrix<T>& operator-=(const ElementarySquareMatrix<T>& m);

        /** \brief operator*= overload.
         *  \param m Reference to ElementarySquareMatrix<T>.
         *  \return Reference to ElementarySquareMatrix<T>.
         */
        ElementarySquareMatrix<T>& operator*=(const ElementarySquareMatrix<T>& m);

        /** \brief operator/= overload.
         *  \param m Reference to ElementarySquareMatrix<T>.
         *  \return Reference to ElementarySquareMatrix<T>.
         */
        ElementarySquareMatrix<T>& operator/=(const ElementarySquareMatrix<T>& m);

        /** \brief operator+ overload.
         *  \param m1 Reference to ConcreteSquareMatrix..
         *  \param m2 Reference to ConcreteSquareMatrix.
         *  \return Instance of ConcreteSquareMatrix..
         */
        friend ConcreteSquareMatrix operator+(const ConcreteSquareMatrix& m1,
                                              const ConcreteSquareMatrix& m2);

        /** \brief operator- overload.
         *  \param m1 Reference to ConcreteSquareMatrix..
         *  \param m2 Reference to ConcreteSquareMatrix.
         *  \return Instance of ConcreteSquareMatrix..
         */
        friend ConcreteSquareMatrix operator-(const ConcreteSquareMatrix& m1,
                                              const ConcreteSquareMatrix& m2);

        /** \brief operator* overload.
         *  \param m1 Reference to ConcreteSquareMatrix..
         *  \param m2 Reference to ConcreteSquareMatrix.
         *  \return Instance of ConcreteSquareMatrix..
         */
        friend ConcreteSquareMatrix operator*(const ConcreteSquareMatrix& m1,
                                              const ConcreteSquareMatrix& m2);

        /** \brief operator/ overload.
         *  \param m1 Reference to ConcreteSquareMatrix..
         *  \param m2 Reference to ConcreteSquareMatrix.
         *  \return Instance of ConcreteSquareMatrix..
         */
        friend ConcreteSquareMatrix operator/(const ConcreteSquareMatrix& m1,
                                              const ConcreteSquareMatrix& m2);

        /** \brief Return a block of pointers from the matrix.
         *  \param start Index of first element.
         *  \param step How many elements to include.
         *  \return Vector of Element pointers.
         *
         *  Function considers indices as if the multi-dimensional
         *  matrix was unrolled to a single-dimensional vector.
         *
         *  \verbatim
         *  Example matrix:  |   Is considered as:
         *  [[2,3,4]         |
         *   [1,8,9]         |   [2,3,4,1,8,9,0,0,1]
         *   [0,0,1]]        |
         *  \endverbatim         *
         *  - Index 0 refers to first element at (0),(0) which is 2.
         *  - Index 5 refers to sixth element at (1),(2) which is 9.
         */
        std::vector<std::shared_ptr<T>> block(unsigned int start,
                                              unsigned int step) const
        {
            unsigned int m_size = pow(n, 2.0);

            if(start > m_size)
            {
                throw std::out_of_range("Start index exceeds elements size.");
            }
            if(step > m_size)
            {
                throw std::out_of_range("Step exceeds elements size.");
            }

            std::vector<std::shared_ptr<T>> ret;

            if(step == 0)
            {
                return ret;
            }

            std::vector<std::shared_ptr<T>> unrolled;
            for(auto& row : elements)
            {
                for(auto& e : row)
                {
                    unrolled.push_back(e);
                }
            }

            for(unsigned int i = start; i < (start + step); i++)
            {
                ret.push_back(unrolled.at(i));
            }

            return ret;
        };

        /** \brief Do multi-threaded operations.
         *  \param rhs Reference to ConcreteSquareMatrix.
         *  \param func Operation to perform on this and rhs.
         */
        void t_oper(const ConcreteSquareMatrix& rhs,
            std::function<IntElement(IntElement&, IntElement&)> func);

    private:
        unsigned int n;
        std::vector<std::vector<std::shared_ptr<T>>> elements;
};

#endif // SQUAREMATRIX_H
