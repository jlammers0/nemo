// ============================================================================
// Copyright Jean-Charles LAMBERT - 2007-2010                                  
// e-mail:   Jean-Charles.Lambert@oamp.fr                                      
// address:  Dynamique des galaxies                                            
//           Laboratoire d'Astrophysique de Marseille                          
//           Pôle de l'Etoile, site de Château-Gombert                         
//           38, rue Frédéric Joliot-Curie                                     
//           13388 Marseille cedex 13 France                                   
//           CNRS U.M.R 6110                                                   
// ============================================================================
// See the complete license in LICENSE and/or "http://www.cecill.info".        
// ============================================================================
#include "formhelp.h"
#include "ui_formhelp.h"

namespace glnemo {
  
FormHelp::FormHelp(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FormHelp)
{
    ui->setupUi(this);
}

FormHelp::~FormHelp()
{
    delete ui;
}
}
