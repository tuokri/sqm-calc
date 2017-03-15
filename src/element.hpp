/** \file element.hpp
 *  \brief Header file for Element.
 */

#ifndef ELEMENT_H
#define ELEMENT_H

#include <iostream>
#include <memory>
#include <string>
#include "valuation.hpp"

/** \class Element
 *  \brief Abstract base class for matrix elements.
 */
class Element
{
    public:
        /** \brief Default destructor.
         */
        virtual ~Element() = default;

        /** \brief Returns a pointer to a clone of this.
         *  \return Pointer to Element.
         */
        virtual Element* clone() const = 0;

        /** \brief Returns string representation of this.
         *  \return std::string.
         */
        virtual std::string toString() const = 0;

        /** \brief Evaluates the value of the Element to an integer.
         *  \param valuation Valuation map.
         *  \return evaluated integer value of the Element.
         *  \throw std::domain_error if variable is undefined.
         */
        virtual int evaluate(const Valuation& valuation) const = 0;

        /** \brief Operator<< overload.
         *  \param os Reference to std::ostream.
         *  \param Element reference.
         *  \return std::ostream reference.
         */
        friend std::ostream& operator<<(std::ostream& os, const Element& e);
};

/* Forward declaration, so that we can
 * define IntElement and VariableElement.
 */
template <typename T>
class TElement;

/* Define IntElement and VariableElement so
 * we can use them in TElement class code.
 */
using IntElement = TElement<int>;
using VariableElement = TElement<char>;

/** \class TElement
 *  \brief Template class for specialized matrices,
 *         notably IntElement and VariableElement.
 *  \tparam T Contained type.
 */
template <typename T>
class TElement : public Element
{
    public:
        /** \brief Constructor with no parameters.
         *         Initializes t to default value 0,
         *         for both integers and characters.
         */
        TElement() : t(0) {};

        /** \brief Parametrized constructor.
         *  \param new_t Reference to new value for t of type T.
         */
        TElement(const T& new_t) : t(new_t) {};

        /** \brief Default (copy) constructor.
         *  \param Reference to TElement<T>.
         */
        TElement(const TElement<T>&) = default;

        /** \brief Move constructor.
         *  \param Rvalue reference to TElement<T>.
         */
        TElement(const TElement<T>&& tel) : t(std::move(tel.t)) {};

        /** \brief Default destructor.
         */
        virtual ~TElement() = default;

        /** \brief t getter.
         *  \return Value of attribute t;
         */
        T getVal() const {return t;};

        /** \brief t setter.
         *  \param new_t New value for t;
         */
        void setVal(T new_t) {t = new_t;};

        /** \brief Returns a pointer to a clone of this.
         *  \return Pointer to Element.
         */
        Element* clone() const override
        {
            return new TElement<T>(*this);
        };

        /** \brief Returns string representation of this.
         *  \return std::string.
         */
        std::string toString() const override;

        /** \brief Evaluates the value of the Element to an integer.
         *  \param valuation Valuation map.
         *  \return evaluated integer value of the Element.
         *  \throw std::domain_error if variable is undefined.
         */
        int evaluate(const Valuation& valuation) const override;

        /** \brief operator= overload (copy assignment).
         *  \param tel Reference to TElement<T>.
         *  \return Reference to TElement<T>.
         */
        TElement<T>& operator=(const TElement<T>& tel)
        {
            t = tel.t;
            return *this;
        };

        /** \brief operator= overload (move assignment).
         *  \param tel Reference to TElement<T>.
         *  \return Reference to TElement<T>.
         */
        TElement<T>& operator=(TElement<T>&& tel)
        {
            t = tel.t;
            tel.t = 0;
            return *this;
        };

        /** \brief operator== overload.
         *  \param tel Reference to TElement<T>.
         *  \return true if equal, else false.
         */
        bool operator==(const TElement<T>& tel) const {return t == tel.t;};

        /** \brief operator!= overload.
         *  \param tel Reference to TElement<T>.
         *  \return false if equal, else true.
         */
        bool operator!=(const TElement<T>& tel) const {return t != tel.t;};

        /** \brief operator+= overload.
         *  \param i Reference to IntElement.
         *  \return Reference to IntElement.
         */
        TElement<T>& operator+=(const TElement<T>& tel);

        /** \brief operator-= overload.
         *  \param i Reference to IntElement.
         *  \return Reference to IntElement.
         */
        TElement<T>& operator-=(const TElement<T>& tel);

        /** \brief operator*= overload.
         *  \param i Reference to IntElement.
         *  \return Reference to IntElement.
         */
        TElement<T>& operator*=(const TElement<T>& tel);

        /** \brief operator/= overload.
         *  \param i Reference to IntElement.
         *  \return Reference to IntElement.
         *  \throw std::invalid_argument if divisor is zero.
         */
        TElement<T>& operator/=(const TElement<T>& tel);

        /** \brief operator* overload.
         *  \param i1 Reference to IntElement.
         *  \param i2 Reference to IntElement.
         *  \return IntElement with new calculated value.
         */
        friend IntElement operator*(const IntElement& i1, const IntElement& i2);

        /** \brief operator+ overload.
         *  \param i1 Reference to IntElement.
         *  \param i2 Reference to IntElement.
         *  \return IntElement with new calculated value.
         */
        friend IntElement operator+(const IntElement& i1, const IntElement& i2);

        /** \brief operator- overload.
         *  \param i1 Reference to IntElement.
         *  \param i2 Reference to IntElement.
         *  \return IntElement with new calculated value.
         */
        friend IntElement operator-(const IntElement& i1, const IntElement& i2);

        /** \brief operator/ overload. Division is floored.
         *  \param i1 Reference to IntElement.
         *  \param i2 Reference to IntElement.
         *  \return IntElement with new calculated value.
         *  \throw std::invalid_argument if divisor is zero.
         */
        friend IntElement operator/(const IntElement& i1, const IntElement& i2);

    private:
        T t;
};

#endif // ELEMENT_H
