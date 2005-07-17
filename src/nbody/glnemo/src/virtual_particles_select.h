// ============================================================================
// Copyright Jean-Charles LAMBERT - 2004-2005                                  
// e-mail:   Jean-Charles.Lambert@oamp.fr                                      
// address:  Dynamique des galaxies                                            
//           Laboratoire d'Astrophysique de Marseille                          
//           2, place Le Verrier                                               
//           13248 Marseille Cedex 4, France                                   
//           CNRS U.M.R 6110                                                   
// ============================================================================
// See the complete license in LICENSE and/or "http://www.cecill.info".        
// ============================================================================
#ifndef VIRTUAL_PARTICLES_SELECT_H
#define VIRTUAL_PARTICLES_SELECT_H
#include <qcolor.h>
#include <qstring.h>
#include <iostream>
#include "particles_select.h"

class VirtualParticlesSelect {
public:
    VirtualParticlesSelect();
    VirtualParticlesSelect(const VirtualParticlesSelect&);
    const VirtualParticlesSelect& VirtualParticlesSelect::operator=(const 
                                         VirtualParticlesSelect&);
    virtual ~VirtualParticlesSelect();

    int npart;            // #particles
    int first_part;       // index of the first particle       
    int last_part;        // index of the last particle        
    int step_part;        // incremental step between particles
    QColor col;           // particles color                   
    bool   is_visible;    // TRUE if particles are visible     
    static int nb_select; 
    std::vector<int> index_list;  
    std::string list_file;  
    int v_type;           // virtual object of type:
                          // (1) ParticlesRange     
                          // (2) ParticlesList      
                          
    std::string error_message;                      
    virtual int loadFile(const char *,ParticlesSelectVector * ) {return 0;};
    virtual int getIndex(int);
    virtual void printRange();
    virtual char * parseString(const char * , const int , 
		  	       ParticlesSelectVector * );
    template <class T> int storeParticles(ParticlesSelectVector * ,const int , 
                                   const char * );
    int storeParticlesList(ParticlesSelectVector * ,const int , 
                                   const char * );
    void setColor();
    int npartSelected(ParticlesSelectVector *,int);
private:                               
    virtual int parseSelectedString(char * , const int , 
				      ParticlesSelectVector * ) { return 0;};
    
};
#endif
// ============================================================================