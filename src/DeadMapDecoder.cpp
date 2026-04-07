#include "DeadMapDecoder.h"

//mGeo;
DeadMapDecoder::DeadMapDecoder(const ITSGeometry& geo) : mGeo(geo){};

std::vector<uint16_t>  DeadMapDecoder::expandvector(std::vector<uint16_t> words, std::string version, string opt = "lane"){

  //std::cout<<" we are expanding "<<std::endl; 
  std::vector<uint16_t> elementlist{};
  if (version == "2" || version == "3" || version == "4"){

    uint16_t firstel = 9999, lastel = 9999;

    for (long unsigned int i=0; i<words.size(); i++){
      //std::cout<<"word id: "<< i << " value: "<< words[i]<<std::endl;
      uint16_t w = words[i];

      if (w & 0x8000){
	        firstel = w & (0x7FFF);
	        lastel = words[i+1];
	        i++;
      }
      else {
        	firstel = lastel = w;
      }



    {

	uint16_t firstlane = mGeo.isFirstOfLane(firstel);
	uint16_t lastlane = mGeo.isLastOfLane(lastel);

 


  //std::cout<<"firstel: "<< firstel << " lastel: "<< lastel << " correspond: "<< " firstlane= "<< firstlane << " lastlane= "<< lastlane <<std::endl;   
	int ndeadlanes[mGeo.N_STAVES] = {0};

	
	if (firstlane == 9999 || lastlane == 9999){
    //std::cout<< "something is 99999" << std::endl;

    if (lastlane==9999)
        lastlane=firstlane;
    else 
        firstlane=lastlane;
	  if (lastel - firstel + 1 < 7){ // dummy to skip the full interval
	    firstel = 24121;
      
	  }
	  
	  else{
	    //QALOG<<"Single chips decoded in OB: from "<<firstel<<" to "<<lastel<<"\n";
	    //QAcheck["Chip interval"] = "MEDIUM";
	  }  
	}

	  
  if (opt == "stave"){ // TO DO
	  for (uint16_t il = firstlane ; il <= lastlane; il++)
    //for (uint16_t il = firstel ; il <= lastel; il++) 
    {
            //il=8760 corresponds to stave: 17791 
            // il=7638 corresponds to stave: 51199
          // std::cout<<"il="<<il<< "corresponds to stave: "<< LaneToStave(il)<< "firstlane: "<<firstlane << " last lane: "<<lastlane  << std::endl;
	         ndeadlanes[mGeo.LaneToStave(il)]++;
           //std::cout<<"filled "<<std::endl;      
	  } 
	} // end opt == stave
 

	if (opt == "stave"){
//      std::cout<<" we are in the stave"<<std::endl; 

	  for (uint16_t ist = 0; ist < mGeo.N_STAVES; ist++){

	    if (ndeadlanes[ist] == mGeo.NLanesPerStave[mGeo.StaveToLayer(ist)]){
	      elementlist.push_back(ist);
        //std::cout<<"pushed!"<<std::endl;
	    }
	  }
	} // end of opt == stave
      } // end opt == lane || opt == stave
    } // end loop word
  } // end map version


  else{
    std::cout<<"FATAL: map version not recognized, returning empty vector.\n";
    //QAcheck["MAP version"] = "FATAL";
  }

  return elementlist;
}


