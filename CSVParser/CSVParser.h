#ifndef SIMPLECSVREADER_CSVPARSER_H
#define SIMPLECSVREADER_CSVPARSER_H
#include <memory>

class CSVParser {
public:
    CSVParser();
    ~CSVParser();
    void parse(std::istream &in);
    void print(std::ostream &out);
private:
    class impl;
    std::unique_ptr<impl> pimpl;
};
#endif //SIMPLECSVREADER_CSVPARSER_H
