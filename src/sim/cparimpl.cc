//=========================================================================
//  CPARIMPL.CC - part of
//
//                  OMNeT++/OMNEST
//           Discrete System Simulation in C++
//
//  Author: Andras Varga
//
//=========================================================================

/*--------------------------------------------------------------*
  Copyright (C) 1992-2017 Andras Varga
  Copyright (C) 2006-2017 OpenSim Ltd.

  This file is distributed WITHOUT ANY WARRANTY. See the file
  `license' for details on this and other legal matters.
*--------------------------------------------------------------*/

#include <sstream>
#include "omnetpp/cparimpl.h"
#include "omnetpp/cproperties.h"
#include "omnetpp/ccomponent.h"
#include "omnetpp/csimulation.h"

#include "omnetpp/cboolparimpl.h"
#include "omnetpp/cintparimpl.h"
#include "omnetpp/cdoubleparimpl.h"
#include "omnetpp/cstringparimpl.h"
#include "omnetpp/cobjectparimpl.h"
#include "omnetpp/cxmlparimpl.h"

#ifdef WITH_PARSIM
#include "omnetpp/ccommbuffer.h"
#endif

namespace omnetpp {

long cParImpl::totalParimplObjs;
long cParImpl::liveParimplObjs;
cStringPool cParImpl::stringPool("cParImpl::stringPool");

cParImpl::cParImpl()
{
    unitp = nullptr;
    baseDirectory = nullptr;
    totalParimplObjs++;
    liveParimplObjs++;
}

cParImpl::~cParImpl()
{
    stringPool.release(unitp);
    stringPool.release(baseDirectory);
    liveParimplObjs--;
}

void cParImpl::copy(const cParImpl& other)
{
    setUnit(other.getUnit());
    setBaseDirectory(other.getBaseDirectory());
}

cParImpl& cParImpl::operator=(const cParImpl& other)
{
    bool shared = isShared();
    cNamedObject::operator=(other);
    copy(other);
    setIsShared(shared);  // preserve FL_ISSHARED flag
    return *this;
}

void cParImpl::parsimPack(cCommBuffer *buffer) const
{
    //TBD
}

void cParImpl::parsimUnpack(cCommBuffer *buffer)
{
    //TBD
}

cParImpl *cParImpl::dup() const
{
    throw cRuntimeError(this, E_CANTDUP);  // cannot instantiate an abstract class
}

const char *cParImpl::getUnit() const
{
    return unitp;
}

void cParImpl::setUnit(const char *s)
{
    stringPool.release(unitp);
    unitp = stringPool.get(s);
}

void cParImpl::setBaseDirectory(const char *s)
{
    stringPool.release(baseDirectory);
    baseDirectory = stringPool.get(s);
}

cValue cParImpl::evaluate(cExpression *expr, cComponent *contextComponent) const
{
    static int depth;
    try {
        depth++;
        if (depth >= 5)
            throw cRuntimeError("Evaluation nesting too deep (circular parameter references?)");
        cExpression::Context context(contextComponent, baseDirectory);
        cValue ret = expr->evaluate(&context);
        depth--;
        return ret;
    }
    catch (std::exception& e) {
        depth = 0;
        throw;
    }
}

int cParImpl::compare(const cParImpl *other) const
{
    int res = strcmp(getName(), other->getName());
    if (res != 0)
        return res;

    unsigned short flags2 = flags & ~FL_ISSHARED;  // ignore "isShared" flag
    unsigned short otherflags2 = other->flags & ~FL_ISSHARED;
    if (flags2 != otherflags2)
        return flags2 < otherflags2 ? -1 : 1;

    if (getType() != other->getType())
        return getType() < other->getType() ? -1 : 1;

    return opp_strcmp(unitp, other->unitp);
}

cParImpl *cParImpl::createWithType(Type type)
{
    switch (type) {
        case cPar::BOOL:    return new cBoolParImpl();
        case cPar::DOUBLE:  return new cDoubleParImpl();
        case cPar::INT:     return new cIntParImpl();
        case cPar::STRING:  return new cStringParImpl();
        case cPar::OBJECT:  return new cObjectParImpl();
        case cPar::XML:     return new cXMLParImpl();
        default: throw cRuntimeError("cParImpl::createWithType(): No such type: %d", type);
    }
}

}  // namespace omnetpp

