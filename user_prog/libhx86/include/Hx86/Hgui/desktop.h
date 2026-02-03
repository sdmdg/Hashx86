#ifndef Desktop_H
#define Desktop_H

#include <Hx86/Hgui/eventHandler.h>
#include <Hx86/Hgui/widget.h>
#include <Hx86/debug.h>

class Desktop : public CompositeWidget {
protected:
public:
    static Desktop* activeInstance;
    Desktop();
    ~Desktop();
    void innitEventHandler();
};

#endif  // Desktop_H
