#ifndef GRAPH_EDITOR_H
#define GRAPH_EDITOR_H

#include <ngin/object.h>

class GraphEditor : public Component
{
public: 
    GraphEditor(Object* parent);
    void loadFromDict(const Dict& d) override;

private:

};

#endif
