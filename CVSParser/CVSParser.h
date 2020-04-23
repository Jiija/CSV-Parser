#ifndef SIMPLECSVREADER_CVSPARSER_H
#define SIMPLECSVREADER_CVSPARSER_H

#include <memory>

class CVSParser {
public:
    CVSParser();
    ~CVSParser();
    void parse(std::istream &in);
    void print(std::ostream &out);
private:
    class impl;
    std::unique_ptr<impl> pimpl;
};
#endif //SIMPLECSVREADER_CVSPARSER_H
