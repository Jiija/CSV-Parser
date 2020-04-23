#include <queue>
#include <unordered_map>
#include <vector>
#include <istream>
#include <sstream>
#include <functional>
#include "CVSParser.h"
using std::vector;
using std::queue;
using std::unordered_map;
using std::string;

using uint = unsigned int;

namespace operations {
    const std::function<uint(uint, uint)> add = [](uint fArg, uint sArg) {return fArg + sArg;};
    const std::function<uint(uint, uint)> sub = [](uint fArg, uint sArg) {return fArg - sArg;};
    const std::function<uint(uint, uint)> div = [](uint fArg, uint sArg) {return fArg / sArg;};
    const std::function<uint(uint, uint)> mul = [](uint fArg, uint sArg) {return fArg * sArg;};
}

struct ExpressionNode {
    const std::function<uint(uint, uint)> &operation;
    string fArgCol;
    int fArgRow;
    string sArgCol;
    int sArgRow;
    size_t exprCellI, exprCellJ;
};

class CVSParser::impl {
public:
    impl();
    ~impl();
    void parse(std::istream &in);
    void print(std::ostream &out);
private:
    queue<ExpressionNode> expressionsQueue;
    vector<vector<uint>> table;
    unordered_map<string, size_t> columns;
    unordered_map<int, size_t> rows;
    void processExpression(const string&, size_t, size_t);
    void calculateExpression(ExpressionNode&);
    bool isOperator(const char &c) const {return c == '+' || c == '-' || c == '/' || c == '*';};
};

void CVSParser::impl::parse(std::istream &in)
{
    string buffer, token;
    std::getline(in, buffer);

    std::istringstream tokenizer(buffer);
    std::getline(tokenizer, token, ',');

    size_t i = 0, j = 0, m = 0;
    while(std::getline(tokenizer, token, ','))
        columns[token] = m++;

    while(std::getline(in, buffer))
    {
        tokenizer.str(buffer);
        std::getline(tokenizer, token, ',');
        rows[std::stoi(token)] = i;
        while(std::getline(tokenizer, token, ','))
        {
            if (token[0] == '=')
              processExpression(token, i, j);
            else
                table[i][j] = std::stoul(token);
            j++;
        }
        i++;
    }

    while (!expressionsQueue.empty()) {
        calculateExpression(expressionsQueue.front());
        expressionsQueue.pop();
    }
}


void CVSParser::impl::processExpression(const string &token, size_t exprI, size_t exprJ)
{
    int i = 1;
    char c = token[i];
    string argColumn, argRow;
    ExpressionNode e;
    while(!isdigit(c))
    {
        argColumn.push_back(c);
        c = token[++i];
    }
    e.firstArgCol = argColumn;


}

void CVSParser::impl::calculateExpression(ExpressionNode &e)
{
    uint firstOperand = table[rows[e.fArgRow]][columns[e.fArgCol]], secondOperand = table[rows[e.sArgRow]][columns[e.sArgCol]];
    if (firstOperand == -1 || secondOperand == -1)
    {
        expressionsQueue.push(e);
        return;
    }


    table[e.exprCellI][e.exprCellJ] = e.operation(firstOperand, secondOperand);
}

