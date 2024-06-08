#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <stack>
#include <string>
#include <cctype>
#include <map>
#include <cmath>
#include <limits>
#include <vector>
#include <list>

int precedence(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/' || op == '%') return 2;
    if (op == '^') return 3;
    return 0;
}

float applyOp(float a, float b, char op) {
    switch (op) {
    case '+': return a + b;
    case '-': return a - b;
    case '*': return a * b;
    case '/': if (b == 0) throw std::runtime_error("Division by zero"); else return a / b;
    case '%': return int(a) % int(b);
    case '^': return std::pow(a, b);
    default: return 0;
    }
}

bool isOperator(const std::string& token) {
    return token == "+" || token == "-" || token == "*" || token == "/" || token == "%" || token == "^";
}

std::vector<std::string> tokenize(const std::string& s, const std::map<std::string, double>& constants, std::map<std::string, double>& variables) {
    std::vector<std::string> tokens;
    std::string current;
    for (char ch : s) {
        if (std::isspace(ch) || isOperator(std::string(1, ch)) || ch == '(' || ch == ')') {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
            if (!std::isspace(ch)) {
                tokens.push_back(std::string(1, ch));
            }
        }
        else {
            current += ch;
        }
    }
    if (!current.empty()) {
        tokens.push_back(current);
    }

    // Convert constants and variables to their numeric values
    for (auto& token : tokens) {
        if (constants.find(token) != constants.end()) {
            token = std::to_string(constants.at(token));
        }
        else if (variables.find(token) != variables.end()) {
            token = std::to_string(variables.at(token));
        }
    }
    return tokens;
}

std::string infixToPostfix(const std::vector<std::string>& tokens) {
    std::stack<std::string> stack;
    std::string postfix;
    std::cout << "\nConverting to Postfix:\n";
    for (const std::string& token : tokens) {
        if (std::isdigit(token[0]) || token.find('.') != std::string::npos || (token.size() > 1 && token[0] == '-')) {  // Número
            postfix += token + " ";
            std::cout << "Added to output: " << token << "\n";
        }
        else if (token == "(") {
            stack.push(token);
            std::cout << "Pushed to stack: " << token << "\n";
        }
        else if (token == ")") {
            while (!stack.empty() && stack.top() != "(") {
                postfix += stack.top() + " ";
                std::cout << "Popped from stack to output: " << stack.top() << "\n";
                stack.pop();
            }
            stack.pop();  // Remueve '('
            std::cout << "Popped from stack: " << "(\n";
        }
        else if (isOperator(token)) {
            while (!stack.empty() && stack.top() != "(" && precedence(stack.top()[0]) >= precedence(token[0])) {
                postfix += stack.top() + " ";
                std::cout << "Popped from stack to output: " << stack.top() << " (Precedence)\n";
                stack.pop();
            }
            stack.push(token);
            std::cout << "Pushed to stack: " << token << "\n";
        }
    }
    while (!stack.empty()) {
        postfix += stack.top() + " ";
        std::cout << "Popped from stack to output at end: " << stack.top() << "\n";
        stack.pop();
    }
    return postfix;
}

std::map<std::string, double> loadConstants(const std::string& filename) {
    std::map<std::string, double> constants;
    std::ifstream file(filename);
    std::string line;

    if (!file) {
        std::cerr << "No se pudo abrir el archivo de constantes: " << filename << std::endl;
        return constants;
    }

    while (getline(file, line)) {
        std::istringstream iss(line);
        std::string key;
        double value;
        if (getline(iss, key, '=') && iss >> value) {
            constants[key] = value;
        }
    }
    return constants;
}

float evaluatePostfix(const std::string& postfix) {
    std::stack<float> stack;
    std::istringstream iss(postfix);
    std::string token;
    while (iss >> token) {
        if (isdigit(token[0]) || (token.size() > 1 && token[0] == '-') || token.find('.') != std::string::npos) {  // If the token is a number
            stack.push(std::stof(token));
        }
        else if (isOperator(token)) {
            float b = stack.top(); stack.pop();
            float a = stack.top(); stack.pop();
            stack.push(applyOp(a, b, token[0]));
        }
    }
    return stack.top();
}


bool isValidExpression(const std::vector<std::string>& tokens) {
    int openParentheses = 0;

    for (const auto& token : tokens) {
        if (token == "(") {
            openParentheses++;
        }
        else if (token == ")") {
            if (openParentheses == 0) {
                throw std::runtime_error("Mismatched parentheses: Too many closing parentheses");
            }
            openParentheses--;
        }
    }

    if (openParentheses != 0) {
        throw std::runtime_error("Mismatched parentheses: Not all parentheses are closed");
    }

    return true;
}

bool hasConsecutiveOperators(const std::vector<std::string>& tokens) {
    bool lastWasOperator = false;
    for (const auto& token : tokens) {
        if (isOperator(token)) {
            if (lastWasOperator) {
                return true;  // Encontr� dos operadores consecutivos
            }
            lastWasOperator = true;
        }
        else {
            lastWasOperator = false;
        }
    }
    return false;
}



int  run_val() {
    auto constants = loadConstants("constants.txt");
    std::map<std::string, double> variables;
    std::list<std::string> history;
    std::string expression;

    while (true) {
        std::cout << "Enter an expression (or type 'exit' to quit): ";
        std::getline(std::cin, expression);

        if (expression == "exit" || expression == "Exit") {
            break;
        }

        size_t equals = expression.find('=');
        if (equals != std::string::npos) {
            std::string varName = expression.substr(0, equals);
            std::string valueExpr = expression.substr(equals + 1);

            try {
                auto tokens = tokenize(valueExpr, constants, variables);
                if (hasConsecutiveOperators(tokens)) {
                    throw std::runtime_error("Invalid sequence of operators");
                }
                if (!isValidExpression(tokens)) {
                    continue; // If expression is not valid, skip further processing
                }
                std::string postfix = infixToPostfix(tokens);
                float value = evaluatePostfix(postfix);
                variables[varName] = value;
                std::cout << "Variable " << varName << " set to " << value << std::endl;
                continue;
            }
            catch (const std::exception& e) {
                std::cerr << "Error setting variable: " << e.what() << std::endl;
                continue;
            }
        }

        try {
            auto tokens = tokenize(expression, constants, variables);
            if (hasConsecutiveOperators(tokens)) {
                throw std::runtime_error("Invalid sequence of operators");
            }
            if (!isValidExpression(tokens)) {
                continue; // If expression is not valid, skip further processing
            }
            std::string postfix = infixToPostfix(tokens);
            float result = evaluatePostfix(postfix);
            std::cout << "Result: " << result << std::endl;
            std::cout << "Postfix Expression: " << postfix << std::endl;
            history.push_back(expression + " = " + std::to_string(result));
        }
        catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            continue;
        }

        std::cout << "History:\n";
        for (const auto& h : history) {
            std::cout << h << std::endl;
        }
    }

    return 0;
}
