//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//

#ifndef __TRANSITION_H__
#define __TRANSITION_H__

#include <omnetpp.h>
#include "ITransition.h"
#include "TransitionScheduler.h"

using namespace omnetpp;

/**
 * A transition in a Petri net
 */
class Transition : public cSimpleModule, public ITransition
{
  protected:
    int priority;
    cPar *transitionTimePar;
    cMessage *fireEvent;
    cMessage *endTransitionEvent;

    TransitionScheduler *transitionScheduler = nullptr;
    struct Neighbour { IPlace *place; int multiplicity; }; // multiplicity is negative for inhibitor arcs
    std::vector<Neighbour> inputPlaces;
    std::vector<Neighbour> outputPlaces;

  public:
    Transition();
    virtual ~Transition();

  protected:
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void refreshDisplay() const override;

    // utility methods
    virtual bool evaluateGuardCondition() {return true;} // override if needed!
    virtual void startFire();
    virtual void endFire();
    virtual void discoverNeighbours();

  public:
    virtual bool canFire() override;
    virtual void scheduleFiring() override;
    virtual void numTokensChanged(IPlace *inputPlace) override;

};

#endif


