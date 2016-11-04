#include "Event/TbCluster.h"

using namespace LHCb;

//=============================================================================
// Clone method
//=============================================================================
TbCluster *TbCluster::clone() {
  TbCluster *c = new TbCluster();
  c->setX(x());
  c->setY(y());
  c->setZ(z());
  c->setXloc(xloc());
  c->setYloc(yloc());
  c->setCharge(charge());
  c->setPlane(plane());
  c->setTime(time());
  c->setXErr(xErr());
  c->setYErr(yErr());
  return c;
}