std::vector<DeadStave> DeadMapDecoder::analysis(int run, string path){

      std::vector<DeadStave> vLongShutdowns;

      
       auto& cm = o2::ccdb::BasicCCDBManager::instance(); //AID: it should be created only once, move to constructor


      float LHCOrbitNS = 88924.6;
      std::string ccdbHost = "http://alice-ccdb.cern.ch";
      std::string detector = "ITS";
      cm.setURL(ccdbHost);



      auto lims = cm.getRunDuration(run);
      if (lims.first == 0 || lims.second == 0) {
            std::cout<<"ERROR - failed to fetch run info from RCT for run "<<run<<"\n";
            return {};
      }
      long runstart = -1, runstop = -1;
      runstart = (long)lims.first;
      runstop = (long)lims.second;
      
      o2::itsmft::TimeDeadMap* obj  = cm.getForTimeStamp<o2::itsmft::TimeDeadMap>(detector+"/Calib/TimeDeadMap",runstop);
      auto* orbitReset = cm.getForTimeStamp<std::vector<Long64_t>>("CTP/Calib/OrbitReset", runstop);

      const int CooldownLength = 5; //how long between two consecutive orbits with no data to create new entry (sec)
      const int DeadStaveDuration = 5; //how long stave should be missing (in minutes) to be pushed for vector
      if (!obj){
                  std::cout<<" problem with dowloading"<<std::endl;
      }else{
                  std::cout<<"Reading map info\n";
                  std::string mapver = obj->getMapVersion();
                  std::cout<<"Map version = "<<mapver<<"\n";
                  std::cout<<"Is default object = "<<obj->isDefault()<<"\n";
                  std::cout<<"Evolving map size = "<<obj->getEvolvingMapSize()<<"\n";
                  std::vector<uint16_t> MapAtOrbit;
                  
                  std::vector<unsigned long> MAPKeys = obj->getEvolvingMapKeys();
                  
                  for (int i=0; i<MAPKeys.size(); i++){
                        unsigned long orbit = MAPKeys[i];
                        std::vector<uint16_t> DeadChips;
                        obj->getMapAtOrbit(orbit, DeadChips);
                        
                        std::vector<uint16_t> StaveMAP = expandvector(DeadChips,"4","stave"); //list of empty staves in this orbit
                        int64_t  orbitResetMUS = (*orbitReset)[0];
                        long UNIX_orbit = std::ceil((orbit * o2::constants::lhc::LHCOrbitNS / 1000 + orbitResetMUS) / 1000); //current time
                        
                        
                        for (int stave_id: StaveMAP){                          
                                  auto iter = std::find_if(vLongShutdowns.rbegin(), vLongShutdowns.rend(), [stave_id](const auto& item) {
                                                                                                            return item.Stave == stave_id;
                                                                                                        }); //looking for input for the same stave
                        if (iter != vLongShutdowns.rend()){

                                  if ( (UNIX_orbit-(*iter).End) > CooldownLength*1000)  {
                                      vLongShutdowns.push_back(DeadStave(UNIX_orbit,UNIX_orbit,stave_id));
                                  } 
                                  else { 
                                    (*iter).End = UNIX_orbit; //extending
                                   
                                  }
                          } else

                            vLongShutdowns.push_back(DeadStave(UNIX_orbit,UNIX_orbit,stave_id));
                        }
                        
                        for (std::vector<DeadStave>::iterator it = vLongShutdowns.begin();it< vLongShutdowns.end();){
                          //std::cout<<"Inside of delete with stave: "<<(*it).Stave<< "time: "<< (*it).Begin << ":"<< (*it).End << " UNIX_orbit-(*it).End is "<< UNIX_orbit-(*it).End << " ( (*it).End - (*it).Begin) "<<( (*it).End - (*it).Begin) / (60*1000) <<std::endl;
                          if ( ( UNIX_orbit-(*it).End > 60*1000) && ( (*it).End - (*it).Begin) < DeadStaveDuration*60*1000){
                                           vLongShutdowns.erase(it);
                          }

                          else ++it;                 


                        }
                        

                                                
                              
                  }
      }
    for (std::vector<DeadStave>::iterator it = vLongShutdowns.begin();it< vLongShutdowns.end();){
        if ( ( (*it).End - (*it).Begin) < DeadStaveDuration*60*1000)
                                           vLongShutdowns.erase(it);
        else ++it;                 
    }



  
  std::ofstream fOut( path + "/output.txt", std::ios::app);
    for (auto shutdown: vLongShutdowns){
      int layer = mGeo.StaveToLayer(shutdown.Stave);
      fOut<<"Run"<< run<<" L"<< layer<<"_"<< shutdown.Stave - mGeo.StaveBoundary[layer] << " with duration: "<< (shutdown.End-shutdown.Begin)/60000 << " begin: "<<shutdown.Begin << " end "<< shutdown.End  <<std::endl;
    }

    return vLongShutdowns;


}

