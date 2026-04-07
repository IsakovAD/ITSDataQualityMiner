#include "ITSGeometry.h"


void ITSGeometry::getlanecoordinates(int laneid) { //[AID] shouldn't it be in constructor

  int laneob = laneid - N_LANES_IB;
  int layer, staveinlayer, laneinlayer;

  if (laneid < 12*9) {laneinlayer = laneid; layer=0;}
  else if (laneid < (12+16)*9) {laneinlayer = laneid - (12*9); layer=1;}
  else if (laneid < N_LANES_IB) { laneinlayer = laneid - (12+16)*9; layer=2;}
  else if (laneob < 24*4*4) {layer = 3; laneinlayer = laneob;}
  else if (laneob < (24+30)*4*4) { layer = 4; laneinlayer = laneob - 24*4*4;}
  else if (laneob < (24+30)*4*4 + 42*7*4) { layer = 5; laneinlayer = laneob - (24+30)*4*4;}
  else { layer = 6; laneinlayer = laneob - (24+30)*4*4 - 42*7*4;}

  vLaneToLayer[laneid] = layer;
  vLaneToLaneInLayer[laneid] = laneinlayer;

  int nz = NZElementsInHalfStave[layer];
  int nseg = NSegmentsStave[layer];
  int nsegh = (nseg == 1) ? 1 : nseg/2;
  
  staveinlayer = laneinlayer / (nz*nseg);
  int laneinstave = laneinlayer % (nz*nseg);
  
  int stave = 0;
  for (int l=0; l<7; l++) stave += (l<layer)*NStaves[l]+(l==layer)*staveinlayer;

  vLaneToStave[laneid] = stave;
  vLaneToStaveInLayer[laneid] = staveinlayer;

}  


uint16_t ITSGeometry::StaveToLayer(uint16_t stv) const{
  int count = 0;
  for (int i=0; i<7; i++){
    count+=NStaves[i];
    if (stv < count) return (uint16_t)i;
  }
  return -1;
}