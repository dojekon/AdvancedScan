#ifndef SHELL_H
#define SHELL_H

#include <qstring.h>

class Shell
{
public:
    Shell();

    static QString Execute(const QString cmd);

};

#endif // SHELL_H
