#include <queue>
#include <unordered_map>
#include <vector>
#include <istream>
#include <sstream>
#include <functional>
#include <algorithm>
#include <iterator>
#include <exception>
#include <limits>
#include "CSVParser.h"
using namespace std;

using uint = unsigned int;
using operation = function<uint(uint, uint)>;
using columnpair = pair<string, uint>;
using rowpair = pair<uint, uint>;

class CSVParserException : public exception {
public:
    CSVParserException(const char* msg) : msg(msg) {};
    const char * what () const throw () { return msg; }
private:
    const char* msg;
};

namespace operations {
    const operation add = [](uint fArg, uint sArg) {
        if (fArg >= UINT_MAX - sArg)
            throw new CSVParserException("Addition overflow");
        return fArg + sArg;
    };

    const operation sub = [](uint fArg, uint sArg) {
        if (fArg < sArg)
            throw new CSVParserException("Substraction underflow");
        return fArg - sArg;
    };

    const operation div = [](uint fArg, uint sArg) {
        if (sArg == 0)
            throw new CSVParserException("Division by zero");
        return fArg / sArg;
    };

    const operation mul = [](uint fArg, uint sArg) {
        uint res = fArg * sArg;
        if (fArg != 0 && sArg != 0 && fArg != res / sArg)
            throw new CSVParserException("Multiplication overflow");
        return res;
    };
}



class CSVParser::impl {
    struct ExpressionNode {
        operation oper;
        string fArgCol;
        uint fArgRow;
        string sArgCol;
        uint sArgRow;
        size_t exprCellI, exprCellJ;
    };
public:
    impl() = default;
    ~impl() = default;
    void parse(istream &in);
    void print(ostream &out);
private:
    queue<ExpressionNode> expressionsQueue;
    size_t m;
    size_t initialQueueSize;
    size_t pushBackCounter;
    vector<vector<uint>> table;
    unordered_map<string, size_t> columns;
    unordered_map<uint, size_t> rows;
    void processExpression(const string&, size_t, size_t);
    void calculateExpression(ExpressionNode&);
};

void CSVParser::impl::parse(istream &in)
{
    table.clear();
    rows.clear();
    columns.clear();
    m = 0;
    pushBackCounter = 0;

    string buffer, token;
    getline(in, buffer);
    buffer.erase(remove_if(buffer.begin(), buffer.end(), ::isspace), buffer.end());

    istringstream tokenizer(buffer);
    getline(tokenizer, token, ',');

    size_t i = 0;
    while(getline(tokenizer, token, ','))
    {
        if (columns.count(token))
            throw CSVParserException("Duplicate column names");
        columns[token] = m++;
    }

    while(getline(in, buffer))
    {
        size_t j = 0;
        table.push_back(vector<uint>());
        buffer.erase(remove_if(buffer.begin(), buffer.end(), ::isspace), buffer.end());
        tokenizer.str(buffer);
        tokenizer.clear();
        getline(tokenizer, token, ',');

        uint column = stoul(token);
        if (rows.count(column))
            throw new CSVParserException("Duplicate row indices");
        rows[column] = i;

        while(getline(tokenizer, token, ','))
        {
            if (token[0] == '=')
            {
                table[i].push_back(-1);
                processExpression(token, i, j);
            }
            else
            {
                table[i].push_back(stoul(token));
            }
            j++;
        }
        if (table[i].size() != m)
            throw CSVParserException("Bad table dimensions");
        i++;
    }

    initialQueueSize = expressionsQueue.size();
    while (!expressionsQueue.empty())
    {
        calculateExpression(expressionsQueue.front());
        expressionsQueue.pop();
    }
}


void CSVParser::impl::processExpression(const string &token, size_t exprI, size_t exprJ)
{
    auto stringIter = token.begin();
    stringIter++;
    string argColumn, argRow;
    ExpressionNode e;
    while(!isdigit(*stringIter))
    {
        argColumn.push_back(*stringIter);
        stringIter++;
    }
    while(isdigit(*stringIter))
    {
        argRow.push_back(*stringIter);
        stringIter++;
    }
    e.fArgCol = argColumn;
    e.fArgRow = stoul(argRow);

    switch(*stringIter) {
        case '+':
            e.oper = operations::add;
            break;
        case '-':
            e.oper = operations::sub;
            break;
        case '*':
            e.oper = operations::mul;
            break;
        case '/':
            e.oper = operations::div;
            break;
        default:
            throw CSVParserException("Unknown operator in one of the expressions");
            break;
    }

    argColumn.clear();
    argRow.clear();
    stringIter++;
    while(!isdigit(*stringIter))
    {
        argColumn.push_back(*stringIter);
        stringIter++;
    }
    while(isdigit(*stringIter))
    {
        argRow.push_back(*stringIter);
        stringIter++;
    }
    e.sArgCol = argColumn;
    e.sArgRow = stoul(argRow);

    e.exprCellI = exprI;
    e.exprCellJ = exprJ;
    expressionsQueue.push(e);
}

void CSVParser::impl::calculateExpression(ExpressionNode &e)
{
    auto argI = rows.find(e.fArgRow);
    auto argJ = columns.find(e.fArgCol);
    if (argI == rows.end() || argJ == columns.end())
        throw CSVParserException("One of the expressions references a non-existent row or column");
    if (argI->second == e.exprCellI && argJ->second == e.exprCellJ)
        throw CSVParserException("Self-reference in one of the expressions");
    uint firstOperand = table[argI->second][argJ->second];

    argI = rows.find(e.sArgRow);
    argJ = columns.find(e.sArgCol);
    if (argI == rows.end() || argJ == columns.end())
        throw CSVParserException("One of the expressions references a non-existent row or column");
    if (argI->second == e.exprCellI && argJ->second == e.exprCellJ)
        throw CSVParserException("Self-reference in one of the expressions");
    uint secondOperand = table[argI->second][argJ->second];

    if (firstOperand == -1 || secondOperand == -1)
    {
        expressionsQueue.push(e);
        pushBackCounter++;
        if (pushBackCounter > initialQueueSize + 1)
            throw CSVParserException("Found a cycle in expression references");
        return;
    }

    table[e.exprCellI][e.exprCellJ] = e.oper(firstOperand, secondOperand);
}

void CSVParser::impl::print(ostream &out)
{
    if (table.empty())
        return;

    vector<columnpair> orderedColumns;
    vector<rowpair> orderedRows;

    copy(columns.begin(), columns.end(), back_inserter(orderedColumns));
    copy(rows.begin(), rows.end(), back_inserter(orderedRows));

    sort(orderedColumns.begin(), orderedColumns.end(), [](const columnpair &l, const columnpair &r) {
        return l.second < r.second;
    });
    sort(orderedRows.begin(), orderedRows.end(), [](const rowpair &l, const rowpair &r) {
        return l.second < r.second;
    });

    for (auto &r : orderedColumns)
    {
        out << ",";
        out << r.first;
    }
    out << endl;

    size_t i = 0;
    for (auto &c : orderedRows)
    {
        out << c.first;
        for (size_t j = 0; j < m; j++)
        {
            out << ",";
            out << table[i][j];
        }
        out << endl;
        i++;
    }
}

CSVParser::CSVParser() : pimpl(make_unique<impl>()) { }

CSVParser::~CSVParser() = default;

void CSVParser::parse(istream &in)
{
    try
    {
        pimpl->parse(in);
    }
    catch(invalid_argument e)
    {
        throw CSVParserException("One of the table's cells is not an unsigned int or expression");
    }
}

void CSVParser::print(ostream &out)
{
    pimpl->print(out);
}
