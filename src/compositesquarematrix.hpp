/** \file compositesquarematrix.hpp
 *  \brief CompositeSquareMatrix header file.
 */

#ifndef COMPOSITESQUAREMATRIX_H
#define COMPOSITESQUAREMATRIX_H

#include <vector>
#include <memory>
#include <functional>
#include "squarematrix.hpp"

/** \class CompositeSquareMatrix
 *  \brief Class to form ConcreteSquareMatrix from with formulas.
 */
class CompositeSquareMatrix : public SquareMatrix
{
    public:
        /** \brief Default constructor.
         */
        CompositeSquareMatrix();

        /** \brief Parametrized constructor.
         *  \param op1 New value for oprnd1.
         *  \param op2 New value for oprnd1.
         *  \param opr New value for oprtor.
         *  \param opc New value for op_char.
         */
        CompositeSquareMatrix(
            const SquareMatrix& op1,
            const SquareMatrix& op2,
            const std::function<ConcreteSquareMatrix(
                const ConcreteSquareMatrix&,
                const ConcreteSquareMatrix&)>& opr,
            char opc);

        /** \brief Copy constructor.
         *  \param m Reference to CompositeSquareMatrix.
         */
        CompositeSquareMatrix(const CompositeSquareMatrix& m);

        /** \brief Move constructor.
         *  \param m Rvalue reference to CompositeSquareMatrix.
         */
        CompositeSquareMatrix(CompositeSquareMatrix&& m);

        /** \brief Default destructor.
         */
        virtual ~CompositeSquareMatrix() = default;

        /** \brief Get row size of the matrix.
         *  \return Size of the matrix row.
         */
        unsigned int getRowSize() const override;

        /** \brief Copy assignment operator overload.
         *  \param m Reference to CompositeSquareMatrix.
         */
        CompositeSquareMatrix& operator=(const CompositeSquareMatrix& m);

        /** \brief Move assignment operator overload.
         *  \param m Rvalue reference to CompositeSquareMatrix.
         */
        CompositeSquareMatrix& operator=(CompositeSquareMatrix&& m);

        /** \brief Returns pointer to a clone of this.
         *  \return SquareMatrix pointer.
         */
        SquareMatrix* clone() const override;

        /** \brief Prints string representation to output stream.
         *  \param os std::ostream reference.
         */
        void print(std::ostream& os) const override;

        /** \brief Returns string representation of the matrix.
         *  \return std::string.
         */
        std::string toString() const override;

        /** \brief Evaluates SquareMatrix to a ConcreteSquareMatrix.
         *  \param val Valuation map.
         *  \return New instance of ConcreteSquareMatrix.
         */
        ConcreteSquareMatrix evaluate(const Valuation& val) const override;

    private:
        std::unique_ptr<SquareMatrix> oprnd1;
        std::unique_ptr<SquareMatrix> oprnd2;
        std::function<ConcreteSquareMatrix(
            const ConcreteSquareMatrix&,
            const ConcreteSquareMatrix&)> oprtor;
        char op_char;
};

#endif // COMPOSITESQUAREMATRIX_H
