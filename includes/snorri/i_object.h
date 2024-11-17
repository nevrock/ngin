#ifndef IOBJECT_H
#define IOBJECT_H

//#include <snorri/point.h> // Include the Point class definition
#include <memory>
#include <glm/glm.hpp>
#include <string>

class Point;
class PointUi;

class IObject {
public:
    virtual ~IObject() {}

    // These should be marked as const methods that return const references for immutable access
    virtual Point& getPoint() = 0;  // Provides mutable access to Point
    virtual PointUi& getPointUi() = 0;  // Provides mutable access to Point
    virtual const Point& getPoint() const = 0;  // Provides immutable access to Point
    virtual const PointUi& getPointUi() const = 0;  // Provides immutable access to Point
    virtual IObject* getParent() const = 0;

    virtual glm::mat4 getWorldMatrix(bool includeSelf = true) const = 0; // Correct method declaration
    virtual glm::vec3 getWorldPosition() const = 0; // Correct method declaration

    virtual bool hasParent() const = 0;
    virtual bool isUi() = 0;
    virtual std::string getName() const = 0;

    virtual unsigned int getHierarchyIndex() const = 0;
};

#endif // IOBJECT_H
