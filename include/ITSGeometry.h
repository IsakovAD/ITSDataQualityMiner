#pragma once


#include <cstdint>


class ITSGeometry {
public:
    ITSGeometry(){
        for (int i=0; i<N_LANES; i++) getlanecoordinates(i);
    };  

    uint16_t  isFirstOfLane(uint16_t chipid);
    uint16_t StaveToLayer(uint16_t stv) const;
    void getlanecoordinates(int laneid) ;

    uint16_t LaneToLayer(uint16_t laneid) const { return vLaneToLayer[laneid];} 
    uint16_t LaneToStave(uint16_t laneid) const { return vLaneToStave[laneid];}
    uint16_t LaneToStaveInLayer(uint16_t laneid) const { return vLaneToStaveInLayer[laneid];}
    uint16_t LaneToLaneInLayer(uint16_t laneid) const { return vLaneToLaneInLayer[laneid];}


    uint16_t isFirstOfLane(uint16_t chipid) const { 
        return chipid < N_LANES_IB ? chipid : N_LANES_IB + (uint16_t)((chipid - N_LANES_IB) / 7);
    }
    
    uint16_t isLastOfLane(uint16_t chipid) const { 
        return chipid < N_LANES_IB ? chipid : isFirstOfLane(chipid - 6);
    } 


    const int NStaves[7] = { 12, 16, 20, 24, 30, 42, 48 };
    static constexpr int StaveBoundary[8] = { 0, 12, 28, 48, 72, 102, 144, 192 };
    const int NZElementsInHalfStave[7] =  {9,9,9, 4, 4, 7, 7};
    const int NSegmentsStave[7] = {1, 1, 1, 4, 4, 4, 4};
    const int NLanesPerStave[7] = {9, 9, 9, 16, 16, 28, 28};
    const int NChipsPerLane[7] = {1, 1, 1, 7, 7, 7, 7};
    const int NChipsPerLayer[7] = {12*9, 16*9, 20*9, 24*112, 30*112, 42*196, 48*196};
    const int N_LANES_IB = 432;
    const int N_LANES_ML = 864; // L3,4
    static const int N_LANES = 3816;
    const int N_STAVES_IB = 12+16+20;
    const int N_STAVES = 192;
    const int N_CHIPS = 24120;
    const int N_CHIPS_IB = N_LANES_IB;
private:
    



    int vLaneToLayer[N_LANES]; // filled by "getlanecoordinates" when called
    int vLaneToStave[N_LANES]; // filled by "getlanecoordinates" when called
    int vLaneToStaveInLayer[N_LANES]; // filled by "getlanecoordinates" when called
    int vLaneToLaneInLayer[N_LANES]; // filled by "getlanecoordinates" when called

};