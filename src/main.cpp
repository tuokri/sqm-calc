/** \file main.cpp
 *  \brief Main implementation file.
 */

#include <iostream>
#include <stack>
#include <memory>
#include <locale>
#include <exception>
#include "utils.hpp"
#include "element.hpp"
#include "squarematrix.hpp"
#include "compositesquarematrix.hpp"

#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

/** \brief Main function for I/O and erroneous input handling.
 *  \return 0 if success, else non-zero.
 */
int main(int argc, char** argv)
{
    if(n_threads == 0)
    {
        std::cout << _RED_ << "Cannot read the amount of system threads." << std::endl;
        std::cout << "Exiting." << _END_ << std::endl;
        return -1;
    }

    int res = Catch::Session().run(argc, argv);

    std::stack<std::unique_ptr<SquareMatrix>> mstack;
    std::string buffer;
    Valuation valuation;

    std::cout << _YEL_ << "*** SQUARE MATRIX CALCULATOR ***" << _END_ << std::endl;
    std::cout << "* Make a selection:" << std::endl;
    std::cout << "* Input operation: " << _GRN_ << "\"+\" \"-\" \"*\" \"/\"" << _END_ << "." << std::endl;
    std::cout << "* Input " << _GRN_ << "\"quit\"" << _END_ << " to quit." << std::endl;
    std::cout << "* Input " << _GRN_ << "\"clearval\"" << _END_ << " to clear valuation map." << std::endl;
    std::cout << "* Input " << _GRN_ << "\"printval\"" << _END_ << " to print valuation map." << std::endl;
    std::cout << "* Input " << _GRN_ << "\"stacksize\"" << _END_ << " to print stack size." << std::endl;
    std::cout << "* Input " << _GRN_ << "\"=\"" << _END_ << " to evaluate matrix at the stack top." << std::endl;
    std::cout << "* Input matrix in string format to add it to stack." << std::endl;
    std::cout << "* \tExample: " << _GRN_ << "\"[[1,2][a,b]]\"." << _END_ << std::endl;
    std::cout << "* \tExample: " << _GRN_ << "\"[[4,2][5,6]]\"." << _END_ << std::endl;
    std::cout << "* Input valuation in format " << _GRN_ << "\"x=2\"" << _END_ << " to add it to valuation map." << std::endl;

    std::string prompt{_YEL_+ "Please input a selection and press ENTER." + _END_};
    while(get_user_input(std::cout, std::cin, prompt, buffer))
    {
        if(buffer == "clearval")
        {
            valuation.clear();
            std::cout << _GRN_ << "Valuation map cleared." << _END_ << std::endl;
        }
        else if(buffer == "stacksize")
        {
            std::cout << _GRN_ << "Stack size: " << mstack.size() << _END_ << std::endl;
        }
        else if(buffer == "printval")
        {
            if(valuation.size() == 0)
            {
                std::cout << _GRN_ << "Valuation map is empty." << _END_ << std::endl;
            }
            else
            {
                for(auto it = valuation.begin(); it != valuation.end(); it++)
                {
                    std::cout << _GRN_ << it->first << " = " << it->second << _END_ << std::endl;
                }
            }
        }
        else if(buffer == "quit")
        {
            break;
        }
        else if(buffer == "+" || buffer == "-" ||
                buffer == "*" || buffer == "/")
        {
            if(mstack.size() < 2)
            {
                std::cout << _RED_ << "Too few matrices in stack." << _END_ << std::endl;
            }
            else
            {
                auto m1 = std::move(mstack.top());
                mstack.pop();
                auto m2 = std::move(mstack.top());
                mstack.pop();

                char opchar = buffer[0];

                std::function<ConcreteSquareMatrix(
                    const ConcreteSquareMatrix&,
                    const ConcreteSquareMatrix&)> func;

                switch(opchar)
                {
                    case '+':
                        func = [](const ConcreteSquareMatrix& m1,
                                  const ConcreteSquareMatrix& m2)
                                  {return m1 + m2;};
                        break;

                    case '-':
                        func = [](const ConcreteSquareMatrix& m1,
                                  const ConcreteSquareMatrix& m2)
                                  {return m1 - m2;};
                        break;

                    case '*':
                        func = [](const ConcreteSquareMatrix& m1,
                                  const ConcreteSquareMatrix& m2)
                                  {return m1 * m2;};
                        break;

                    case '/':
                        func = [](const ConcreteSquareMatrix& m1,
                                  const ConcreteSquareMatrix& m2)
                                  {return m1 / m2;};
                        break;

                    default:
                        std::cout << "Something went terribly wrong." << std::endl;
                        return -1;
                }

                CompositeSquareMatrix csm{*m1, *m2, func, opchar};

                csm.print(std::cout);
                std::cout << std::endl;

                mstack.push(
                    std::move(std::unique_ptr<CompositeSquareMatrix>{
                        static_cast<CompositeSquareMatrix*>(csm.clone())}));
            }
        }
        else if(buffer == "=")
        {
            if(mstack.size() < 1)
            {
                std::cout << _RED_ << "Stack is empty." << _END_ << std::endl;
            }
            else
            {
                try
                {
                    std::unique_ptr<SquareMatrix> mptr{mstack.top()->clone()};
                    std::cout << _BLU_ << "Calculating : " << _END_ << _GRN_ << mptr->toString() << _END_ << std::endl;
                    std::cout << _BLU_ << "Result : " << _END_;
                    std::cout << _GRN_;
                    mptr->evaluate(valuation).print(std::cout);
                    std::cout << _END_ << std::endl;
                }
                catch(std::exception& e)
                {
                    std::cout << _RED_ << "Error while calculating matrices: ";
                    std::cout << e.what() << _END_ << std::endl;
                }
            }

        }
        else if(isalpha(buffer[0]))
        {
            std::string num;
            char var = buffer[0];

            if(buffer[1] != '=')
            {
                std::cout << _RED_ << "Invalid valuation input." << _END_ << std::endl;
            }
            else
            {
                for(unsigned int i = 2; i < buffer.size(); i++)
                {
                    if(!std::isdigit(buffer[i]))
                    {
                        std::cout << _RED_ << "Invalid valuation input." << _END_ << std::endl;
                        break;
                    }
                    num += buffer[i];
                }

                try
                {
                    valuation.insert({var, std::stoi(num)});
                    std::cout << _GRN_ << "Added valuation." << _END_ << std::endl;
                }
                catch(std::exception)
                {
                    std::cout << _RED_ << "Invalid valuation input." << _END_ << std::endl;
                }
            }

        }
        else if(buffer[0] == '[')
        {
            bool is_alpha;

            /* Determine whether we are to
             * create VariableElements or IntElements.
             */
            for(unsigned int i = 2; i < buffer.size(); i++)
            {
                if(isalpha(buffer[i]))
                {
                    is_alpha = true;
                    break;
                }
            }

            try
            {
                if(is_alpha)
                {
                    mstack.push(
                        std::unique_ptr<SquareMatrix>{
                            new SymbolicSquareMatrix{buffer}});
                }
                else
                {
                    mstack.push(
                        std::unique_ptr<SquareMatrix>{
                            new ConcreteSquareMatrix{buffer}});
                }

                std::cout << _GRN_ << "Added matrix to stack." << _END_ << std::endl;
            }
            catch(std::exception)
            {
                std::cout << _RED_ << "Input was not recognized." << _END_ << std::endl;
            }
        }
        else
        {
            std::cout << _RED_ << "Input was not recognized." << _END_ << std::endl;
        }
    }

    return res;
}
